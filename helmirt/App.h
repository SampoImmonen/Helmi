#pragma once

#include <string>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "RTImage.h"
#include "Renderer.h"
#include "ScopedTimer.h"
namespace helmirt {

	class App
	{
	public:
		App();
		App(int height, int width);
		~App() {};

		bool initApp();
		bool loadScene(const std::string& modelpath);
		void loadTriangles(const std::vector<RTTriangle>& triangles);
		void renderRT();
		void saveRTImage(const std::string& filepath);
		void updateCamera(const glm::vec3& position, const glm::vec3& direction);


		Renderer m_renderer;
		RTImage m_rtimage;

	private:

		int m_width = 800, m_height = 600;
		//currenly only support single object in scene
		glm::vec3 m_scale = glm::vec3(5.0f);
		Camera m_camera;
	};
}

