#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"
#include "hittables.h"
#include "random.h"
#include "material.h"

class Camera
{
private:
    vec3 defocus_disk_u, defocus_disk_v;

public:
    int img_width{ 1920 }, img_height, samples{ 1 }, depth{ 50 };
    float vfov{ 20.0f }, aspect_ratio{ 1.77778f }, viewport_width, viewport_height, defocus_angle{ 0.6f }, focus_dist{ 10.0f };
    float focal_len{ 1.0f }, pix_samples_scale, rgb{ 1.0f };
    vec3 viewport_u, viewport_v, pix_delta_u, pix_delta_v;
    vec3 viewport_upper_left, pix00_loc;
    vec3 vup = vec3(0, 1, 0);
    point3 centre = point3(0, 0, 0);
    point3 lookfrom = point3(13, 2, 3);
    point3 lookat = point3(0, 0, 0);

    __host__ __device__ Camera()
    {
        img_height = (int)(img_width / aspect_ratio);
        img_height = (img_height < 1) ? 1 : img_height;
        centre = lookfrom;

        // Build camera basis from lookfrom/lookat
        vec3 forward = unit_vector(lookat - lookfrom);
        vec3 right = unit_vector(cross(forward, vup));
        vec3 up = cross(right, forward);

        viewport_width = viewport_height * (float(img_width) / img_height);

        // Viewport spans right and -up directions
        viewport_u = viewport_width * right;
        viewport_v = viewport_height * (-up);

        pix_delta_u = viewport_u / img_width;
        pix_delta_v = viewport_v / img_height;

        // Upper left corner: go forward by focal_len, then back half viewport
        viewport_upper_left = centre - (focus_dist * forward) - viewport_u / 2.0f - viewport_v / 2.0f;
        pix00_loc = viewport_upper_left + 0.5f * (pix_delta_u + pix_delta_v);
    }

    // Call every frame after changing lookfrom/lookat
    __host__ void update()
    {
        centre = lookfrom;
        pix_samples_scale = 1.0f / samples;

        vec3 forward = unit_vector(lookat - lookfrom);
        vec3 right = unit_vector(cross(forward, vup));
        vec3 up = cross(right, forward);

        viewport_height = 2.0f * std::tanf(vfov * 0.00873f);
        viewport_width = viewport_height * (float(img_width) / img_height);

        viewport_u = viewport_width * right;
        viewport_v = viewport_height * (-up);

        pix_delta_u = viewport_u / img_width;
        pix_delta_v = viewport_v / img_height;

        viewport_upper_left = centre
            + focus_dist * forward
            - viewport_u / 2.0f
            - viewport_v / 2.0f;

        pix00_loc = viewport_upper_left + 0.5f * (pix_delta_u + pix_delta_v);

        // Calculate cam. defocus disk basis vects.
        auto defocus_rad = focus_dist * std::tanf(defocus_angle * 0.00873f);
        defocus_disk_u = right * defocus_rad;
        defocus_disk_v = up * defocus_rad;
    }

    __device__ point3 defocusDiskSample(curandState* state) const
    {
        // Returns a rand. pt. in the cam. defocus disk
        auto p = randOnDisk(state);
        return(centre + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v));
    }

    __device__ Ray getRay(int i, int j, curandState* state) const
    {
        auto offset = sampleSquare(state);
        auto pix_sample = pix00_loc
            + ((i + offset.x()) * pix_delta_u)
            + ((j + offset.y()) * pix_delta_v);

        auto ray_orig = (defocus_angle <= 0.0f) ? centre : defocusDiskSample(state);
        auto ray_dir = pix_sample - ray_orig;

        return Ray(ray_orig, ray_dir);
    }

    __device__ vec3 sampleSquare(curandState* state) const
    {
        return vec3(curand_uniform(state) - 0.5f,
            curand_uniform(state) - 0.5f,
            0.0f);
    }

    __device__ vec3 rayColor(const Ray& r, const Hittables* world, bool& has_hit, HitRecord& rec, Ray& scattered, curandState* state) const
    {
        // Ensure Interval is strictly positive to avoid self-intersection
        if (has_hit = world->hit(r, Interval(0.001f, FLT_MAX), rec))
        {
            vec3 attenuation_out;
            if (rec.mat->scatter(r, rec, attenuation_out, scattered, state))
                return attenuation_out; // Return the material reflectance
            return vec3(0, 0, 0); // Ray was absorbed (Black)
        }

        // Sky Color (Miss)
        vec3 dir_r = unit_vector(r.direction());
        float a = 0.5f * (dir_r.y() + 1.0f);
        return (1.0f - a) * vec3(1, 1, 1) + a * vec3(0.5f, 0.7f, 1.0f);
    }
};

#endif
