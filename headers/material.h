#ifndef MATERIAL_H
#define MATERIAL_H

#include <cstdarg>
#include "sphere.h"
#include "ray.h"

class Material;

typedef vec3 Albedo;
typedef Material* pMaterial;

enum class MatType { Lambertian, Metal, Dielectric };

inline void allocMaterials(pMaterial, int);

class Material
{
private:
    Albedo albedo;
    MatType mat_type;
    float fuzz, refractive_idx;

    __device__ static float reflectance(float cos, float refractive_idx)
    {
        /* Use Schlick's approx. for reflectance */
        auto r0 = (1.0f - refractive_idx) / (1.0f + refractive_idx);
        r0 = r0 * r0;

        return(r0 + (1.0f - r0) * powf(1.0f - cos, 5.0f));
    }

public:
    Material() {}

    Material(MatType type, const Albedo& albedo, float fuzz = 0.0f, float refractive_idx = 0.0f) : mat_type(type), albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1), refractive_idx(refractive_idx) {}

    __device__ bool scatter(const Ray& r_in, const HitRecord& rec, vec3& attenuation, Ray& scattered, curandState* state = nullptr)
    {
        switch (mat_type)
        {
        case MatType::Lambertian:
        {
            vec3 scatt_dir = rec.normal + randUnitVect(state);
            if (scatt_dir.nearZero()) scatt_dir = rec.normal;
            scattered = Ray(rec.p, scatt_dir);
            attenuation = albedo;
            return true;
        }
        case MatType::Metal:
        {
            vec3 reflected = reflect(r_in.direction(), rec.normal);
            reflected = unit_vector(reflected) + (fuzz * randUnitVect(state));

            scattered = Ray(rec.p, reflected);
            attenuation = albedo;
            return(dot(scattered.direction(), rec.normal) > 0);
        }
        case MatType::Dielectric:
        {
            attenuation = vec3(1.0f, 1.0f, 1.0f);
            float ri = rec.front_face ? (1.0f / refractive_idx) : refractive_idx;

            vec3 unit_dir = unit_vector(r_in.direction()), direction;
            float cos_theta = fminf(dot(-unit_dir, rec.normal), 1.0f);
            float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

            bool cant_refract = ri * sin_theta > 1.0f;
            if (cant_refract || reflectance(cos_theta, ri) > curand_uniform(state))
                direction = reflect(unit_dir, rec.normal);
            else
                direction = refract(unit_dir, rec.normal, ri);

            scattered = Ray(rec.p, direction);
            return(true);
        }
        default:
            return false;
        }
    }
};

inline void allocMaterials(pMaterial mat[], int mats)
{
    for (int i = 0; i < mats; ++i)
        cudaMallocManaged(&mat[i], sizeof(Material));
}

inline void prefetchCuda(Hittables* d_world, pMaterial mat[], int mats)
{
    int device = 0, i = 0;
    cudaGetDevice(&device);

    if (d_world != nullptr) cudaMemPrefetchAsync(d_world, sizeof(Hittables), device, NULL);

    pMaterial m;
    while (i < mats)
    {
        m = mat[i++];
        cudaMemPrefetchAsync(m, sizeof(Material), device, NULL);
    }

    cudaDeviceSynchronize();    // Ensure the GPU is ready
}

inline void freeMaterials(pMaterial mat[], int mats)
{
    int i = 0;
    for (pMaterial m = mat[i]; i < mats; m = mat[++i])
    {
        cudaFree(m);
        m = nullptr;
    }
}

#endif
