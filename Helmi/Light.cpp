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
    shader.setUniformInt("dirLight.shadowMap", 7);
    m_shadowMap.bindDepthTexture(7);
}

void DirectionalLight::setUniformsPBR(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("dirlight.direction", direction);
    shader.setUniformVec3("dirlight.intensity", intensity);
    shader.setUniform1f("dirlight.size", size);
    shader.setUniformInt("dirlight.castShadows", castsShadows);
    shader.setUniformInt("dirlight.shadowMap", 7);
    m_shadowMap.bindDepthTexture(7);
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
        ImGui::InputFloat3("intensity", &intensity[0]);
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

void SpotLight::setUniformsPBR(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("spotlight.position", m_position);
    shader.setUniformVec3("spotlight.direction", m_direction);
    shader.setUniformVec3("spotlight.intensity", m_intensity);

    shader.setUniform1f("spotlight.cutOff", glm::cos(glm::radians(m_cutOff)));
    shader.setUniform1f("spotlight.outerCutOff", glm::cos(glm::radians(m_outerCutOff)));

    shader.setUniform1f("spotlight.constant", m_constant);
    shader.setUniform1f("spotlight.linear", m_linear);
    shader.setUniform1f("spotlight.quadratic", m_quadratic);

    shader.setUniform1f("spotlight.size", size);
    shader.setUniformInt("spotlight.castShadows", castsShadows);
    shader.setUniformInt("spotlight.shadowMap", 5);
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
        ImGui::SliderFloat3("position2", &m_position[0], -10.0f, 10.0f);
        ImGui::SliderFloat3("direction2", &m_direction[0], -5.0f, 5.0f);
    
        //colors
        ImGui::ColorEdit3("ambient", &ambient[0]);
        ImGui::ColorEdit3("diffuse", &diffuse[0]);
        ImGui::ColorEdit3("specular", &specular[0]);

        //cutoff
        ImGui::SliderFloat("inner cutoff", &m_cutOff, 1.0f, m_outerCutOff);
        ImGui::SliderFloat("outer Cutoff cutoff", &m_outerCutOff, 1.0f, 89.0f);

        //attenuation
        ImGui::SliderFloat("constant", &m_constant, 0.1f, 10.0f);
        ImGui::SliderFloat("linear", &m_linear, 0.0f, 5.0f);
        ImGui::SliderFloat("quadratic", &m_quadratic, 0.0f, 5.0f);

        //shadows
        ImGui::Checkbox("casts shadows", &castsShadows);
        ImGui::SliderFloat("light size", &size, 0.0f, 1.0f);
        ImGui::InputFloat3("intensity", &m_intensity[0]);
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

PointLight::PointLight(const glm::vec3& position)
{
    type = LightType::PointLight;
    m_shadowMap = DepthCubeMapFBO(1024, 1024);
}

void PointLight::setUniforms(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("pointLight.position", m_position);
    shader.setUniformVec3("pointLight.ambient", ambient);
    shader.setUniformVec3("pointLight.diffuse", diffuse);
    shader.setUniformVec3("pointLight.specular", specular);

    shader.setUniform1f("pointLight.constant", m_constant);
    shader.setUniform1f("pointLight.linear", m_linear);
    shader.setUniform1f("pointLight.quadratic", m_quadratic);

    shader.setUniformInt("pointLight.castShadows", castsShadows);
    shader.setUniform1f("pointLight.far_plane", m_far);
    shader.setUniformInt("pointLight.shadowMap", 6);
    m_shadowMap.bindDepthTexture(6);

}

void PointLight::setUniformsPBR(Shader& shader)
{
    shader.UseProgram();
    shader.setUniformVec3("pointlight.position", m_position);
    shader.setUniformVec3("pointlight.intensity", m_intensity);

    shader.setUniform1f("pointlight.constant", m_constant);
    shader.setUniform1f("pointlight.linear", m_linear);
    shader.setUniform1f("pointlight.quadratic", m_quadratic);
    //shader.setUniform1f("pointlight.size", m_size);

    shader.setUniformInt("pointlight.castShadows", castsShadows);
    shader.setUniform1f("pointlight.far_plane", m_far);
    shader.setUniformInt("pointlight.shadowMap", 6);
    m_shadowMap.bindDepthTexture(6);
}

void PointLight::update(float ts)
{
}

glm::mat4 PointLight::getLightSpaceMatrix(float scale)
{
    return glm::mat4();
}

void PointLight::ImGuiControls()
{
    if (ImGui::CollapsingHeader("PointLight")) {
        //position and direction
        ImGui::InputFloat3("position", &m_position[0]);
    
        //colors
        ImGui::ColorEdit3("ambient", &ambient[0]);
        ImGui::ColorEdit3("diffuse", &diffuse[0]);
        ImGui::ColorEdit3("specular", &specular[0]);

        ImGui::SliderFloat("constant", &m_constant, 0.1f, 10.0f);
        ImGui::SliderFloat("linear", &m_linear, 0.0f, 5.0f);
        ImGui::SliderFloat("quadratic", &m_quadratic, 0.0f, 5.0f);

        ImGui::InputFloat3("Intensity", &m_intensity[0]);
        ImGui::Checkbox("casts shadows", &castsShadows);
    }
}

void PointLight::prepareShadowMap(Shader& shader, float scale)
{
    //set pointlight shadow map shader uniforms
    shader.UseProgram();
    float aspectratio = (float)m_shadowMap.getWidth() / (float)m_shadowMap.getHeight();
    glm::mat4 shadowproj = glm::perspective(glm::radians(90.0f), aspectratio, m_near, m_far);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.reserve(6);
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowproj *
        glm::lookAt(m_position, m_position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
    //set uniforms
    for (unsigned int i = 0; i < 6; ++i) {
        shader.setUniformMat4f(("shadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);
    }
    shader.setUniformVec3("lightPos", m_position);
    shader.setUniform1f("far_plane", m_far);
    m_shadowMap.bind();
}

void PointLight::bindShadowMapTexture(int unit)
{
}

void PointLight::bindShadowMap()
{
}

void PointLight::unbindShadowMap()
{
}
