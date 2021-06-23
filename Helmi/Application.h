#pragma once

#include <string>
#include <memory>

#include <glad/glad.h> 
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Shader.h"
#include "CubeMap.h"
#include "App.h"
#include "Buffers.h"



struct ScreenVAO {


	unsigned int VAO, VBO, EBO;
	ScreenVAO() {

		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

};

struct SystemInfo {
	const unsigned char* driver_name;
	const char* glsl_version;
};

struct FpsInfo {
	float deltatime = 0;
	float lastFrame = 0;
	float lastTime = 0.0f;
	float fps = 0.0f;
	int nbFrames = 0;

	float update() {
		float currentFrame = glfwGetTime();
		deltatime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		nbFrames++;
		if (currentFrame - lastTime >= 1.0f) {
			lastTime += 1.0f;
			fps = 1000.0 / double(nbFrames);
			nbFrames = 0;
		}
		return fps;
	}
};

struct MouseInfo {
	bool firstMouse = true;
	float lastX, lastY;
};

enum ImageMode {
	Normal = 0,
	rtImage
};

class Application
{
public:

	Application();
	~Application();

	bool loadScene(const std::string& filepath);
	void startApplication();

	//callbacks
	void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void framebufferSizeCallback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow* window);

private:
	//initialization
	void initApp();
	void initGLFW();
	void initImGui();
	void initHelmirt();
	void setupCallbacks();

	//main funcs
	void render();
	void update();


	int m_width = 800, m_height = 600;
	Camera m_glcamera;
	GLFWwindow* m_window;
	ImGuiIO imgui_io;

	//helmirt
	helmirt::App app;

	//information
	FpsInfo m_fpsinfo;
	SystemInfo m_systeminfo;
	MouseInfo m_mouseinfo;

	//scene information
	std::vector<Shader> m_shaders;
	std::vector<std::unique_ptr<Light>> m_lights;
	std::vector<Model> m_models;
	CubeMap m_skybox;
	FrameBuffer m_fbo;

	//temporary solution
	unsigned int qVAO, qVBO;

	//imgui
	bool my_tool_active;

	bool show_rt = false;
	unsigned int getTextureId();

	//static wrapper class to handle glfw callbacks
	// (https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback)
	class GLFWCallbackWrapper {
	public:
		GLFWCallbackWrapper() = delete;
		GLFWCallbackWrapper(const GLFWCallbackWrapper&) = delete;
		GLFWCallbackWrapper(GLFWCallbackWrapper&&) = delete;
		~GLFWCallbackWrapper() = delete;

		static void MouseCallback(GLFWwindow* window, double positionX, double positionY);
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

		static void SetApplication(Application *application);

	private:
		static Application* s_application;
	};

};

