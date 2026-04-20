#ifndef SHADERS_H
#define SHADERS_H

#include "controls.h"

#pragma comment(lib, "C:/glfw-3.4.bin.WIN64/lib-vc2022/glfw3_mt.lib")
#pragma comment(lib, "C:/MyProjects/Requirements/glew/lib/Release/x64/glew32.lib")
#pragma comment(lib, "C:/MyProjects/Requirements/glew/lib/Release/x64/glew32s.lib")
#pragma comment(lib, "opengl32.lib")

const char* vert_src = R"(
    #version 430 core
    out vec2 uv;
    void main()
    {
        vec2 pos[3] = vec2[](vec2(-1,-1), vec2(3,-1), vec2(-1,3));
        gl_Position = vec4(pos[gl_VertexID], 0, 1);
        uv = pos[gl_VertexID] * 0.5 + 0.5;
    }
)";

const char* frag_src = R"(
    #version 430 core
    in vec2 uv;
    out vec4 out_color;
    uniform sampler2D tex;
    void main()
    {
        out_color = texture(tex, vec2(uv.x, 1.0 - uv.y));  // flip Y
    }
)";

GLuint compileShader()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert_src, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag_src, nullptr);
    glCompileShader(fs);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void initForGL(Camera& cam, GLFWwindow** window, GLuint& pbo, GLuint& texture, GLuint& shader, GLuint& vao)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    *window = glfwCreateWindow(cam.img_width, cam.img_height, "Ray Tracer", nullptr, nullptr);
    glfwMakeContextCurrent(*window);
    glewInit();

    glfwSetWindowUserPointer(*window, &cam);
    glfwSetCursorPosCallback(*window, mouseCB);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, cam.img_width, cam.img_height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenBuffers(1, &pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, cam.img_width * cam.img_height * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);

    shader = compileShader();
    glGenVertexArrays(1, &vao);
}

void renderGLGraphics(const Camera& cam, GLuint& pbo, GLuint& texture, GLuint& shader, GLuint& vao, GLFWwindow** window)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cam.img_width, cam.img_height, GL_RGB, GL_FLOAT, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Clean up GL state so ImGui can bind its own textures cleanly
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void unInitForGL(GLuint& pbo, GLuint& texture, GLuint& shader, GLuint& vao, GLFWwindow** window)
{
    glDeleteBuffers(1, &pbo);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(*window);
    glfwTerminate();
}

#endif
