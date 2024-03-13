#ifndef CAMERA_H
#define CAMERA_H

#include "DEFINITIONS.h"

#include "SETTINGS.h"

#include "RAY.h"

#include "WORLD.h"

#include "MATERIALS.h"

#include "PERLIN.h"


class Camera {
  public: int xRes,
  yRes;
  VEC3 eye,
  lookat,
  up;
  double n,
  fovy;
  double t,
  b,
  r,
  l;
  VEC3 phong;
  VEC3 U,
  V,
  W;
  double eta;
  double aperture;
  double focalLength;
  int cameraBackGroundX;

  Camera(const int & XRES,
    const int & YRES,
      const VEC3 & EYE,
        const VEC3 & LOOKAT,
          const VEC3 & UP,
            const double & NEAR, double FOVY,
              const VEC3 & PHONG,
                const double & air,
                  double aper, double foc_len): xRes(XRES),
  yRes(YRES),
  eye(EYE),
  lookat(LOOKAT),
  up(UP),
  n(NEAR),
  fovy(FOVY * (M_PI / 180.0)),
  phong(PHONG),
  eta(air),
  aperture(aper),
  focalLength(foc_len) {
    t = abs(n) * tan(fovy / 2.0);
    r = (double(xRes) / double(yRes)) * t;
    l = -r;
    b = -t;
    W = ((eye - lookat).normalized());
    U = ((up.cross(W)).normalized());
    V = W.cross(U);
    pn = PerlinNoise(seed);
    cameraBackGroundX = 0;
  }

  void clearInputs(float * & pixels) {
    for (int i = 0; i < 3 * xRes * yRes; i++) {
      pixels[i] = 0;
    }
  }

  int pixelIndex(float x, float y) {
    return 3 * ((floor(x)) + ((floor(y)) * xRes));
  }

  void update() {
    t = abs(n) * tan(fovy / 2.0);
    r = (double(xRes) / double(yRes)) * t;
    l = -r;
    b = -t;
    W = ((eye - lookat).normalized());
    U = ((up.cross(W)).normalized());
    V = W.cross(U);
    cameraBackGroundX += 4;
    // cout << "Eye is now " << eye << endl << endl;
  }

