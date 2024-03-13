// #ifndef TORUS_H
// #define TORUS_H

// #include "DEFINITIONS.h"
// #include "PRIMITIVE.h"
// #include "SETTINGS.h"

// class Torus : public Primitive {
// private:
//     POINT3 ctr; 
//     double r1; 
//     double r2;
//     VEC3 col;
//     double p;
//     shared_ptr<Material> mat; 

// public:
//     // Constructors
//     Torus(const POINT3& center, const double& majorRadius, const double& minorRadius, const VEC3& color) 
//         : ctr(center), r1(majorRadius), r2(minorRadius), col(color), p(10.0), mat(nullptr) {}

//     Torus(const POINT3& center, const double& majorRadius, const double& minorRadius, const double& phong, const VEC3& color, shared_ptr<Material> material) 
//         : ctr(center), r1(majorRadius), r2(minorRadius), col(color), p(phong), mat(material) {}

//     // Getters
//     POINT3 center() const { return ctr; }
//     double majorRadius() const { return r1; }
//     double minorRadius() const { return r2; }
//     VEC3 color() const { return col; }
//     double phong() const { return p; }
//     shared_ptr<Material> material() const { return mat; }

//     // Bounding box method (you need to define how to calculate this for a torus)
//     Box boundingBox() const {
//         // Calculate the bounding box based on the major and minor radii
//     }

//     VEC3 torNormal(const POINT3& p) const {
// 	VEC3 normal;
// 	double param_squared = r1 * r1 + r2 * r2;
    
// 	double x = p[0];
// 	double y = p[1];
// 	double z = p[2];
// 	double sum_squared = x * x + y * y + z * z;
	
// 	normal[0] = 4.0 * x * (sum_squared - param_squared);
// 	normal[1] = 4.0 * y * (sum_squared - param_squared + 2.0 * r1 * r1);
// 	normal[2] = 4.0 * z * (sum_squared - param_squared);
// 	normal.normalize();
	
// 	return normal;	
// }


//     // Hit function (requires solving a quartic equation)
//     bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
//         VEC3 d = r.direction();
//         VEC3 o = r.origin();

//         double x1 = o[0]; double y1 = o[1]; double z1 = o[2];
//         double d1 = o[0]; double d2 = d[1]; double d3 = d[2];

//         double coeffs[5];
// 	    double roots[4];

//         double sum_d_sqrd 	= d1 * d1 + d2 * d2 + d3 * d3;
//         double e			= x1 * x1 + y1 * y1 + z1 * z1 - r1 * r1 - r2 * r2;
//         double f			= x1 * d1 + y1 * d2 + z1 * d3;
//         double four_a_sqrd	= 4.0 * r1 * r1;

//         coeffs[0] = e * e - four_a_sqrd * (r2 * r2 - y1 * y1); 	// constant term
//         coeffs[1] = 4.0 * f * e + 2.0 * four_a_sqrd * y1 * d2;
//         coeffs[2] = 2.0 * sum_d_sqrd * e + 4.0 * f * f + four_a_sqrd * d2 * d2;
//         coeffs[3] = 4.0 * sum_d_sqrd * f;
//         coeffs[4] = sum_d_sqrd * sum_d_sqrd;

//         int num_real_roots = SolveQuartic(coeffs, roots);
//         double t = t1;

	
//         if (num_real_roots == 0)
//         {
//             return false;
//         }

//         for (int j = 0; j < num_real_roots; j++)  
//             if (roots[j] > t0) {
//                 if (roots[j] < t)
//                     t = roots[j];
//             }
        
//         if (t < t0 || t > t1)
//             {
//                 return false;
//             }
        
//         rec.t = t;
//         rec.p = r.at(rec.t);
//         VEC3 out = torNormal(rec.p);
//         rec.set_normal(r, out);
//         rec.phong = this->phong();
//         rec.material = this->material();

    
//         if (rec.material != nullptr)
//         {
//             rec.kd = rec.material->color();
//         }
//         else
//         {
//             rec.kd = this->color();
//         }
//         // rec.kd = this->color();
//         return true;
//     }

// };

// #endif
