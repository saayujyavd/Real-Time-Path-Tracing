#ifndef KERNEL_H
#define KERNEL_H

#include <GL/glew.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include "camera.h"

void initForCuda(Camera**, Camera&, Hittables**, Hittables*);
void regCudaGraphicsPbo(GLuint, cudaGraphicsResource_t*);
void renderCudaGraphics(Camera*, Camera&, Hittables*, cudaGraphicsResource_t*);
void unInitForCuda(cudaGraphicsResource_t*, Camera*, Hittables*);

#endif