  void render(const Primitive & world,
    const vector < Light > & lights, float * & pixels, bool antialiasing,
      bool softShadow, bool depth_of_view) {
    double sample_grid_size = 4.0;
    double disk_sample = 4.0;

    for (size_t y = 0; y < yRes; y++) {
      if (y % 10 == 0) {
        if (y != 0) {
          cout << "\x1b[1A\x1b[2K";
        }

        cout << "Completion: " << (int)(y / (float) yRes * 100) << "%" << endl;
      }
      for (size_t x = 0; x < xRes; x++) {
        VEC3 color(0.0, 0.0, 0.0);
        if (antialiasing && depth_of_view) {
          for (size_t p = 0; p < sample_grid_size; p++) {
            for (size_t q = 0; q < sample_grid_size; q++) {
              double xi = (double) rand() / RAND_MAX;
              double sample_u = l + ((r - l) * (x + ((p + xi) / sample_grid_size))) / xRes;
              double sample_v = b + ((t - b) * (y + ((q + xi) / sample_grid_size))) / yRes;

              VEC3 lensPoint = eye + aperture * random_in_unit_disk();
              VEC3 pixelDirection = -sample_u * U - sample_v * V - n * W;
              VEC3 focalPoint = eye + focalLength * pixelDirection.normalized();
              VEC3 direction = (focalPoint - lensPoint).normalized();
              // VEC3 direction = -u * U + -v * V - n * W;
              // direction.normalize();
              Ray r(lensPoint, direction);
              double backgroundX = cameraBackGroundX + x;
              double backgroundY = y;
              color += rayColor(r, world, lights, 10, softShadow, backgroundX, backgroundY);
            }
          }
          color = color / (sample_grid_size * sample_grid_size);
        } else if (depth_of_view) {
          double u = l + (((r - l) * (x + 0.5)) / xRes);
          double v = b + (((t - b) * (y + 0.5)) / yRes);
          // I think notes refer to this as the S vector
          VEC3 focalPoint = eye + -u * U + -v * V + -focalLength * W;
          for (size_t i = 0; i < disk_sample * disk_sample; i++) {

            VEC3 lensPoint = eye + aperture * random_in_unit_disk();
            VEC3 direction1 = -u * U + -v * V - n * W;
            direction1.normalize();
            VEC3 pixelDirection = -u * U - v * V - n * W;
            VEC3 focalPoint = eye + focalLength * pixelDirection.normalized();
            // Calculate direction from lens point to the focal plane
            VEC3 direction = (focalPoint - lensPoint).normalized();

            Ray r(lensPoint, direction);
            r.debug = false; // (x > 191 && x < 195 && y > 276 && y < 278); // Set debugging parameters
            if (r.debug && i == 0) {
              cout << "\n\n\n======== For point " << x << ", " << y << endl;
              cout << "eye is " << endl << eye << endl;
              cout << "lensPoint is" << endl << lensPoint << endl;
              cout << "dir1 is" << endl << direction1 << endl;
              cout << "focalPoint is" << endl << focalPoint << endl;
              cout << "dir is" << endl << direction << endl;
            }

            double backgroundX = cameraBackGroundX + x;
            double backgroundY = y;
            color += rayColor(r, world, lights, 10, softShadow, backgroundX, backgroundY);
          }
          color = color / (disk_sample * disk_sample);
        } else if (antialiasing) {
          for (size_t p = 0; p < sample_grid_size; p++) {
            for (size_t q = 0; q < sample_grid_size; q++) {
              double xi = (double) rand() / RAND_MAX;
              double u = l + ((r - l) * (x + ((p + xi) / sample_grid_size))) / xRes;
              double v = b + ((t - b) * (y + ((q + xi) / sample_grid_size))) / yRes;

              VEC3 direction = -u * U + -v * V - n * W;
              direction.normalize();
              Ray r(eye, direction);
              double backgroundX = cameraBackGroundX + x;
              double backgroundY = y;
              color += rayColor(r, world, lights, 10, softShadow, backgroundX, backgroundY);
            }
          }
          color = color / (sample_grid_size * sample_grid_size);
        } else {
          double u = l + (((r - l) * (x + 0.5)) / xRes);
          double v = b + (((t - b) * (y + 0.5)) / yRes);

          VEC3 direction = -u * U + -v * V - n * W;
          direction.normalize();
          Ray r(eye, direction);
          r.debug = false; // (x == 10 && y== 10); // Set debugging parameters
          // if (r.debug) {
          //   cout << "\n\n\n======== For point " << x << ", " << y << endl;
          // }

          double backgroundX = cameraBackGroundX + x;
          double backgroundY = y;
          color = rayColor(r, world, lights, 10, softShadow, backgroundX, backgroundY);
          // if (r.debug) {
          //   cout << "======== Color is " << color << endl << endl;
          // }
        }

        int idx = pixelIndex(x, y);
        pixels[idx] = floor(255.0f * min(1.0, color[0]));
        pixels[idx + 1] = floor(255.0f * min(1.0, color[1]));
        pixels[idx + 2] = floor(255.0f * min(1.0, color[2]));
      }
    }
    cout << "\x1b[1A\x1b[2K";
  }

  private: unsigned int seed = 237;
  PerlinNoise pn;

  VEC3 sampleSoftShadow(VEC3 lightPosition, VEC3 lightNormal, float rx, float ry) const {
    VEC3 lightZ = lightNormal;
    VEC3 randomDir = VEC3(1, 0, 0);
    if (abs(randomDir.dot(lightZ)) < 0.0001) {
      randomDir = VEC3(0, 1, 0);
    }
    VEC3 lightX = lightZ.cross(randomDir).normalized();
    VEC3 lightY = lightZ.cross(lightX).normalized();

    // double x = rx * lightEdgeSize;
    // double y = ry * lightEdgeSize;

    VEC3 sampledPoint = lightPosition + rx * lightX + ry * lightY;
    return sampledPoint;
  }

