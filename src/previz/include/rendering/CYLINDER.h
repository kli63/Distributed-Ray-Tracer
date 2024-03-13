#ifndef CYLINDER_H
#define CYLINDER_H

#include "DEFINITIONS.h"
#include "PRIMITIVE.h"
#include "SETTINGS.h"

class Cylinder : public Primitive {
private:
    VEC3 a, b;
    shared_ptr<Material> mat;
    float radius;
    VEC3 col;
    float p;
    Box aabb;

public:
    Cylinder(const VEC3& a_1, const VEC3& b_1, const float& p_radius, const float& phong, const VEC3& color, shared_ptr<Material> material) 
        : a(a_1), b(b_1), col(color), p(phong), radius(p_radius), mat(material) {
            VEC3 min_vertex = VEC3(
                min(a[0], b[0]) - radius,
                min(a[1], b[1]) - radius,
                min(a[2], b[2]) - radius
            );

            VEC3 max_vertex = VEC3(
                max(a[0], b[0]) + radius,
                max(a[1], b[1]) + radius,
                max(a[2], b[2]) + radius
            );

            aabb = Box(min_vertex, max_vertex);

        }

    // POINT3 c() const  { return center; }
    // POINT3 d() const  { return direction; }
    VEC3 color() const  { return col; }
    float r() const { return radius; }
    double phong() const { return p; }
    shared_ptr<Material> material() const { return mat; }
    Box boundingBox() const override { return aabb; }

    float sign(float val) const {
        return (val > 0) ? 1 : -1;
    }

    VEC4 cylIntersect(const VEC3 ro, const VEC3 rd, VEC3 a, VEC3 b, float ra) const {
        VEC3  ba = b  - a;
        VEC3  oc = ro - a;
        float baba = ba.dot(ba);
        float bard = ba.dot(rd);
        float baoc = ba.dot(oc);
        float k2 = baba            - bard*bard;
        float k1 = baba*oc.dot(rd) - baoc*bard;
        float k0 = baba*oc.dot(oc) - baoc*baoc - ra*ra*baba;
        float h = k1*k1 - k2*k0;
        if( h<0.0 ) return VEC4(-1.0, -1.0, -1.0, -1.0);//no intersection
        h = sqrt(h);
        float t = (-k1-h)/k2;
        // body
        float y = baoc + t*bard;
        if( y>0.0 && y<baba ) {
            VEC3 zzz = (oc+t*rd - ba*y/baba)/ra;
            return VEC4(t, zzz[0], zzz[1], zzz[2]);
        }
        // caps
        t = ( ((y<0.0) ? 0.0 : baba) - baoc)/bard;
        if( abs(k1+k2*t)<h )
        {
            VEC3 zzz = ba*sign(y)/sqrt(baba);
            return VEC4( t, zzz[0], zzz[1], zzz[2]);
        }
        return VEC4(-1.0, 0, 0, 0);//no intersection
    }

    // normal at point p of cylinder (a,b,ra), see above
    VEC3 cylNormal(VEC3 p, VEC3 a, VEC3 b, float ra ) const 
    {
        VEC3  pa = p - a;
        VEC3  ba = b - a;
        float baba = ba.dot(ba);
        float paba = pa.dot(ba);
        float h = paba/baba;
        return (pa - ba*h)/ra;
    }



    bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
        // cout << "1" << endl;
        bool debug = false;
        // VEC3 a1 = center + direction * (length / 2);
        // VEC3 b1 = center - direction * (length / 2);
        VEC3 d = r.direction();
        if (debug)
        {
            cout << "Eye: " << r.origin() << "\nDirection: " << r.direction() << "\na: " << a << "\nb: " << b << "\nradius: " << radius << endl;
            // d = VEC3(0, 0.357047, 0.934086);
        }
        VEC4 result = cylIntersect(r.origin(), d, a, b, radius);
        if (debug) {
            cout << "Ray result is " << result << endl;
        }
        if (result[0] == -1) {
            // cout << "2" << endl;
            return false;
        } else {
            float t = result[0];
            if (t < t0 || t > t1)
            {
                return false;
            }
            
            VEC3 hitCoord;
            hitCoord[0] = result[1];
            hitCoord[1] = result[2];
            hitCoord[2] = result[3];
            rec.t = t;
            rec.p = r.at(t);
            VEC3 normal = cylNormal(rec.p, a, b, radius);
            normal.normalize();
            rec.set_normal(r, normal);
            if (debug) {
                cout << "Normal vector is " << rec.normal << endl;
            }
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
            
            // cout << "3" << endl;

            return true;
        }
    }

    // bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
    // //      VEC3 d = r.direction();
    // //     VEC3 o = r.origin();
    // //     VEC3 c = this->center();
    // // }
};


#endif
