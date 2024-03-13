#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "DEFINITIONS.h"
#include "PRIMITIVE.h"
#include "SETTINGS.h"

VEC3 minVec3(const VEC3& v1, const VEC3& v2, const VEC3& v3) {
    return VEC3(min({v1[0], v2[0], v3[0]}),
                min({v1[1], v2[1], v3[1]}),
                min({v1[2], v2[2], v3[2]}));
}

VEC3 maxVec3(const VEC3& v1, const VEC3& v2, const VEC3& v3) {
    return VEC3(max({v1[0], v2[0], v3[0]}),
                max({v1[1], v2[1], v3[1]}),
                max({v1[2], v2[2], v3[2]}));
}

class Triangle : public Primitive {
private:
    POINT3 vert1;
    POINT3 vert2;
    POINT3 vert3;
    VEC3 col;
    double p;
    shared_ptr<Material> mat;
    Box aabb;

public:
    Triangle(const POINT3& V1, const POINT3& V2, const POINT3& V3, const double& phong, const VEC3& color, shared_ptr<Material> material) 
        : vert1(V1), vert2(V2), vert3(V3), col(color), p(phong), mat(material) {
            VEC3 min = minVec3(vert1, vert2, vert3);
            VEC3 max = maxVec3(vert1, vert2, vert3);

            aabb = Box(min, max);
        }

    POINT3 v1() const  { return vert1; }
    POINT3 v2() const  { return vert2; }
    POINT3 v3() const  { return vert3; }
    VEC3 color() const  { return col; }
    double phong() const { return p; }
    shared_ptr<Material> material() const { return mat; }
    Box boundingBox() const override { return aabb; }


    // adaptation of Moller-Trumbore intersection algorithm
    bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
        
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
                if (rec.material != nullptr)
                {
                    rec.kd = rec.material->color();
                }
                else
                {
                    rec.kd = this->color();
                }
                return true;
            }
        }
        
        return false;
    }
};

#endif