#ifndef RAY_H
#define RAY_H

#include "DEFINITIONS.h"
#include "SETTINGS.h"


class Ray {
    public:
        Ray() {}

        Ray(const POINT3& origin, const VEC3& direction) : orig(origin), dir(direction) {}

        // Ray(const Camera& cam, int x, int y) {
        //   double u = cam.l + (((cam.r - cam.l) * (x + 0.5)) / cam.xRes);
        //   double v = cam.b + (((cam.t - cam.b) * (y + 0.5)) / cam.yRes);

        //   VEC3 direction = -u * cam.U + -v * cam.V - cam.n * cam.W;
        //   // direction.normalize();

        //   orig = cam.eye;
        //   dir = direction;
        // }

        POINT3 origin() const  { return orig; }
        VEC3 direction() const { return dir; }
        
        POINT3 at(double t) const {
            return orig + t * dir;
        }
        bool debug = false;
        bool reflected = false;

    private:
    POINT3 orig;
    VEC3 dir;
};

VEC3 reflect(const VEC3& d, const VEC3& n)
{
    return (d - (2 * d.dot(n) * n));
}

double distance(const POINT3& a, const POINT3& b)
{
    return (sqrt(pow((a[0] - b[0]),2)+ pow((a[1] - b[1]),2) + pow((a[2] - b[2]),2)));
}

double unitAngle(const VEC3& a, const VEC3& b)
{
    return acos(a.dot(b));
}

bool refract(const VEC3& d, const VEC3& n, VEC3& t, double ior) {
    double cos_theta = min((-d).dot(n), 1.0);
    VEC3 t1 =  ior * (d + cos_theta * n);
    double sin_theta = sqrt(abs(1.0 - (cos_theta * cos_theta)));

    VEC3 t2 = -sqrt(abs(1.0 - (t1.dot(t1)))) * n;
    t = t1 + t2;
    
    return true;
}

VEC3 refract1(const VEC3& d, const VEC3& n, double ior) {
    auto cos_theta = min((-d).dot(n), 1.0);
    VEC3 t1 =  ior * (d + cos_theta*n);
    VEC3 t2 = -sqrt(abs(1.0 - t1.dot(t1))) * n;
    return t1 + t2;
}

double fresnel(const VEC3& d, const VEC3& n, const VEC3& t, double n1, double n2)
{
    double cos_theta =  max(min((-d).dot(n), 1.0), -1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    
    double sin_phi = (n1 / n2) * sin_theta;
    if (sin_phi >= 1.0)
    {
        return 1.0;
    }

    double cos_phi = sqrt(abs(1.0 - sin_phi * sin_phi));

    double par = (n2 * cos_theta - n1 * cos_phi) / (n2 * cos_theta + n1 * cos_phi);
    double perp = (n1 * cos_theta - n2 * cos_phi) / (n1 * cos_theta + n2 * cos_phi);

    double kRef = (par * par + perp * perp) / 2.0;

    return kRef;
}

VEC3 compMult(const VEC3& v1, const VEC3& v2)
{
    return VEC3(v1[0] * v2[0], v1[1] * v2[1], v1[2] * v2[2]);
}

bool near_zero(const VEC3& v) 
{
    double s = 1e-8;
    return (fabs(v[0]) < s) && (fabs(v[1]) < s) && (fabs(v[2]) < s);
}



#endif