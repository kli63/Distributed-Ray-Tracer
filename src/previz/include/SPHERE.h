#ifndef SPHERE_H
#define SPHERE_H

#include "DEFINITIONS.h"
#include "PRIMITIVE.h"
#include "SETTINGS.h"

class Sphere : public Primitive {
private:
    POINT3 ctr;
    double rad;
    VEC3 col;
    double p;
    shared_ptr<Material> mat;
    Box aabb;

public:
    Sphere(const POINT3& center, const double& radius, const VEC3& color) 
        : ctr(center), rad(radius), col(color), p(10.0), mat(nullptr) {
            VEC3 min = this->center() - VEC3(this->radius(), this->radius(), this->radius());
            VEC3 max = this->center() + VEC3(this->radius(), this->radius(), this->radius());

            aabb = Box(min, max);
        }

    Sphere(const POINT3& center, const double& radius, const double& phong, const VEC3& color, shared_ptr<Material> material) 
        : ctr(center), rad(radius), col(color), p(phong), mat(material) {
            VEC3 min = this->center() - VEC3(this->radius(), this->radius(), this->radius());
            VEC3 max = this->center() + VEC3(this->radius(), this->radius(), this->radius());

            aabb = Box(min, max);
            // cout << "min" << endl << min << endl << "max" << endl << max << endl;
        }
    

    POINT3 center() const  { return ctr; }
    double radius() const  { return rad; }
    VEC3 color() const  { return col; }
    double phong() const { return p; }
    shared_ptr<Material> material() const { return mat; }
    Box boundingBox() const override { return aabb; }

    bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override{
        VEC3 d = r.direction();
        VEC3 o = r.origin();
        VEC3 c = this->center();
        double R = this->radius();

        double A = d.dot(d);
        double B = 2.0 * d.dot(o - c);
        double C = (o - c).dot(o - c) - (R * R);
        double discr = (B * B) - (4.0 * (A * C));

        if (discr < 0)
        {
            return false;
        }

        double t = (-B - sqrt(discr)) / (2.0 * A);
        if (t <= t0 || t1 <= t)
        {
            t = (-B + sqrt(discr)) / (2.0 * A);
            if (t <= t0 || t1 <= t)
            {
                return false;
            }
        }

        rec.t = t;
        rec.p = r.at(rec.t);
        VEC3 out = (rec.p - this->center()) / this->radius();
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
        // rec.kd = this->color();
        return true;
    }
};

#endif