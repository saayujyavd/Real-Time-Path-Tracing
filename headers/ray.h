#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class Ray
{
private:
	point3 a;
	vec3 b;

public:
	__host__ __device__ Ray(void) {}
	__host__ __device__ Ray(const point3& orig, const vec3& dir) : a(orig), b(dir) {}

	__host__ __device__ const point3& origin() const { return(a); }
	__host__ __device__ const vec3& direction() const { return(b); }

	__device__ point3 at(double T) const
	{
		return(a + T * b);
	}
};

#endif
