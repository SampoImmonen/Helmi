#include "Light.h"

DirectionalLight::DirectionalLight(const glm::vec3& direction) : direction(direction)
{
    type = LightType::DirectionalLight;
    castsShadows = true;
    m_shadowMap = ShadowMapBuffer(1024, 1024);
}

void DirectionalLight::setUniforms(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("dirLight.ambient", ambient);
    shader.setUniformVec3("dirLight.diffuse", diffuse);
    shader.setUniformVec3("dirLight.specular", specular);
    shader.setUniformVec3("dirLight.direction", direction);
    shader.setUniform1f("dirLight.size", size);
    shader.setUniformInt("dirLight.castShadows", castsShadows);
    shader.setUniformInt("dirLight.shadowMap", 4);
    m_shadowMap.bindDepthTexture(4);
}


void DirectionalLight::update(float ts)
{
    
}

glm::mat4 DirectionalLight::getLightSpaceMatrix(float scale)
{
    float near_plane = 0.1f, far_plane = scale*50.0f;
    glm::mat4 projection = glm::ortho<float>(-20.0f*scale, 20.0f*scale, -20.0f*scale, 20.0f*scale, near_plane, far_plane);
    glm::mat4 view = glm::lookAt(20.0f*scale*glm::normalize(direction), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    return projection * view;
}

void DirectionalLight::ImGuiControls()
{
    if (ImGui::CollapsingHeader("DirectionLight")){
        ImGui::SliderFloat3("direction", &direction[0], -5.0f, 5.0f);
        ImGui::ColorEdit3("ambient", &ambient[0]);
        ImGui::ColorEdit3("diffuse", &diffuse[0]);
        ImGui::ColorEdit3("specular", &specular[0]);
        ImGui::InputFloat("size", &size);
        ImGui::Checkbox("casts shadows", &castsShadows);
    }
}

void DirectionalLight::prepareShadowMap(Shader& shader, float scale)
{
    shader.UseProgram();
    shader.setUniformMat4f("lightSpaceMatrix", getLightSpaceMatrix(scale));
    m_shadowMap.bind();
}

void DirectionalLight::bindShadowMapTexture(int unit)
{
    m_shadowMap.bindDepthTexture(unit);
}

void DirectionalLight::bindShadowMap()
{
    m_shadowMap.bind();
}

void DirectionalLight::unbindShadowMap()
{
    m_shadowMap.unbind();
}

SpotLight::SpotLight(const glm::vec3& position, const glm::vec3 direction): m_position(position), m_direction(direction)
{
    type = LightType::SpotLight;
    m_shadowMap = ShadowMapBuffer(1024, 1024);
}

void SpotLight::setUniforms(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("spotLight.ambient", ambient);
    shader.setUniformVec3("spotLight.diffuse", diffuse);
    shader.setUniformVec3("spotLight.specular", specular);

    shader.setUniformVec3("spotLight.position", m_position);
    shader.setUniformVec3("spotLight.direction", m_direction);

    shader.setUniform1f("spotLight.cutOff", glm::cos(glm::radians(m_cutOff)));
    shader.setUniform1f("spotLight.outerCutOff", glm::cos(glm::radians(m_outerCutOff)));

    shader.setUniform1f("spotLight.constant", m_constant);
    shader.setUniform1f("spotLight.linear", m_linear);
    shader.setUniform1f("spotLight.quadratic", m_quadratic);
    shader.setUniform1f("spotLight.size", size);
    shader.setUniformInt("spotLight.castShadows", castsShadows);
    shader.setUniformInt("spotLight.shadowMap", 5);
    m_shadowMap.bindDepthTexture(5);

}

void SpotLight::update(float ts)
{
}

glm::mat4 SpotLight::getLightSpaceMatrix(float scale)
{
    float near_plane = 0.1f;
    float far_plane = getFarPlane(scale);
    glm::mat4 projection = glm::perspective(2.0f*glm::radians(m_outerCutOff), 1.0f, near_plane, far_plane);
    glm::mat4 view = glm::lookAt(m_position, m_position+m_direction, glm::vec3(0.0f,1.0f,0.0f));
    return projection * view;
}

void SpotLight::ImGuiControls()
{

    if (ImGui::CollapsingHeader("SpotLight")){

        //position and direction
        ImGui::SliderFloat3("position", &m_position[0], -10.0f, 10.0f);
        ImGui::SliderFloat3("direction", &m_direction[0], -5.0f, 5.0f);
    
        //colors
        ImGui::ColorEdit3("ambient", &ambient[0]);
        ImGui::ColorEdit3("diffuse", &diffuse[0]);
        ImGui::ColorEdit3("specular", &specular[0]);

        //cutoff
        ImGui::SliderFloat("inner cutoff", &m_cutOff, 1.0f, m_outerCutOff);
        ImGui::SliderFloat("outer Cutoff cutoff", &m_outerCutOff, 1.0f, 89.0f);

        ImGui::SliderFloat("constant", &m_constant, 0.1f, 10.0f);
        ImGui::SliderFloat("linear", &m_linear, 0.0f, 5.0f);
        ImGui::SliderFloat("quadratic", &m_quadratic, 0.0f, 5.0f);

        ImGui::Checkbox("casts shadows", &castsShadows);
        ImGui::SliderFloat("light size", &size, 0.0f, 1.0f);
    }
}

void SpotLight::prepareShadowMap(Shader& shader, float scale)
{
    shader.UseProgram();
    shader.setUniformMat4f("lightSpaceMatrix", getLightSpaceMatrix(scale));
    m_shadowMap.bind();
}

void SpotLight::bindShadowMapTexture(int unit)
{
    m_shadowMap.bindDepthTexture(unit);
}

void SpotLight::bindShadowMap()
{
    m_shadowMap.bind();
}

void SpotLight::unbindShadowMap()
{
    m_shadowMap.unbind();
}

float SpotLight::getFarPlane(float scale)
{
    return 100.0f;
}


