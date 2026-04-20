#ifndef VEC3_H
#define VEC3_H

#include <cstdio>
#include <cmath>
#include <curand_kernel.h>
#include <cuda_runtime.h>

inline float randFloat(void);
inline float rand_float(float, float);

class vec3 {
public:
    float e[3];

    __host__ __device__ vec3() : e{ 0,0,0 } {}
    __host__ __device__ vec3(float e0, float e1, float e2) : e{ e0, e1, e2 } {}

    __host__ __device__ float x() const { return e[0]; }
    __host__ __device__ float y() const { return e[1]; }
    __host__ __device__ float z() const { return e[2]; }

    __host__ __device__ vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    __host__ __device__ float operator[](int i) const { return e[i]; }
    __host__ __device__ float& operator[](int i) { return e[i]; }

    __host__ __device__ vec3& operator+=(const vec3& v) {
        e[0] += v.e[0]; e[1] += v.e[1]; e[2] += v.e[2];
        return *this;
    }
    __host__ __device__ vec3& operator*=(float t) {
        e[0] *= t; e[1] *= t; e[2] *= t;
        return *this;
    }

    __host__ __device__ vec3& operator*=(const vec3& v)
    {
        return(*this = vec3(e[0] * v.e[0], e[1] * v.e[1], e[2] * v.e[2]));
    }

    __host__ __device__ vec3& operator/=(float t) {
        return *this *= 1.0f / t;
    }

    __host__ __device__ bool operator==(const vec3& v)
    {
        return((v.e[0] == this->e[0]) && (v.e[1] == this->e[1]) && (v.e[2] == this->e[2]));
    }

    __host__ __device__ float length() const {
        return sqrtf(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
    }

    __host__ __device__ float length_squared() const {
        return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    __host__ __device__ bool nearZero() const
    {
        auto s = 1e-8;
        return((std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) && (std::fabs(e[2]) < s));
    }
};

using point3 = vec3;

inline FILE* operator<<(FILE* out, const vec3& v) {
    fprintf(out, "%f %f %f", v.e[0], v.e[1], v.e[2]);
    return out;
}

__host__ __device__ inline vec3 operator+(const vec3& u, const vec3& v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}
__host__ __device__ inline vec3 operator-(const vec3& u, const vec3& v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}
__host__ __device__ inline vec3 operator*(const vec3& u, const vec3& v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}
__host__ __device__ inline vec3 operator*(float t, const vec3& v) {
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
__host__ __device__ inline vec3 operator*(const vec3& v, float t) {
    return t * v;
}
__host__ __device__ inline vec3 operator/(const vec3& v, float t) {
    return (1.0f / t) * v;
}
__host__ __device__ inline float dot(const vec3& u, const vec3& v) {
    return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}
__host__ __device__ inline vec3 cross(const vec3& u, const vec3& v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
        u.e[2] * v.e[0] - u.e[0] * v.e[2],
        u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}
__host__ __device__ inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline float randFloat()
{
    return(std::rand() / (RAND_MAX + 1.0f));
}

inline vec3 random(float min, float max)
{
    return(vec3(rand_float(min, max), rand_float(min, max), rand_float(min, max)));
}

inline vec3 random()
{
    return(vec3(randFloat(), randFloat(), randFloat()));
}

inline float rand_float(float min, float max)
{
    return(min + (max - min) * randFloat());
}

__device__ static float random_float(curandState* localState, float min_val, float max_val)
{
    // Generates integers from A to B (inclusive)
    return(min_val + (curand_uniform(localState) * max_val - min_val));
}

__device__ inline vec3 randOnDisk(curandState* state)
{
    while (true)
    {
        auto p = vec3(random_float(state, -1.0f, 1.0f), random_float(state, -1.0f, 1.0f), 0.0f);
        if (p.length_squared() < 1.0f) return(p);
    }
}

__device__ static vec3 random(curandState* state, float min, float max)
{
    return(vec3(random_float(state, min, max), random_float(state, min, max), random_float(state, min, max)));
}

__device__ static vec3 randUnitVect(curandState* state)
{
    while (true)
    {
        auto p = random(state, -1.0f, 1.0f);
        auto lensq = p.length_squared();

        if (1e-160 < lensq && lensq <= 1.0f) return(p / sqrtf(lensq));
    }
}

__device__ static vec3 randOnHem(const vec3& normal, curandState* state)
{
    vec3 on_sphere = randUnitVect(state);
    if (dot(on_sphere, normal) > 0.0f) return(on_sphere);
    else return(-on_sphere);
}

__device__ inline vec3 reflect(const vec3& v, const vec3& n)
{
    return(v - 2 * dot(v, n) * n);
}

__device__ inline vec3 refract(const vec3& uv, const vec3& n, float refractive_idx)
{
    auto cos_theta = fminf(dot(-uv, n), 1.0f);
    vec3 r_perp = refractive_idx * (uv + cos_theta * n);
    vec3 r_parallel = -sqrt(fabsf(1.0f - r_perp.length_squared())) * n;

    return(r_perp + r_parallel);
}

#endif
