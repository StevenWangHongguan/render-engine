#include <fstream>
#include <sstream>
#include <iostream>

#include "Scene.hpp"
#include "Renderer.hpp"
#include "Engine.hpp"
#include "Mesh.hpp"
#include "Transform.hpp"
#include "Material.hpp"
#include "Debug.hpp"
#include "IO.hpp"
#include "LightInformation.hpp"
#include "GlobalTransform.hpp"
#include "Light.hpp"


//skinnedMesh----------------------------------------------------
#include "skinnedMesh.hpp"


void processInput(Camera &camera) {
    float speed = 20.0f * Engine::GetInstance().GetRenderer().GetDeltaTime();
    if (io::KeyPress(Key::w)) {
        camera.Translate(camera.Front() * speed);
    }
    if (io::KeyPress(Key::s)) {
        camera.Translate(camera.Front() * -speed);
    }
    if (io::KeyPress(Key::a)) {
        camera.Translate(camera.Right() * -speed);
    }
    if (io::KeyPress(Key::d)) {
        camera.Translate(camera.Right() * speed);
    }
    if (io::KeyPress(Key::escape)) {
        Engine::GetInstance().GetRenderer().Close();
    }

    MousePos current = io::GetMousePosition();
    static MousePos last = current;
    if (io::MouseButtonClick(MouseButton::right)) {
        MousePos offset = current - last;
        camera.ProcessMouseMovement(offset.x, offset.y);
    }
    last = current;
}


int main(int argc, char *argv[]) {
    static_assert(std::is_base_of_v<Component, DirectionalLight>, "fuck");

    Engine& engine = Engine::GetInstance();

    engine.EnableUniformBuffer<LightInformation>();
    engine.EnableUniformBuffer<GlobalTransform>();

    Scene& scene = engine.CreateScene();
    engine.MakeCurrentScene(scene);

    // --1--
    GameObject& sphere = scene.CreateGameObject(); {
        sphere.CreateComponent<Mesh>(SimpleMesh::Sphere());
        auto& material = sphere.CreateComponent<Material>();
        material.SetShader(engine.GetDefaultShader());
        material.SetAlbedo(1, 1, 1);
        material.SetMetallic(0.1);
        material.SetRoughness(0.8);
        auto& transform = sphere.CreateComponent<Transform>();
        transform.SetPosition(0, 0, -10);
    }

    // --2--
    GameObject& ground = scene.CreateGameObject(); {
        ground.CreateComponent<Mesh>(SimpleMesh::Quad());
        auto& material = ground.CreateComponent<Material>();
        material.SetShader(engine.GetDefaultShader());
        material.SetAlbedo(1, 1, 1);
        material.SetMetallic(0.3);
        material.SetRoughness(0.5);
        auto& transform = ground.CreateComponent<Transform>();
        transform.SetPosition(0, -2, -10);
        transform.SetRotation(1, 0, 0, -90);
        transform.SetScale(10, 10, 10);
    }

    // --3--
    GameObject& lamp = scene.CreateGameObject(); {
        glm::vec3 light_color = glm::vec3 { 1, 1, 1 } * 2.0f;
        glm::vec3 light_position = glm::vec3{-1, 3, 1} * 5.0f;

        lamp.CreateComponent<Mesh>(SimpleMesh::Sphere());
        auto& material = lamp.CreateComponent<Material>();
        material.SetShader(engine.GetDefaultShader());
        material.SetEmissive(light_color);

        auto& transform = lamp.CreateComponent<Transform>();
        transform.SetPosition(glm::vec3{-1, 3, 1} * 5.0f);
        transform.SetScale(0.33);

        auto& light = lamp.CreateComponent<DirectionalLight>();
        light.SetColor(light_color);
        // TODO: fix with Transform
        light.position = light_position;
        light.direction = glm::vec3{0, 0, -10} - light.position;
    }

    // skinnedMesh----------------------------------------------------
    Shader bobShader("shader/skinnedMesh/bob.vert", "shader/skinnedMesh/bob.frag");
    SkinnedMesh bob;
    bob.LoadMesh("asset/bob/boblampclean.md5mesh");
    // skinnedMesh end----------------------------------------------------

    scene.CreateSkybox();

    scene.Build();

    Renderer& renderer = engine.GetRenderer();
    while (!renderer.ShouldEnd()) {
        renderer.UpdateBeforeRendering();
        processInput(scene.GetCurrentCamera());

        scene.Update();

        //bob render-------------------------------------------------------------
        bobShader.UseShaderProgram();
        glm::mat4 projection = glm::perspective(scene.GetCurrentCamera().GetFovy(), float(1280) / float(720), 0.1f, 1000.0f);
        glm::mat4 view = scene.GetCurrentCamera().GetViewMatrix();
        glm::mat4 model(1.0f);
        model = translate(model, glm::vec3(5.0, -2.0, -8));
        model = rotate(model, glm::radians(-40.0f), glm::vec3(0, 1, 0));
        model = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        model = scale(model, glm::vec3(0.05f));
        glm::mat4 MVP = projection * view * model;
        bobShader.Set("MVP", MVP);
        bobShader.Set("M", model);
        bobShader.Set("lightPos", glm::vec3{-1, 3, 1} * 5.0f);
        bobShader.Set("lightColor", glm::vec3 { 1, 1, 1 } * 2.0f);
        bobShader.Set("viewPos", scene.GetCurrentCamera().Position());

        std::vector<glm::mat4> transforms;
        bob.BoneTransform(static_cast<float>(glfwGetTime()), transforms);
        for (unsigned int i = 0; i < transforms.size(); i++) {
            bobShader.Set("gBones[" + std::to_string(i) + "]", transforms[i]);
        }
        bobShader.Set("diffuseTexture", 0);
        bob.Render();
        //bob render end----------------------------------------------------


        renderer.UpdateAfterRendering();
    }

    return 0;
}
