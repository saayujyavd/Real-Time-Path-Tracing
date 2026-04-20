#ifndef SPHERE_H
#define SPHERE_H

#include "interval.h"

class Material;

class HitRecord
{
public:
	point3 p;
	vec3 normal;
	Material* mat;
	double t;
	bool front_face;

	__host__ __device__ void setFaceNorm(const Ray& r, const vec3& outward_norm)
	{
		front_face = dot(r.direction(), outward_norm) < 0;
		normal = front_face ? outward_norm : -outward_norm;
	}
};

class Sphere
{
private:
	Material* mat;
	point3 centre;
	float radius;

public:
	Sphere() : centre(point3(0, 0, 0)), radius(0.0f) {}
	Sphere(const point3& centre, float radius, Material* mat) : centre(centre), radius(radius), mat(mat) {}

	__device__ bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const
	{
		vec3 oc = centre - r.origin();
		auto a = r.direction().length_squared();
		auto h = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius * radius;

		auto discriminant = h * h - a * c;
		if (discriminant < 0) return(false);
		float sqrtd = sqrtf(discriminant);

		auto root = (h - sqrtd) / a;
		if (!ray_t.surrounds(root))
		{
			root = (h + sqrtd) / a;
			if (!ray_t.surrounds(root)) return(false);
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_norm = (rec.p - centre) / radius;
		rec.setFaceNorm(r, outward_norm);

		rec.mat = this->mat;
		if (rec.mat == nullptr) {
			// This shouldn't happen, but good for debugging
		}
		return(true);
	}
};

#endif
