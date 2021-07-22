#include "Application.h"

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		abort();
	}
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

float quadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

void Application::GLFWCallbackWrapper::MouseCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->mouseCallback(window, positionX, positionY);
}

void Application::GLFWCallbackWrapper::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	s_application->framebufferSizeCallback(window, width, height);
}

void Application::GLFWCallbackWrapper::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	s_application->scrollCallback(window, xoffset, yoffset);
}

void Application::GLFWCallbackWrapper::SetApplication(Application* application)
{
	GLFWCallbackWrapper::s_application = application;
}

Application* Application::GLFWCallbackWrapper::s_application = nullptr;

Application::Application()
{
	
	initApp();
	loadScene(MODELS + std::string("sponza.obj"));
	m_skybox = CubeMap("textures/skybox/");
	m_fbo = FrameBuffer(m_width, m_height);
	m_shadowmap = ShadowMapBuffer(1024, 1024);

	glGenVertexArrays(1, &qVAO);
	glGenBuffers(1, &qVBO);
	glBindVertexArray(qVAO);
	glBindBuffer(GL_ARRAY_BUFFER, qVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}


Application::~Application()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

bool Application::loadScene(const std::string& filepath)
{
	//at the moment hardcoded
	m_models.push_back(Model(filepath.c_str()));
	//DirectionalLight dir(glm::vec3(-1.0f, -1.0f, 1.0f));
	std::unique_ptr<Light> ptr = std::make_unique<DirectionalLight>(DirectionalLight(glm::vec3(-2.0f, 4.0f, -1.0f)));
	std::unique_ptr<Light> ptr2 = std::make_unique<SpotLight>(SpotLight(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(2.0f, 0.0f, 2.0f)));
	
	m_lights.push_back(std::move(ptr));
	m_lights.push_back(std::move(ptr2));
	Shader shader("Shader.vert", "Shader.frag");
	m_shaders.push_back(shader);
	Shader skybox("SkyboxShader.vert", "SkyboxShader.frag");
	m_shaders.push_back(skybox);
	Shader imageShader("ImageShader.vert", "ImageShader.frag");
	m_shaders.push_back(imageShader);
	Shader shadowShader("ShadowDepthShader.vert", "ShadowDepthShader.frag");
	m_shaders.push_back(shadowShader);
	Shader showShadowShader("ShowShadowMap.vert", "ShowShadowMap.frag");
	m_shaders.push_back(showShadowShader);
	//load scene into rtformat
	initHelmirt();
	return true;
}

void Application::startApplication()
{
	std::cout << "starting main loop\n";
	while (!glfwWindowShouldClose(m_window)) {
		m_fpsinfo.update();
		update();
		render();
	}
}

void Application::initApp()
{
	std::cout << "Helmi Rendering engine 2020\n";
	initGLFW();
	//set glsl_version
	m_systeminfo.glsl_version = "#version 330";
	std::cout << m_systeminfo.driver_name << "\n";
	std::cout << "glsl version: "<< m_systeminfo.glsl_version << "\n";
	initImGui();

}

void Application::initGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	m_window = glfwCreateWindow(m_width, m_height, "Helmi", NULL, NULL);
	
	if (m_window == NULL) {
		std::cout << "Failed to create GLFW Window\n";
		std::cout << "exiting program\n";
		exit(3);
	}
	glfwMakeContextCurrent(m_window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD\n";
		exit(3);
	}
	m_systeminfo.driver_name = glGetString(GL_VENDOR | GL_VERSION);
	glViewport(0, 0, m_width, m_height);
	setupCallbacks();

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR);
	glfwSwapInterval(0);
}

void Application::initImGui()
{
	//IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	imgui_io = ImGui::GetIO(); (void)imgui_io;
	//imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//imgui_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(m_systeminfo.glsl_version);

}

void Application::initHelmirt()
{
	//load and transform triangels
	//currently supports single mesh
	app.loadTriangles(m_models[0].trianglesToRT());
	
	//transform harcoded at the moment
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(m_scale));
	app.m_renderer.transformTriangles(model);
	app.m_renderer.constructBVH(helmirt::SPATIAL_MEDIAN, 8);
	app.m_rtimage.createTexture();
}

void Application::setupCallbacks()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetFramebufferSizeCallback(m_window, GLFWCallbackWrapper::FramebufferResizeCallback);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MouseCallback);
	glfwSetScrollCallback(m_window, GLFWCallbackWrapper::scrollCallback);
}

