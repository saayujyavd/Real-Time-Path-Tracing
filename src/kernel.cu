#include <cuda_runtime.h>
#include "..\headers\kernel.h"

#pragma comment(lib, "cudart.lib")
#pragma comment(lib, "curand.lib")

__global__ void setupRand(curandState* state, int W, int H)
{
    int xidx = blockIdx.x * blockDim.x + threadIdx.x;
    int yidx = blockIdx.y * blockDim.y + threadIdx.y;

    if(xidx >= W || yidx >= H) return;
    
    int idx = yidx * W + xidx;
    curand_init(1234, idx, 0, &state[idx]);
}

// 1. The actual GPU Kernel
__global__ void myKernel(vec3* d_output, Camera* cam, Hittables* world, curandState* state)
{
    int xidx = blockIdx.x * blockDim.x + threadIdx.x;
    int yidx = blockIdx.y * blockDim.y + threadIdx.y;

    if (xidx < cam->img_width && yidx < cam->img_height)
    {
        int idx = yidx * cam->img_width + xidx;
        curandState local_state = state[idx];

        vec3 pix_col(0, 0, 0);

        for (int sample = 0; sample < cam->samples; ++sample)
        {
            Ray r = cam->getRay(xidx, yidx, &local_state);
            vec3 attenuation(1, 1, 1);
            vec3 self_illumination(0, 0, 0); // To catch the light

            for (int depth = 0; depth < cam->depth; ++depth)
            {
                HitRecord rec;
                bool has_hit;
                Ray scattered;

                // Keep your exact function
                vec3 v = cam->rayColor(r, world, has_hit, rec, scattered, &local_state);

                if (!has_hit) { 
                    // v is the Sky color. It provides the light.
                    self_illumination = attenuation * v; 
                    break; 
                }

                if (v == vec3(0, 0, 0)) { 
                    // Absorbed by material
                    self_illumination = vec3(0, 0, 0);
                    break; 
                }

                // v is the material reflectance (e.g., 0.5 grey)
                attenuation *= v;
                r = scattered;
            }

            pix_col += self_illumination;
        }

        d_output[idx] = pix_col * cam->pix_samples_scale;
        state[idx] = local_state;
    }
}

void launch(vec3* d_pixels, Camera* d_cam, Hittables* d_world, int W, int H)
{
    int threads_x = 16, threads_y = 16;

    dim3 threads(threads_x, threads_y);
    dim3 blocks((W + threads_x - 1) / threads_x, (H + threads_y - 1) / threads_y);

    static curandState* state = nullptr;
    if(state == nullptr)
    {
        cudaMalloc(&state, W * H * sizeof(curandState));
        setupRand<<<blocks, threads>>>(state, W, H);
    }

    myKernel<<<blocks, threads>>>(d_pixels, d_cam, d_world, state);
}

void initForCuda(Camera** d_cam, Camera& host_cam, Hittables** d_world, Hittables* host_world)
{
    cudaMallocManaged(d_cam, sizeof(Camera));
    **d_cam = host_cam;
    **d_world = *host_world;  // deep copy the whole struct including objs[] and idx
}

void regCudaGraphicsPbo(GLuint pbo, cudaGraphicsResource_t* cuda_pbo)
{
    cudaGraphicsGLRegisterBuffer(cuda_pbo, pbo, cudaGraphicsMapFlagsWriteDiscard);
}

void renderCudaGraphics(Camera* d_cam, Camera& host_cam, Hittables* d_world, cudaGraphicsResource_t* cuda_pbo)
{
    // Update the managed camera object with new host data (position, etc.)
    *d_cam = host_cam; 

    cudaGraphicsMapResources(1, cuda_pbo);
    vec3* d_pixels;
    size_t size;
    cudaGraphicsResourceGetMappedPointer((void**)&d_pixels, &size, *cuda_pbo);

    // Launch the kernel
    launch(d_pixels, d_cam, d_world, host_cam.img_width, host_cam.img_height);

    // Synchronize to catch errors and finish rendering before Unmap
    cudaDeviceSynchronize();

    cudaGraphicsUnmapResources(1, cuda_pbo);
}

void unInitForCuda(cudaGraphicsResource_t* cuda_pbo, Camera* d_cam, Hittables* d_world)
{
    cudaGraphicsUnregisterResource(*cuda_pbo);
    cudaFree(d_cam);
    cudaFree(d_world);
}
