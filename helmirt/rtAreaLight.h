#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"
#include "Random.h"

namespace helmirt {

/*
Light for ray traced rendering

Create unified light interface when path tracing architecture is clear
(until then lights are hardcoded for ray tracing)
*/

class rtAreaLight
{
public:
	rtAreaLight() {};
	~rtAreaLight() {};
	void setPosition(const glm::vec3& position) { m_position = position; }
	void setNormal(const glm::vec3& normal) { m_normal = normal; }
	glm::vec3 getPosition() const { return m_position; };
	glm::vec3 getNormal() const { return m_normal; }
	
	glm::mat4 getModelMatrix() const;
	void calculateModelMatrix();
	glm::vec3 getColor() const;
	//sadmple a point and pdf value from light
	void sample(float& pdf, glm::vec3& point, Random& rng) const;

	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_normal = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec2 m_size = glm::vec2(1.0f, 1.0f);

private:
	glm::vec3 m_emission = glm::vec3(100.0f);
	glm::mat4 m_modelmatrix = glm::mat4(1.0f);
};

class rtPointLight {

};

}
