#pragma once

#include <glm/glm.hpp>
#include "math.h"
#include "Ray.h"

namespace helmirt {

	class Camera {
	public:

		Camera() {}

		Camera(
			glm::vec3 lookfrom,
			glm::vec3 lookat,
			glm::vec3 vup,
			float vfov,
			float aspect_ratio
		) {

			m_vfov = vfov;
			m_aspect_ratio = aspect_ratio;
			m_up = vup;
			m_origin = lookfrom;
			m_lookat = lookat;

			updateVectors();

		}

		Ray get_ray(float u, float v) const {
			return Ray(m_origin, m_lower_left_corner + u * m_horizontal + v * m_vertical - m_origin);
		}

		void updateView(const glm::vec3& lookfrom, const glm::vec3& lookat) {
			
			m_origin = lookfrom;
			m_lookat = lookat;
			updateVectors();
		}

		void updateAspectRatio(int height, int width) {

			m_aspect_ratio = (float)(width) / height;
			updateVectors();
		}


	private:

		float m_vfov;
		float m_aspect_ratio;
		float m_focal_length = 1.0f;

		glm::vec3 m_up;
		glm::vec3 m_origin;
		glm::vec3 m_lookat;
		
		glm::vec3 m_lower_left_corner;
		glm::vec3 m_horizontal;
		glm::vec3 m_vertical;

		void updateVectors() {
			auto theta = glm::radians(m_vfov);
			auto h = tan(theta / 2);
			auto viewport_height = 2.0f * h;
			auto viewport_width = m_aspect_ratio * viewport_height;


			auto w = glm::normalize(m_origin - m_lookat);
			auto u = glm::normalize(glm::cross(m_up, w));
			auto v = glm::cross(w, u);

			m_horizontal = viewport_width * u;
			m_vertical = viewport_height * v;
			m_lower_left_corner = m_origin - m_horizontal / 2.0f - m_vertical / 2.0f - w;
		}
	};

}
