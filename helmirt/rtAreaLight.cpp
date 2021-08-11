#include "rtAreaLight.h"

namespace helmirt {

	glm::mat4 rtAreaLight::getModelMatrix() const
	{
		return m_modelmatrix;
	}

	void rtAreaLight::calculateModelMatrix()
	{
		glm::vec3 col2 = glm::normalize(m_normal);
		glm::vec3 col0 = glm::normalize(glm::cross(glm::vec3(0, 1.0f, 0), col2));
		glm::vec3 col1 = glm::normalize(glm::cross(col2, col0));
		glm::mat4 model(1.0f);
		model[0] = glm::vec4(col0, 0);
		model[1] = glm::vec4(col1, 0);
		model[2] = glm::vec4(col2, 0);
		model[3] = glm::vec4(m_position, 1);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_size[0], m_size[1], 0));
		glm::mat4 modelmatrix = model * scale;
		m_modelmatrix = model * scale;
	}

	glm::vec3 rtAreaLight::getColor() const
	{
		return m_emission / 100.0f;
	}

	void rtAreaLight::sample(float& pdf, glm::vec3& point, Random& rng) const
	{
		//sample a random point from the area light
		//also return pdf value
		//to be done
		glm::vec2 random_point = rng.getVec2(-1.0f, 1.0f);
		glm::vec3 tangent = glm::vec3(m_modelmatrix[0]);
		glm::vec3 bitangent = glm::vec3(m_modelmatrix[1]);
		point = m_position + random_point[0] * tangent + random_point[1] * bitangent;
		pdf = 1.0f / (4.0f * m_size[0] * m_size[1]);

	}
	
}