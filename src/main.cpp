#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>

#include "../headers/shaders.h"
#include "../headers/camera.h"
#include "../headers/kernel.h"
#include "../headers/material.h"
#include "../headers/gui.h"

#pragma comment(lib, "cudart.lib")
#pragma comment(lib, "cuda.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

#ifdef SPHERES
#undef SPHERES
#define SPHERES 484
#endif

int main()
{
    // 1. Allocate the world
    Hittables* world, * d_world;   // Declare d_world
    allocWorld(&world, &d_world, nullptr);

    // 2. Allocate Materials
    pMaterial mat[SPHERES];
    allocMaterials(mat, SPHERES);

    int mat_idx = 0;
    *mat[mat_idx] = Material(MatType::Lambertian, Albedo(0.5f, 0.5f, 0.5f));
    world->add(Sphere(point3(0.0f, -1000.0f, -0.0f), 1000.0f, mat[mat_idx++]));

    int sqrt_mats = std::sqrt(SPHERES / 4);
    for (int a = -sqrt_mats; a < sqrt_mats; ++a)
    {
        for (int b = -sqrt_mats; b < sqrt_mats; ++b)
        {
            auto choose_mat = randFloat();
            point3 center(a + 0.9f * randFloat(), 0.2f, b + 0.9f * randFloat());

            if ((center - point3(4, 0.2, 0)).length() > 0.9f)
            {
                if (choose_mat < 0.8f)
                {
                    auto albedo = random() * random();

                    *mat[mat_idx] = Material(MatType::Lambertian, albedo);
                    world->add(Sphere(center, 0.2, mat[mat_idx++]));
                }
                else if (choose_mat < 0.95f)
                {
                    auto albedo = random(0.5f, 1.0f);
                    auto fuzz = rand_float(0.0f, 0.5f);

                    *mat[mat_idx] = Material(MatType::Metal, albedo, fuzz);
                    world->add(Sphere(center, 0.2, mat[mat_idx++]));
                }
                else
                {
                    auto albedo = Albedo(1.0f, 1.0f, 1.0f);
                    *mat[mat_idx] = Material(MatType::Dielectric, albedo, 0.0f, 1.5f);
                    world->add(Sphere(center, 0.2, mat[mat_idx++]));
                }
            }
        }
    }
    auto albedo = Albedo(1.0f, 1.0f, 1.0f);
    *mat[mat_idx] = Material(MatType::Dielectric, albedo, 0.0f, 1.5f);
    world->add(Sphere(point3(0, 1, 0), 1.0f, mat[mat_idx++]));

    *mat[mat_idx] = Material(MatType::Lambertian, Albedo(0.4f, 0.2f, 0.1f));
    world->add(Sphere(point3(-4, 1, 0), 1.0f, mat[mat_idx++]));

    *mat[mat_idx] = Material(MatType::Metal, Albedo(0.7f, 0.6f, 0.5f), 0.0f);
    world->add(Sphere(point3(4, 1, 0), 1.0f, mat[mat_idx++]));

    GLFWwindow* window = nullptr;
    GLuint pbo, texture, shader, vao;
    Camera cam, *d_cam = nullptr;      // Declare d_cam
    
    initForGL(cam, &window, pbo, texture, shader, vao);
    initForCuda(&d_cam, cam, &d_world, world);

    cudaGraphicsResource_t cuda_pbo;
    regCudaGraphicsPbo(pbo, &cuda_pbo);

    setImgui(window);
    prefetchCuda(d_world, mat, 1);
    while (!glfwWindowShouldClose(window))
    {
        // Define UI
        defImguiMenu(cam);

        // Render Scene
        processKeys(window, &cam);
        renderCudaGraphics(d_cam, cam, d_world, &cuda_pbo);
        renderGLGraphics(cam, pbo, texture, shader, vao, &window);

        // Draw UI and Swap
        renderImgui(window);
    }

    unInitForCuda(&cuda_pbo, d_cam, d_world);
    unInitForGL(pbo, texture, shader, vao, &window);
    freeMaterials(mat, 1);
    freeWorld(world, d_world, nullptr);
    return(0);
}
