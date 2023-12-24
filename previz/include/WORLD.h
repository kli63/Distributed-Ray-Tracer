#ifndef WORLD_H
#define WORLD_H

#include "DEFINITIONS.h"
#include "SETTINGS.h"

#include <vector>
#include <memory>
#include "PRIMITIVE.h"

class Primitives : public Primitive {
    private:
        Box aabb;

    public:
        vector<shared_ptr<Primitive>> scene;

        Primitives() {}
        Primitives(shared_ptr<Primitive> object) { addtoScene(object); }
        void clean() { scene.clear(); }

        void addtoScene(shared_ptr<Primitive> object) {
            scene.push_back(object);
            aabb = Box(aabb, object->boundingBox());
        }

        bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
            hit_record temp;
            bool hit = false;
            double min_t = t1;

        
            for (const auto& primitive: scene)
            {
                if (primitive->hit(r, t0, min_t, temp))
                {
                    if (min_t > temp.t)
                    {
                        hit = true;
                        min_t = temp.t;
                        rec = temp;
                    }
                }
            }

            return hit;
        }

        Box boundingBox() const override { return aabb; }

};

class Light {
    private:
        POINT3 pos;
        VEC3 col;

    public:
        Light() {}

        Light(const POINT3& position, const VEC3& color) : pos(position), col(color) {}

        POINT3 position() const { return pos; }
        VEC3 color() const { return col; }
};


#endif