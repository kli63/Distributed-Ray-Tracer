#ifndef MATERIAL_H
#define MATERIAL_H

#include "SETTINGS.h"
#include "DEFINITIONS.h"
#include "RAY.h"
#include "WORLD.h"


class Material {
    public:
        virtual ~Material() = default;
        virtual bool isReflective() const = 0;
        virtual bool isGlass() const = 0;
        virtual VEC3 color() const = 0;
        virtual double index() const = 0;
        virtual bool isEmissive() const = 0;


        virtual bool scatter(
            const Ray& r_in, const hit_record& rec, Ray& scattered) const = 0;

        virtual VEC3 emitted(double u, double v, const POINT3& p) const {
            return VEC3(0.0, 0.0, 0.0);
        }      
};


class Lambertian : public Material {
    public:
        Lambertian(const VEC3& color) : albedo(color) {}
        bool isReflective() const override { return false; }
        bool isGlass() const override { return false; }
        VEC3 color() const override { return albedo; }
        double index() const override { return ior; }
        bool isEmissive() const override { return false; }


        bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) 
        const override { return false;}

    private:
        VEC3 albedo;
        double ior; 
};

class Mirror : public Material {
    public:
        Mirror(const VEC3& color) : albedo(color) {}
        bool isReflective() const override { return true; }
        VEC3 color() const override { return albedo; }
        bool isGlass() const override { return false; }
        double index() const override { return ior; }
        bool isEmissive() const override { return false; }

        bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) const override 
        {
            VEC3 reflected_direction = reflect((r_in.direction()).normalized(), rec.normal);
            scattered = Ray(rec.p + reflected_direction * 0.0001, reflected_direction);
            return true;
        }

    private:
        VEC3 albedo;
        double ior; 
}; 

class GlossyMirror : public Material {
    public:
        GlossyMirror(const VEC3& color, float glossy) : albedo(color), glossiness(glossy) {}
        
        bool isReflective() const override { return true; }
        VEC3 color() const override { return albedo; }
        bool isGlass() const override { return false; }
        double index() const override { return ior; }
        bool isEmissive() const override { return false; }

        bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) const override 
        {
            // VEC3 reflected_direction = reflect((r_in.direction()).normalized(), rec.normal);
            // scattered = Ray(rec.p + reflected_direction * 0.0001, reflected_direction);
            // return true;

            double a = rand() / (double) RAND_MAX;
            double b = rand() / (double) RAND_MAX;
            VEC3 reflectZ = reflect((r_in.direction()).normalized(), rec.normal);

            VEC3 randomDir = VEC3(1, 0, 0);
            if (abs(randomDir.dot(reflectZ)) < 0.0001) {
                randomDir = VEC3(0, 1, 0);
            }
            VEC3 reflectX = reflectZ.cross(randomDir).normalized();
            VEC3 reflectY = reflectZ.cross(reflectX).normalized();

            float u = -glossiness / 2 + a * glossiness;
            float v = -glossiness / 2 + b * glossiness;

            reflectZ = reflectZ + u * reflectX + v * reflectY;


            scattered = Ray(rec.p + reflectZ * 0.0001, reflectZ);
            return true;
        }

    private:
        VEC3 albedo;
        float glossiness;
        double ior; 
};  

class basicDielectric : public Material {
    public:
        basicDielectric(const VEC3& color, double index_of_refraction) : albedo(color), ior(index_of_refraction) {}
        bool isReflective() const override { return false; }
        VEC3 color() const override { return albedo; }
        bool isGlass() const override { return false; }
        double index() const override { return ior; }
        bool isEmissive() const override { return false; }

        bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) const override 
        {
            double refraction_ratio = rec.front ? (ior) : (1.0/ior);

            VEC3 unit_direction = (r_in.direction()).normalized();
            double cos_theta = min((-unit_direction).dot(rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - (cos_theta * cos_theta));

            bool tir = refraction_ratio * sin_theta > 1.0;
            VEC3 direction(0.0, 0.0, 0.0);

            if (tir)
            {
                direction = reflect(unit_direction, rec.normal); 
            }
            else
            {
                direction = refract1(unit_direction, rec.normal, refraction_ratio);
                // cout << direction << endl << endl;
            }


            scattered = Ray(rec.p + direction * 0.001, direction);
            return true;
        }

    private:
        VEC3 albedo;
        double ior; // Index of Refraction
};


class fresnelDielectric : public Material {
    public:
        fresnelDielectric(const VEC3& color, double index_of_refraction) : albedo(color), ior(index_of_refraction) {}
        bool isReflective() const override { return false; }
        bool isEmissive() const override { return false; }
        VEC3 color() const override { return albedo; }
        bool isGlass() const override { return true; }
        double index() const override { return ior; }
        

        bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) const override 
        {
            double refraction_ratio = rec.front ? (ior) : (1.0/ior);

            VEC3 unit_direction = (r_in.direction()).normalized();
            double cos_theta = (-unit_direction).dot(rec.normal);
            double sin_theta = sqrt(1.0 - (cos_theta * cos_theta));

            bool tir = refraction_ratio * sin_theta > 1.0;
            VEC3 direction(0.0, 0.0, 0.0);

            if (tir)
            {
                direction = reflect(unit_direction, rec.normal); 
            }
            else
            {
                refract(unit_direction, rec.normal, direction, refraction_ratio);
            }

            scattered = Ray(rec.p + direction * 0.001, direction);
            return true;
        }

    private:
        VEC3 albedo;
        double ior; // Index of Refraction
};


class Emissive : public Material {
  public:
    Emissive(const VEC3& color) : emit(color) {}

    bool isReflective() const override { return false; }
    bool isEmissive() const override { return false; }
    VEC3 color() const override { return emit; }
    bool isGlass() const override { return false; }
    double index() const override { return ior; }

    bool scatter(const Ray& r_in, const hit_record& rec, Ray& scattered) 
        const override { return false;}

    VEC3 emitted(double u, double v, const POINT3& p) const override {
        return emit;
    }

  private:
    VEC3 emit;
    double ior;
};





#endif