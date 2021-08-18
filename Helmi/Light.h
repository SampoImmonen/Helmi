#pragma once

#include "Shader.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Buffers.h"
#include "CubeMap.h"

enum class LightType {
	DirectionalLight,
	PointLight,
	SpotLight,
};

class Light
{
public:
	virtual ~Light() {};
	virtual void setUniforms(Shader& shader) = 0;
	virtual void update(float ts) = 0;
	virtual float* get_direction() { return nullptr; }
	virtual float* get_size() { return nullptr; }
	virtual glm::mat4 getLightSpaceMatrix(float scale = 1.0f) = 0;
	virtual void ImGuiControls() = 0;
	virtual void prepareShadowMap(Shader& shader, float scale=1.0f) = 0;
	virtual void bindShadowMapTexture(int unit) = 0;
	virtual void bindShadowMap() = 0;
	virtual void unbindShadowMap() = 0;
	virtual glm::vec3* getPosition() { return nullptr; }
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
	LightType type;
	bool castsShadows = false;
	
};

class DirectionalLight : public Light {

public:
	DirectionalLight(const glm::vec3& direction);
	void setUniforms(Shader& shader) override;
	void update(float ts) override;
	float* get_direction() override { return &direction[0]; }
	float* get_size() override { return &size; }
	glm::mat4 getLightSpaceMatrix(float scale = 1.0f) override;
	void ImGuiControls() override;
	void prepareShadowMap(Shader& shader, float scale=1.0f) override;
	void bindShadowMapTexture(int unit) override;
	virtual void bindShadowMap() override;
	virtual void unbindShadowMap() override;
private:
	//direction points towards light
	glm::vec3 direction;
	//light size used for pcss soft shadows
	float size = 0.05f;
	ShadowMapBuffer m_shadowMap;
};

class SpotLight : public Light {
public:
	SpotLight() {};
	SpotLight(const glm::vec3& position, const glm::vec3 direction);
	void setUniforms(Shader& shader) override;
	void update(float ts) override;
	glm::mat4 getLightSpaceMatrix(float scale = 1.0f) override;
	void ImGuiControls() override;
	void prepareShadowMap(Shader& shader, float scale=1.0f) override;
	void bindShadowMapTexture(int unit) override;
	virtual void bindShadowMap() override;
	virtual void unbindShadowMap() override;

private:
	glm::vec3 m_position = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
	float getFarPlane(float scale);

	ShadowMapBuffer m_shadowMap;
	float size = 0.05f;
	//cutoff angles in degrees
	float m_cutOff = 25.5f, m_outerCutOff=27.5f;
	float m_constant=1.0f, m_linear=0.0f, m_quadratic=0.0f;
};

class PointLight : public Light {
public:
	PointLight() {};
	PointLight(const glm::vec3& position);
	void setUniforms(Shader& shader) override;
	void update(float ts) override;
	glm::mat4 getLightSpaceMatrix(float scale = 1.0f) override;
	void ImGuiControls() override;
	void prepareShadowMap(Shader& shader, float scale = 1.0f) override;
	void bindShadowMapTexture(int unit) override;
	virtual void bindShadowMap() override;
	virtual void unbindShadowMap() override;
	virtual glm::vec3* getPosition() override { return &m_position; }
private:
	glm::vec3 m_position = glm::vec3(0.0f, 1.0f, 0.0f);
	//attenuation factors
	float m_constant = 1.0f, m_linear = 1.5f, m_quadratic = 0.0f;
	DepthCubeMapFBO m_shadowMap;
	float m_near = 1.0f, m_far = 25.0f;
};