void Application::render()
{	



	processInput(m_window);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(m_scale));
	//shadow map pass
	glEnable(GL_DEPTH_TEST);
	m_shaders[3].UseProgram();
	glm::mat4 lightMatrix = m_lights[1]->getLightSpaceMatrix(m_scale);
	m_shaders[3].setUniformMat4f("lightSpaceMatrix", lightMatrix);
	m_shaders[3].setUniformMat4f("model", model);
	m_shadowmap.bind();
	//m_models[0].Draw(m_shaders[0]);
	for (auto m : m_models[0].meshes) {
		m.SimpleDraw(m_shaders[3]);
	}
	m_shadowmap.unbind();
	//glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	//draw scene
	m_fbo.bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw skybox
	
	if (!m_show_shadowmap) {
		glm::mat4 projection = glm::perspective(glm::radians(m_glcamera.Zoom), (float)m_width / m_height, 0.1f, 100.0f);
		glm::mat4 view = m_glcamera.GetViewMatrix();
		m_skybox.draw(m_shaders[1], projection, view);
	

	
	//draw models
		m_shaders[0].UseProgram();
		m_lights[0]->setUniforms(m_shaders[0]);
		m_lights[1]->setUniforms(m_shaders[0]);
		m_shaders[0].setUniform1f("exposure", exposure);
		m_shaders[0].setUniformMat4f("lightSpaceMatrix", lightMatrix);
		m_shaders[0].setUniformMat4f("model", model);
		m_shaders[0].setUniformMat4f("projection", projection);
		m_shaders[0].setUniformMat4f("view", view);
		m_shaders[0].setUniformVec3("viewPos", m_glcamera.Position);
		m_shaders[0].setUniformInt("shadowMap", 3);
		m_shadowmap.bindDepthTexture(3);
		m_models[0].Draw(m_shaders[0]);
	}
	if (m_show_shadowmap) {
		m_shaders[4].UseProgram();
		glBindVertexArray(qVAO);
		glDisable(GL_DEPTH_TEST);
		m_shadowmap.bindDepthTexture(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	m_fbo.unbind();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//m_shaders[2].UseProgram();
	//GL_CHECK(glBindVertexArray(qVAO));
	//glDisable(GL_DEPTH_TEST);
	//m_fbo.bindColorTexture();
	//GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));

	//render imgui 
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(m_width, m_height));
	ImGui::Begin("Scene", &my_tool_active, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open...", "Ctrl+O")) {}
			if (ImGui::MenuItem("Save...", "Ctrl+S")) {}
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImVec2 wsize = ImGui::GetWindowSize();
	unsigned int textureId = getTextureId();
	ImGui::Image((ImTextureID)textureId, wsize, ImVec2(0, 1), ImVec2(1, 0));
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImGui::End();


	ImGui::Begin("control panel");
	ImGui::Text("Moi");
	ImGui::Text("fps %fms", m_fpsinfo.fps);
	ImGui::Text("%d", m_height);
	ImGui::Text("%d", m_width);
	ImGui::Checkbox("show rtimage", &show_rt);
	ImGui::Checkbox("show shadowmap", &m_show_shadowmap);
	ImGui::Text("%s", "Lights");

	if (ImGui::CollapsingHeader("light settings")){
		for (size_t i = 0; i < m_lights.size(); ++i) {
			m_lights[i]->ImGuiControls();
		}
	}
	if (ImGui::CollapsingHeader("general scene settings")){
		ImGui::SliderFloat("exposure", &exposure, 0.01f, 10.0f);
		ImGui::SliderFloat("scale", &m_scale, 1.0f, 20.0f);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void Application::update()
{
	//update state of scene objects etc...
	//no dynamic objects yet...
}

void Application::reloadShaders()
{
	Shader bl("Shader.vert", "Shader.frag");
	Shader showShadowShader("ShowShadowMap.vert", "ShowShadowMap.frag");
	m_shaders[0] = bl;
	m_shaders[4] = showShadowShader;
}

unsigned int Application::getTextureId()
{
	if (show_rt) return *app.m_rtimage.getTexture();
	return m_fbo.m_color;
}

void Application::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (m_mouseinfo.firstMouse) {
		m_mouseinfo.lastX = xpos;
		m_mouseinfo.lastY = ypos;
		m_mouseinfo.firstMouse = false;
	}

	m_glcamera.ProcessMouseMovement(xpos - m_mouseinfo.lastX, m_mouseinfo.lastY - ypos);
	m_mouseinfo.lastX = xpos;
	m_mouseinfo.lastY = ypos;

	//TODO update helmirt camera info;
	app.updateCamera(m_glcamera.Position, m_glcamera.Position + m_glcamera.Front);
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	m_glcamera.ProcessMouseScroll(yoffset);
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	m_height = height;
	m_width = width;
	glViewport(0, 0, m_width, m_height);
	m_fbo.Resize(m_width, m_height);
}

void Application::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		m_glcamera.ProcessKeyboard(FORWARD, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		m_glcamera.ProcessKeyboard(LEFT, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		m_glcamera.ProcessKeyboard(RIGHT, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		m_glcamera.ProcessKeyboard(BACKWARD, m_fpsinfo.deltatime);
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		std::cout << "TraceRay\n";
		app.renderRT();
		//app.saveRTImage("image.ppm");
		app.m_rtimage.updateTexture();
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		//show_rt = !show_rt;
	}
	

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		reloadShaders();
	}

	app.updateCamera(m_glcamera.Position, m_glcamera.Position + m_glcamera.Front);
}


