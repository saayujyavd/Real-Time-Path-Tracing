#ifndef HITTABLES_H
#define HITTABLES_H

#include<cstdarg>
#include "sphere.h"

#define SPHERES	484

class Hittables
{
public:
	Sphere objs[SPHERES];
	unsigned int idx = 0;

	Hittables() {}

	void add(const Sphere& obj) { objs[idx++] = obj; }

	__device__ bool hit(const Ray& r, Interval ray_t, HitRecord& rec) const
	{
		HitRecord temp_rec;
		bool hit_anything = false;
		auto closest_so_far = ray_t.max;

		for (unsigned int i = 0; i < idx; i++)
		{
			if (objs[i].hit(r, Interval(ray_t.min, closest_so_far), temp_rec))
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return(hit_anything);
	}
};

inline void allocWorld(Hittables** world1, ...)
{
	va_list world;
	va_start(world, world1);
	cudaMallocManaged(world1, sizeof(Hittables));

	Hittables** w;
	while ((w = va_arg(world, Hittables**)) != nullptr)
		cudaMallocManaged(w, sizeof(Hittables));

	va_end(world);
}

inline void freeWorld(Hittables* world1, ...)
{
	va_list world;
	va_start(world, world1);
	cudaFree(world1);

	Hittables* w;
	while ((w = va_arg(world, Hittables*)) != nullptr)
	{
		cudaFree(w);
		w = nullptr;
	}

	va_end(world);
}

#endif
