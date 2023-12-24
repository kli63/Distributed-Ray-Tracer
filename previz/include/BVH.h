#ifndef BVH_H
#define BVH_H
#include <vector>
#include <memory>

#include <vector>
#include <memory>
#include "PRIMITIVE.h"
#include "WORLD.h"
#include <algorithm>

class bvh_node : public Primitive {
    private:
        shared_ptr<Primitive> left;
        shared_ptr<Primitive> right;
        Box bbox;

        static bool box_compare(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b, int axis_index)
        {
            return a->boundingBox().axis(axis_index).min < b->boundingBox().axis(axis_index).min;
        }

        static bool compare_along_x(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b)
        {
            return box_compare(a, b, 0);
        }

        static bool compare_along_y(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b)
        {
            return box_compare(a, b, 1);
        }

        static bool compare_along_z(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b)
        {
            return box_compare(a, b, 2);
        }

    public:
        bvh_node(const Primitives& world): bvh_node(world.scene, 0, world.scene.size()) {}

        bvh_node(const vector<shared_ptr<Primitive>>& objects, size_t start, size_t end) {
            vector<shared_ptr<Primitive>> list = objects;

            int axis = rand() % 3;
            static bool (*compare_function)(const shared_ptr<Primitive> a, const shared_ptr<Primitive> b);
            if (axis == 0)
            {
                compare_function = compare_along_x;
            }
            else if (axis == 1)
            {
                compare_function = compare_along_y;
            }
            else
            {
                compare_function = compare_along_z;
            }

            size_t span = end - start;

            if (span == 1)
            {
                left = right = list[start];
            }
            else if (span == 2)
            {
                if (compare_function(list[start], list[start + 1]))
                {
                    left = objects[start];
                    right = objects[start + 1];
                }
                else
                {
                    left = objects[start + 1];
                    right = objects[start];
                }
            }
            else
            {
                sort(list.begin() + start, list.begin() + end, compare_function);

                int mid = start +  span / 2;
                // cout << mid << endl;
                left = make_shared<bvh_node>(list, start, mid);
                right = make_shared<bvh_node>(list, mid, end);
            }

            bbox = Box(left->boundingBox(), right->boundingBox());
        }

        bool hit(const Ray& r, double t0, double t1, hit_record& rec) const override {
                Interval t0t1(t0, t1);

                if (!bbox.hit(r, t0t1))
                {
                    return false;
                }

                // cout << "hit!" << endl;
                bool hit_left = false, hit_right = false;
                hit_record lrec, rrec;

                if (left != nullptr) {
                    hit_left = left->hit(r, t0, t1, lrec);
                }
                if (right != nullptr) {
                    hit_right = right->hit(r, t0, t1, rrec);
                }

                if (hit_left && hit_right) 
                {
                    rec = (lrec.t < rrec.t) ? lrec : rrec;
                    return true;
                } 
                else if (hit_left) 
                {
                    rec = lrec;
                    return true;
                } 
                else if (hit_right) 
                {
                    rec = rrec;
                    return true;
                } 
                else 
                {
                    return false;
                }
            }

        Box boundingBox() const override { return bbox; }

};

#endif