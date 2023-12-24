#ifndef TEXTURE_TRIANGLES_H2
#define TEXTURE_TRIANGLES_H2

#include "DEFINITIONS.h"
#include "PRIMITIVE.h"
#include "SETTINGS.h"
#include "TRIANGLES.h"

class TextureTriangle2 : public Primitive {
private:
    POINT3 vert1;
    POINT3 vert2;
    POINT3 vert3;

    float *texture;
    int textXRes;
    VEC2 text1;
    VEC2 text2;
    VEC2 text3;
    double p;
    shared_ptr<Material> mat;
    Box aabb;

public:
    TextureTriangle2(const POINT3& V1, const POINT3& V2, const POINT3& V3, const double& phong, shared_ptr<Material> material, float *textureSource, int tXRes, VEC2 t1, VEC2 t2, VEC2 t3) 
        : vert1(V1), vert2(V2), vert3(V3), p(phong), mat(material), texture(textureSource), text1(t1), text2(t2), text3(t3), textXRes(tXRes) {
            VEC3 min = minVec3(vert1, vert2, vert3);
            VEC3 max = maxVec3(vert1, vert2, vert3);

            aabb = Box(min, max);
        }

    POINT3 v1() const  { return vert1; }
    POINT3 v2() const  { return vert2; }
    POINT3 v3() const  { return vert3; }
    double phong() const { return p; }
    shared_ptr<Material> material() const { return mat; }
    Box boundingBox() const override { return aabb; }


    // adaptation of Moller-Trumbore intersection algorithm
    bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
        bool debug = false; // r.debug;
        const double e = 0.0000001;


        VEC3 e1 = vert2 - vert1;
        VEC3 e2 = vert3 - vert1;
        VEC3 perp_e2 = (r.direction()).cross(e2);
        VEC3 origin_ray = r.origin() - vert1;
        VEC3 perp_e1 = origin_ray.cross(e1);
        
        double a = e1.dot(perp_e2);
        double scale = 1.0 / a;
        bool parallel_check = (a > -e && a < e);      
        if (parallel_check)
        {
            return false; 
        }
        
        double bary_x = scale * origin_ray.dot(perp_e2);
        double bary_y = scale * (r.direction()).dot(perp_e1);
        if (bary_x < 0.0 || bary_x > 1.0)
        {
            return false;
        }
        if (bary_y < 0.0 || bary_x + bary_y > 1.0)
        {
            return false;
        }
        
        double t = scale * e2.dot(perp_e1);

        if (t > e)
        {
            if(t < t1 && t > t0) {
                rec.t = t;
                rec.p = r.at(t);
                VEC3 out = (e1.cross(e2)).normalized();
                rec.set_normal(r, out);
                rec.phong = this->phong();
                rec.material = this->material();

                // VEC2 t1(0, 0);
                // VEC2 t2(0, 10000);
                // VEC2 t3(10000, 0);
                
                // VEC2 texturePoint = bary_x * t1 + bary_y * t2 + (1 - bary_x - bary_y) * t3;
                // int idealRes = 10000;
                double expansion = 1;
                VEC2 texturePoint = bary_x * text1 * expansion + bary_y * text2 * expansion + (1 - bary_x - bary_y) * text3 * expansion;
                if (debug) {
                    cout << "t2's expansion is " << text2 * expansion << endl;
                }
                int textX = (int) texturePoint[0] % textXRes;
                int textY = (int) texturePoint[1] % textXRes;

                float rangeToBlend = 150.0;

                

                int textCoord = (textX + textY * textXRes) * 3;

                VEC3 textureColor(texture[textCoord] / 255, texture[textCoord+1] / 255, texture[textCoord+2] / 255);

                if (abs(textXRes - textX) < rangeToBlend || abs(textXRes - textY) < rangeToBlend) {
                    int otherSideX = textX;
                    int otherSideY = textY;
                    float blend = 0;
                    if (abs(textXRes - textX) < rangeToBlend) {
                        blend += 0.5 * ((rangeToBlend - abs(textXRes - textX)) / rangeToBlend);
                        otherSideX = (textXRes - textX + 20);
                    }

                    if (abs(textXRes - textY) < rangeToBlend) {
                        blend = (blend > 0.5 * ((rangeToBlend - abs(textXRes - textY)) / rangeToBlend)) ? blend : ((rangeToBlend - abs(textXRes - textY)) / rangeToBlend);
                        otherSideY = (textXRes - textY + 20);
                    }
                    int otherSideTextCoord = (otherSideX + otherSideY * textXRes) * 3;
                    if (debug) {
                        cout << "Blend is " << blend << endl;
                    }
                    textureColor = (1 - blend) * textureColor + blend * VEC3(texture[otherSideTextCoord] / 255, texture[otherSideTextCoord+1] / 255, texture[otherSideTextCoord+2] / 255);

                }
                if (debug) {
                    cout << "Hitting " << textX << ", " << textY << " with " << bary_x << ", " << bary_y << ", " << (1 - bary_x - bary_y) << " with color " << textureColor << endl;
                }
                rec.kd = textureColor;
                return true;
            }
        }
        
        return false;
    }
};

#endif