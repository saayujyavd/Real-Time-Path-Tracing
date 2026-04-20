#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

const float infinity{ FLT_MAX };

class Interval
{
public:
	float min, max;

	__device__ Interval() : min(infinity), max(-infinity) {}	/* default interv. is empty */
	__host__ __device__ Interval(float min, float max) : min(min), max(max) {}

	float size() const
	{
		return(max - min);
	}

	bool contains(float x) const
	{
		return(min <= x && x <= max);
	}

	__host__ __device__ bool surrounds(float x) const
	{
		return(min < x && x < max);
	}

	float clamp(float x) const
	{
		if (x < min) return(min);
		if (x > max) return(max);

		return(x);
	}

	static const Interval empty, universe;
};
/*
const Interval Interval::empty = Interval(infinity, -infinity);
const Interval Interval::universe = Interval(-infinity, infinity);
*/
#endif
