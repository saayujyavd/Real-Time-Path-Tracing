#ifndef RANDOM_H
#define RANDOM_H

#include <cstdlib>
#include <curand_kernel.h>

__device__ inline float randf(curandState state)
{
	return(curand_uniform(&state));
}

__device__ inline float randf(float min, float max, curandState state)
{
	return(min + (max - min) * curand_uniform(&state));
}

#endif