  VEC3 rayColor(const Ray & r,
    const Primitive & world,
      const vector < Light > & lights, int depth, bool softShadow, double x, double y) const {
    hit_record rec, srec;
    VEC3 color = VEC3(0.0, 0.0, 0.0);

    bool debug = false;

    if (depth <= 0) {
      return color;
    }

    if (world.hit(r, 0.0, INFINITY, rec)) {
      if (lights.empty()) {
        return rec.kd;
      }

      POINT3 p = rec.p;
      VEC3 v = (eye - p).normalized();
      VEC3 n = rec.normal;
      VEC3 c_p = phong;

      for (const auto & light: lights) {
        VEC3 l = (light.position() - p).normalized();
        VEC3 lightPos = light.position();
        if (debug) {
          cout << "For light at pos " << lightPos << endl;
        }

        int SAMPLE_SIZE = 4;
        float radius = 0.1;
        if (!softShadow) {
          SAMPLE_SIZE = 1;
          radius = 0;
        }
        for (int i = 0; i < SAMPLE_SIZE; i++) {
          for (int j = 0; j < SAMPLE_SIZE; j++) {
            double rand_x = 2 * (double) rand() / RAND_MAX - 1;
            double rand_y = 2 * (double) rand() / RAND_MAX - 1;

            double x = radius * (i + rand_x) / SAMPLE_SIZE;
            double y = radius * (j + rand_y) / SAMPLE_SIZE;

            VEC3 lightNormal = lightPos - p;
            lightPos = sampleSoftShadow(lightPos, lightNormal, x, y);
            if (debug) {
              cout << "Sampled point " << i << " is " << lightPos << endl;
            }
            VEC3 l = (lightPos - p).normalized();
            // VEC3 h = (v + l).normalized();
            rec.ks = light.color();

            Ray sr(p, l);

            if (!world.hit(sr, 0.0001, INFINITY, srec) || distance(p, srec.p) > distance(p, lightPos)) {
              VEC3 h = (l.normalized() + (-r.direction()).normalized()).normalized();
              double n_dot_l = max(0.0, n.dot(l));
              double n_dot_h = max(0.0, n.dot(h));

              if (debug) {
                cout << "Adding " << (compMult(rec.kd, (rec.ks * n_dot_l)) + compMult(c_p, rec.ks) * pow(n_dot_h, rec.phong)) / (float)(SAMPLE_SIZE * SAMPLE_SIZE) << " to the color " << endl;
              }

              float reduction = SAMPLE_SIZE * (SAMPLE_SIZE);
              color += (compMult(rec.kd, (rec.ks * n_dot_l)) + compMult(c_p, rec.ks) * pow(n_dot_h, rec.phong)) / (float)(reduction);
            } else {
              if (debug) {
                cout << "Didn't hit light\n";
              }

            }
          }

        }
      }

      Ray scatteredRay;
      VEC3 color_from_emission = rec.material -> emitted(rec.u, rec.v, rec.p);
      if (rec.material -> scatter(r, rec, scatteredRay)) {
        if (rec.material -> isGlass()) {

          VEC3 d = r.direction();
          d.normalize();
          rec.eta_t = eta / rec.material -> index();
          double c;

          double k_reflect = fresnel(d, n, (scatteredRay.direction()).normalized(), eta, rec.eta_t);
          double k_refract = 1.0 - k_reflect;
          // // cout << k_reflect << endl;

          VEC3 r = reflect(d, n);
          Ray reflect_ray(rec.p + r * 0.01, r);
          reflect_ray.reflected = true;
          VEC3 t;
          if (d.dot(n) < 0) {
            refract(d, n, t, rec.material -> index());
            c = (-d).dot(n);
          } else {
            if (refract(d, -n, t, 1.0 / rec.material -> index())) {
              cout << "2" << endl;
              c = t.dot(n);
            } else {
              color += rayColor(reflect_ray, world, lights, depth - 1, softShadow, x, y);
            }
          }
          Ray refract_ray(rec.p + t * 0.01, t);
          double R0 = ((rec.eta_t - 1.0) * (rec.eta_t - 1.0)) / ((rec.eta_t + 1.0) * (rec.eta_t + 1.0));
          double R = R0 + (1 - R0) * pow((1 - c), 5);

          color += R * rayColor(reflect_ray, world, lights, depth - 1, softShadow, x, y);
        }
        scatteredRay.reflected = true;
        color += rayColor(scatteredRay, world, lights, depth - 1, softShadow, x, y);
      }
      return color;
    } else {
      // Ray doesn't hit anything
      color = VEC3(135, 206, 235); // sky blue

      double beta = (double) y / (640.0);
      beta = pow(beta, 2);
      double alpha = (double) x / (480.0);

      if (r.reflected) {
        // cout << "Reflected" << endl;
        alpha += 1.5; // + 200;
        beta += 2.8; // + 400;
      }

      // VEC3 color1 = VEC3(238,175,97);
      // VEC3 color2 = VEC3(251,144,98);
      // VEC3 color3 = VEC3(238,93,108);
      // VEC3 color4 = VEC3(206,73,147);

      VEC3 color1 = VEC3(13, 23, 14);
      VEC3 color2 = VEC3(51, 51, 51);
      VEC3 color3 = VEC3(55, 234, 249);
      VEC3 color4 = VEC3(119, 81, 169);

      float comp1 = 3 * (pn.noise(alpha * 37, beta * 37, 0.8));
      float comp2 = (pn.noise(alpha * 19, beta * 19, 0.8));
      float comp3 = 2 * (pn.noise(alpha * 53, beta * 53, 0.8));
      // float comp4 = 0.1 * (pn.noise(alpha * 23, beta * 23, 0.8));

      float sumComp = comp1 + comp2 + comp3; // + comp4;

      float coef1 = 0.7; // comp1 / sumComp;
      float coef2 = 0.2; // comp2 / sumComp;
      float coef3 = 0.07; // comp3 / sumComp;
      float coef4 = 0.03; // comp4 / sumComp;

      // VEC3 baseColor = coef1 * color1 + color2 * coef2 + coef3 * color3 + coef4 * color4;
      VEC3 baseColor(2, 30, 50);

      VEC3 cloudColor = VEC3(137, 255, 119);
      VEC3 cloudColor2 = VEC3(55, 234, 249);
      VEC3 cloudColor3 = VEC3(224, 253, 163);

      float cc1 = comp1 / sumComp;
      float cc2 = comp2 / sumComp;
      float cc3 = 1 - cc1 - cc2;

      cloudColor = cc1 * cloudColor + cc2 * cloudColor2 + cc3 * cloudColor3;

      double n = (pn.noise(alpha * 14, beta * 15, 0.8));

      // float r = 135;
      // float g = 206;
      // float b = 235;

      float r = baseColor[0];
      float g = baseColor[1];
      float b = baseColor[2];

      float cloudProminance = 2;

      n = 1 / (1 + exp(-9 * (n - 0.8)));

      // n = pow(n, cloudProminance);
      // n /= 1.5;
      // n = pow(n, (cloudProminance/n));
      // if (n < 0.5) {
      //     n = pow(n, 2);
      // }
      // if (n > 0.5) {
      //     n = sqrt(n);
      // }
      // n *= 1.3;

      float rToW = cloudColor[0] - r;
      float gToW = cloudColor[1] - g;
      float bToW = cloudColor[2] - b;

      r += rToW * n;
      g += gToW * n;
      b += bToW * n;

      // tones of grey
      r = (r > 255) ? 255 : r;
      g = (g > 255) ? 255 : g;
      b = (b > 255) ? 255 : b;

    //   color = VEC3(r, g, b) / 255.0;
    // cout << "Returning " << VEC3(r, g, b) << endl;
      return VEC3(r, g, b) / 255.0;
      /*
       */
    }

  }
};

#endif