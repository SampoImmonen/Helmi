#include "Light.h"

DirectionalLight::DirectionalLight(const glm::vec3& direction) : direction(direction)
{
}

void DirectionalLight::setUniforms(Shader& shader)
{
    shader.setUniformVec3("dirLight.ambient", ambient);
    shader.setUniformVec3("dirLight.diffuse", diffuse);
    shader.setUniformVec3("dirLight.specular", specular);
    shader.setUniformVec3("dirLight.direction", direction);
}

void DirectionalLight::update(float ts)
{
    
}

glm::mat4 DirectionalLight::getLightSpaceMatrix(float scale)
{
    float near_plane = 0.1f, far_plane = scale*50.0f;
    glm::mat4 projection = glm::ortho<float>(-10.0f*scale, 10.0f*scale, -10.0f*scale, 10.0f*scale, near_plane, far_plane);
    glm::mat4 view = glm::lookAt(20.0f*scale*glm::normalize(direction), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return projection * view;
}

