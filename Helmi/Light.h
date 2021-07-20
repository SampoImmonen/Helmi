#pragma once

#include "Shader.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

class Light
{
public:
	virtual ~Light() {};
	virtual void setUniforms(Shader& shader) = 0;
	virtual void update(float ts) = 0;
	virtual float* get_direction() { return nullptr; }
	virtual glm::mat4 getLightSpaceMatrix(float scale = 1.0f) = 0;
	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
};

class DirectionalLight : public Light {

public:
	DirectionalLight(const glm::vec3& direction);
	void setUniforms(Shader& shader) override;
	void update(float ts) override;
	float* get_direction() override { return &direction[0]; }
	glm::mat4 getLightSpaceMatrix(float scale = 1.0f) override;
private:
	//direction points towards light
	glm::vec3 direction;

};

