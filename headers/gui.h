#ifndef IMGUI_H
#define IMGUI_H

#include "./Imgui/imconfig.h"
#include "./Imgui/imgui.h"
#include "./Imgui/imgui_impl_glfw.h"
#include "./Imgui/imgui_impl_opengl3.h"
#include "./Imgui/imgui_internal.h"

void setImgui(GLFWwindow* window) {
    // 1. Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 2. Setup Style (Optional but looks better)
    ImGui::StyleColorsDark();

    // 3. Setup Platform/Renderer backends
    // The "true" parameter allows ImGui to install its own mouse/key callbacks
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    // Use the version that matches your renderGLGraphics shader
    ImGui_ImplOpenGL3_Init("#version 330");
}

void defImguiMenu(Camera& cam) {
    glfwPollEvents(); // Handle mouse/keys first

    // Start ImGui Frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImVec4 purple = ImVec4(0.5f, 0.0f, 0.5f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_TitleBg, purple);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, purple);
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, purple);
    ImGui::PushStyleColor(ImGuiCol_Border, purple);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, purple);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, purple);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, purple);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, purple);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, purple);

    ImGui::Begin("Rendering Settings");
    ImGui::PopStyleColor(9);

    ImGui::SeparatorText("Sampling");
    ImGui::DragInt("Samples per Pixel", &cam.samples, 1, 1, 500, "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragInt("Max Depth", &cam.depth, 1, 1, 50, "%d", ImGuiSliderFlags_AlwaysClamp);

    ImGui::SeparatorText("Camera");
    ImGui::DragFloat("Focus Distance", &cam.focus_dist, 0.1f, 0.1f, 50.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Defocus Angle", &cam.defocus_angle, 0.1f, 0.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("FOV", &cam.vfov, 0.5f, 5.0f, 120.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Gamma RGB", &cam.rgb, 0.1f, 0.0f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("Move Speed", &cam_speed, 0.01f, 0.01f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    ImGui::SeparatorText("Performance");
    ImGui::Text("%.3f ms/frame (%.1f FPS)",
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void renderImgui(GLFWwindow* window) {
    // Unbind your scene texture BEFORE ImGui renders
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);  // also unbind PBO

    ImGui::Render();

    // 1. Save current GL state (Optional but safer)
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    // 2. THE FIX: Explicitly unbind everything from your raytracer
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 3. Force Blending (If this is off, text turns black/invisible)
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 4. Disable stuff that clips UI
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);

    // 5. Draw the UI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // 6. Final Swap
    glfwSwapBuffers(window);
}

#endif
