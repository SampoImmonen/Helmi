#pragma once

#include "Shader.h"
#include "glm/glm.hpp"


class Light
{
public:
	virtual ~Light() {};
	virtual void setUniforms(Shader& shader) = 0;
	virtual void update(float ts) = 0;
	virtual float* get_direction() { return nullptr; }
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
private:
	glm::vec3 direction;

};

