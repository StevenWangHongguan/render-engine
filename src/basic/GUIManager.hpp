//
// Created by Krisu on 2020-02-05.
//

#ifndef RENDER_ENGINE_GUIMANAGER_HPP
#define RENDER_ENGINE_GUIMANAGER_HPP

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <basic/Renderer.hpp>

/*
 * Handling GUI things.
 * GUIManager is using imgui, while window is using glfw in implementation
 */

class GUIManager {
public:
    explicit GUIManager(const Renderer &renderer) : renderer(renderer) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(renderer.window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void drawUI() {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* Drawing UI... */

        ImGui::Render();

    };

private:
    const Renderer &renderer;
};


#endif //RENDER_ENGINE_GUIMANAGER_HPP