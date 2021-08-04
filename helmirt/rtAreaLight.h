#pragma once

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "glad/glad.h"

namespace helmirt {


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
	glm::vec3 getColor() const;
	//sample a point and pdf value from light
	glm::vec3 sample() const;

private:
	glm::vec3 m_position = glm::vec3(0.0f);
	glm::vec3 m_normal = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec2 m_size = glm::vec2(1.0f);
	glm::vec3 m_emission = glm::vec3(100.0f);
};

}
