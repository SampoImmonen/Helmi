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
