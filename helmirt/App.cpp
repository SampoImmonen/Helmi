#include "App.h"


namespace helmirt {

	helmirt::App::App()
	{
		m_rtimage = RTImage(m_height, m_width);
		initApp();
	}

	helmirt::App::App(int height, int width) : m_height(height), m_width(width)
	{
		m_rtimage = RTImage(height, width);
		initApp();
	}

	bool helmirt::App::initApp()
	{
		std::cout << "initializing app\n";
		//setup camera default position (remove when we upgrade to dynamic camera)
		float aspect_ratio = (float)(m_width) / m_height;
		glm::vec3 cam_pos(-0.5f, 1.5f, 0.0f);
		glm::vec3 cam_dir(0, -1.5f, -5.0f);
		glm::vec3 up(0, 1.0f, 0);
		m_camera = Camera(cam_pos, cam_dir, up, 90.0f, aspect_ratio);

		//setup default scene (for now)
		//loadScene(MODELS + std::string("sponza.obj"));

		return false;
	}

	bool helmirt::App::loadScene(const std::string& modelpath)
	{
		ScopedTimer;
		m_renderer.loadScene(modelpath);
		glm::mat4 model(1.0f);
		model = glm::scale(model, glm::vec3(m_scale));
		m_renderer.transformTriangles(model);
		//add logic to load bvh from file if one exists (much faster for large scenes)
		m_renderer.constructBVH(SPATIAL_MEDIAN, 8);
		return false;
	}

	void App::loadTriangles(const std::vector<RTTriangle>& triangles)
	{
		m_renderer.loadTriangles(triangles);
	}

	void helmirt::App::renderRT()
	{
		PROFILE_FUNC();
		m_renderer.render(m_rtimage, m_camera);
	}

	void helmirt::App::saveRTImage(const std::string& filepath)
	{
		m_rtimage.toPPMFile(filepath);
	}
	void App::updateCamera(const glm::vec3& position, const glm::vec3& direction)
	{
		//update camera vectors
		m_camera.updateView(position, direction);

	}
}