//
// Created by Krisu on 2019-12-06.
//

#include "Light.hpp"

Light PointLight(glm::vec3 position, glm::vec3 color) {
    Light light(LightType::Point);
    light.position = position;
    light.color = color;
    return light;
}

Light DirectionalLight(glm::vec3 direction, glm::vec3 color) {
    Light light(LightType::Directional);
    light.direction = direction;
    light.color = color;
    return light;
}

Light SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 color,
                float cone_angle_in_radian) {
    Light light(LightType::Spot);
    light.position = position;
    light.direction = direction;
    light.color = color;
    light.cone_angle_in_radian = cone_angle_in_radian;
    return light;
}