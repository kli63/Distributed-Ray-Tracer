#ifndef PRIMITIVE_H
#define PRIMITIVE_H
#include <vector>
#include <memory>

#include "DEFINITIONS.h"
#include "SETTINGS.h"
#include "RAY.h"

class Material;

class Interval {
    public:
        double min;
        double max;
        
        // empty interval
        Interval() : min(INFINITY), max(-INFINITY) {}

        Interval(double MIN, double MAX) : min(MIN), max(MAX) {}

        // Merge interval constructor
        Interval(const Interval& a, const Interval& b)
            : min(std::min(a.min, b.min)), max(std::max(a.max, b.max)) {}

        bool contains(double x) const {
            return min <= x && x <= max;
        }

        bool surrounds(double x) const {
            return min < x && x < max;
        }

        double size() const {
            return max - min;
        }

        Interval expand(double delta) const {
            double padding = delta / 2.0;
            return Interval(min - padding, max + padding);
        }

    static const Interval empty, universe;
};

const static Interval empty(+INFINITY, -INFINITY);
const static Interval universe(-INFINITY, +INFINITY);

class Box {
    public:
        Interval x, y, z;

        Box() {}

        Box(const POINT3& p1, const POINT3& p2) {
            x = Interval(min(p1[0], p2[0]), max(p1[0], p2[0]));
            y = Interval(min(p1[1], p2[1]), max(p1[1], p2[1]));
            z = Interval(min(p1[2], p2[2]), max(p1[2], p2[2]));
        }

        Box(const Interval& x_interval, const Interval& y_interval, const Interval& z_interval)
            : x(x_interval), y(y_interval), z(z_interval) {}

        // Analagous to combine (combination constructor) 
        Box(const Box& a, const Box& b)
        {
            x = Interval(a.x, b.x);
            y = Interval(a.y, b.y);
            z = Interval(a.z, b.z);

        }

        const Interval& axis(int n) const {
            // Shirley Marschner Convention
            if (n == 0) return x;
            if (n == 1) return y;
            return z;
        }

        bool hit(const Ray& r, Interval& r_interval) const {
            double t_min;
            double t_max;
            for (size_t i = 0 ; i < 3; i++)
            {
                double direction = r.direction()[i];
                double origin = r.origin()[i];
                // reciprocal of direction 
                double a = 1.0 / direction;

                if (a >= 0)
                {
                    t_min = a * (axis(i).min - origin);
                    t_max = a * (axis(i).max - origin);
                }
                else
                {
                    t_min = a * (axis(i).max - origin);
                    t_max = a * (axis(i).min - origin);
                }

                if (t_min > r_interval.min) r_interval.min = t_min;
                if (t_max < r_interval.max) r_interval.max = t_max;

                if (t_min > r_interval.max || r_interval.min > t_max)
                {
                    // cout << "miss!" << endl;
                    return false;
                }
            }
            // cout << "hit!" << endl;
            return true;
        }
};



class hit_record {
    public:
        POINT3 p;
        VEC3 normal;
        double t;
        bool front;
        double phong;
        double eta_t;
        double u, v;
        
        VEC3 kd;
        VEC3 ks;
        shared_ptr<Material> material;

        hit_record() : kd(VEC3(0.0, 0.0, 0.0)), ks(VEC3(0.0,0.0,0.0)) {}

        void set_normal(const Ray& r, const VEC3& out)
        {
            front = (r.direction()).dot(out) < 0;
            if (front)
            {
                normal = out;
            }
            else
            {
                normal = -out;
            }
        }
};

class Primitive {
public:
    virtual ~Primitive() = default;
    virtual bool hit(const Ray& ray, double t0, double t1, hit_record& rec) const = 0;
    virtual Box boundingBox() const = 0;

};

#endif