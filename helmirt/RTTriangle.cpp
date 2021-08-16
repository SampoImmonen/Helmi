#include "RTTriangle.h"

namespace helmirt{

	helmirt::RTTriangle::RTTriangle(std::array<glm::vec3, 3> vertices) : m_vertices(vertices)
	{
		calculateNormal();
	}

	helmirt::RTTriangle::RTTriangle(std::array<glm::vec3, 3> vertices, std::array<glm::vec2, 3> txCoordinates) : m_vertices(vertices), m_txCoordinates(txCoordinates)
	{
		calculateNormal();
	}

	void helmirt::RTTriangle::calculateNormal()
	{
		m_normal = glm::normalize(glm::cross(m_vertices[1] - m_vertices[0], m_vertices[2] - m_vertices[0]));
	}

	void RTTriangle::calculateTangents()
	{
		glm::vec3 deltapos1 = m_vertices[1] - m_vertices[0];
		glm::vec3 deltapos2 = m_vertices[2] - m_vertices[0];

		glm::vec2 deltauv1 = m_txCoordinates[1] - m_txCoordinates[0];
		glm::vec2 deltauv2 = m_txCoordinates[2] - m_txCoordinates[0];

		float r = 1.0f / (deltauv1.x * deltauv2.y - deltauv1.y * deltauv2.x);
		m_tangent = (deltapos1 * deltauv2.y - deltapos2 * deltauv1.y) * r;
		m_bitangent = (deltapos2 * deltauv1.x - deltapos1 * deltauv2.x) * r;
		m_tangent = glm::normalize(m_tangent);
		m_bitangent = glm::normalize(m_bitangent);
	}

	glm::mat3 RTTriangle::getTBN() const 
	{
		return glm::mat3(m_tangent, m_bitangent, m_normal);
	}

	void helmirt::RTTriangle::applyTransform(const glm::mat4& mat)
	{
		for (auto& v : m_vertices) {
			v = glm::vec3(mat * glm::vec4(v, 1.0f));
		}
		calculateNormal();
		calculateTangents();
	}

	bool helmirt::RTTriangle::intersect(const Ray& ray, float& t, float& u, float& v) const
	{
		// Möller–Trumbore intersection
		float epsilon = 0.0000001f;

		glm::vec3 v0v1 = m_vertices[1] - m_vertices[0];
		glm::vec3 v0v2 = m_vertices[2] - m_vertices[0];

		glm::vec3 pvec = glm::cross(ray.dir, v0v2);
		float det = glm::dot(v0v1, pvec);

		if (std::abs(det) < epsilon) false;

		float invDet = 1 / det;

		glm::vec3 tvec = ray.orig - m_vertices[0];
		u = glm::dot(tvec, pvec) * invDet;
		if (u < 0 || u > 1.0f) return false;

		glm::vec3 qvec = glm::cross(tvec, v0v1);
		v = glm::dot(ray.dir, qvec) * invDet;
		if (v < 0 || u + v >1.0f) return false;

		t = glm::dot(v0v2, qvec) * invDet;

		return true;
	}

	inline glm::vec3 helmirt::RTTriangle::max() const
	{
		float x = std::max({ m_vertices[0].x, m_vertices[1].x, m_vertices[2].x });
		float y = std::max({ m_vertices[0].y, m_vertices[1].y, m_vertices[2].y });
		float z = std::max({ m_vertices[0].z, m_vertices[1].z, m_vertices[2].z });
		return glm::vec3(x, y, z);
	}

	inline glm::vec3 helmirt::RTTriangle::min() const
	{
		float x = std::min({ m_vertices[0].x, m_vertices[1].x, m_vertices[2].x });
		float y = std::min({ m_vertices[0].y, m_vertices[1].y, m_vertices[2].y });
		float z = std::min({ m_vertices[0].z, m_vertices[1].z, m_vertices[2].z });
		return glm::vec3(x, y, z);
	}

	inline float helmirt::RTTriangle::area() const
	{
		return glm::cross(m_vertices[1] - m_vertices[0], m_vertices[2] - m_vertices[0]).length() * 0.5f;
	}

	inline glm::vec3 helmirt::RTTriangle::centroid() const
	{
		return (m_vertices[0] + m_vertices[1] + m_vertices[2]) * (1.0f / 3.0f);
	}

	BoundingBox helmirt::RTTriangle::boundingbox() const
	{
		return BoundingBox(min(), max());
	}

	bool RTTriangle::hasTextureType(TextureType type) const
	{
		switch (type) {
		case diffuseTexture:
			return m_material->diffuse_map != nullptr;
		case normalTexture:
			return m_material->normal_map != nullptr;
		case specularTexture:
			return m_material->specular_map != nullptr;
		}
	}

}