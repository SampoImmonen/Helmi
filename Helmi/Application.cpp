#include "Application.h"

//put these where???
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

//used to render texture quad to screen
const float quadVertices[] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

//used render rtLights
const float lightquadVertices[] = {
	-1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

	-1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
	 1.0f, 1.0f, 0.0f,  1.0f, 1.0f
};

//GLFW wrapper function hack
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
	loadScene(MODELS + std::string("shadowstest.obj"));
	m_skybox = CubeMap("textures/skybox/");
	m_fbo = FrameBuffer(m_width, m_height);
	m_hdrFBO = HDRFrameBuffer(m_width, m_height);
	m_pingpongBuffer = PingPongFrameBuffer(m_width, m_height);
	m_shadowmap = ShadowMapBuffer(1024, 1024);

	//make screenquad vao
	glGenVertexArrays(1, &qVAO);
	glGenBuffers(1, &qVBO);
	glBindVertexArray(qVAO);
	glBindBuffer(GL_ARRAY_BUFFER, qVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	//VAO for drawing ligthpositions
	glGenVertexArrays(1, &threeDqVAO);
	glGenBuffers(1, &threeDqVBO);
	glBindVertexArray(threeDqVAO);
	glBindBuffer(GL_ARRAY_BUFFER, threeDqVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightquadVertices), &lightquadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
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
	std::unique_ptr<Light> ptr3 = std::make_unique<PointLight>(PointLight(glm::vec3(0.0f, 5.0f, 0.0f)));
	m_lights.push_back(std::move(ptr));
	m_lights.push_back(std::move(ptr2));
	m_lights.push_back(std::move(ptr3));
	Shader shader("shaders/BlinnPhongShader.vert", "shaders/BlinnPhongShader.frag"); // 0
	m_shaders.push_back(shader);
	Shader skybox("shaders/SkyboxShader.vert", "shaders/SkyboxShader.frag");
	m_shaders.push_back(skybox);
	Shader imageShader("shaders/ImageShader.vert", "shaders/ImageShader.frag");
	m_shaders.push_back(imageShader);
	Shader shadowShader("shaders/ShadowDepthShader.vert", "shaders/ShadowDepthShader.frag");
	m_shaders.push_back(shadowShader);
	Shader showShadowShader("shaders/ShowShadowMap.vert", "shaders/ShowShadowMap.frag");
	m_shaders.push_back(showShadowShader);
	Shader postProcessingShader("shaders/ImageShader.vert", "shaders/PostProcessingBasic.frag");
	m_shaders.push_back(postProcessingShader);
	Shader bloomBlurShader("shaders/ImageShader.vert", "shaders/GaussianBlurShader.frag");
	m_shaders.push_back(bloomBlurShader);
	Shader areaLightShader("shaders/3DquadShader.vert", "shaders/3DquadShader.frag"); // 7
	m_shaders.push_back(areaLightShader);
	Shader pointShadowShader("shaders/PointLightDepthShader.vert", "shaders/PointLightDepthShader.frag", "shaders/PointLightDepthShader.geo"); //8
	m_shaders.push_back(pointShadowShader);
	Shader PBRShader("PBRShader.vert", "PBRShader.frag");
	m_shaders.push_back(PBRShader);
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

bool Application::loadModel(const std::string& filepath)
{
	//in the future used to load and add new models into the scene
	return false;
}

void Application::loadNewScene()
{
	//opens a file dialog and lets u choose a new model to show (windows file dialog)
	std::string path = FileHandler::openFilePath();
	if (!path.empty()) {
		std::cout << "loading model from: " << path << "\n";
		m_models[0] = Model(path.c_str());
	}
	// update rtTriangles as well??
}

void Application::addModel()
{
	std::string path = FileHandler::openFilePath();
	//add exception handling for non obj or fbx files
	m_models.push_back(Model(path.c_str()));
}

void Application::deleteModel(int i)
{
	m_models.erase(m_models.begin() + i);
}

void Application::initApp()
{
	std::cout << "Helmi Rendering engine 2020\n";
	initGLFW();
	//set glsl_version
	m_systeminfo.glsl_version = "#version 430";
	std::cout << m_systeminfo.driver_name << "\n";
	std::cout << "glsl version: "<< m_systeminfo.glsl_version << "\n";
	initImGui();

}

void Application::initGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//imgui_io = ImGui::GetIO(); (void)imgui_io;
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(m_systeminfo.glsl_version);

}

void Application::initHelmirt()
{
	//load and transform triangels
	//currently supports single model
	app.loadTriangles(m_models[0].trianglesToRT());
	
	//transform triangles into worldspace
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(m_scale));
	app.m_renderer.transformTriangles(model);
	// create bvh for the scene
	app.m_renderer.constructBVH(helmirt::SPATIAL_MEDIAN, 8);
	//init texture for the raytraced image to render on the screen, this has to be done after opengl has been initialized
	app.m_rtimage.createTexture();
}

void Application::setupCallbacks()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetFramebufferSizeCallback(m_window, GLFWCallbackWrapper::FramebufferResizeCallback);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MouseCallback);
	glfwSetScrollCallback(m_window, GLFWCallbackWrapper::scrollCallback);
}

void Application::ImGuiMouseCallback(const ImVec2& mousepos)
{
	m_mouseinfo.ImlastX = mousepos[0];
	m_mouseinfo.ImlastY = mousepos[1];
	//std::cout << m_mouseinfo.lastX << "\n";
	std::cout << mousepos[0] << " " << mousepos[1] << "\n";
}

void Application::ImGuiContentResizeCallback(const ImVec2& size)
{
	
	//size[0] is width and size[1] is height of content panel
	if ((size[0] != m_width) || (size[1] != m_height)) {
		m_width = size[0];
		m_height = size[1];
		glViewport(0, 0, m_width, m_height);
		m_fbo.Resize(m_height, m_width);
		m_hdrFBO.resize(m_height, m_width);
		m_pingpongBuffer.resize(m_height, m_width);
		app.m_rtimage.resize(m_height, m_width);
	}
}

void Application::render()
{	

	processInput(m_window);
	//shadow map pass
	updateShadowMaps();

	glm::mat4 projection = glm::perspective(glm::radians(m_glcamera.Zoom), (float)m_width / m_height, 0.1f, 100.0f);
	glm::mat4 view = m_glcamera.GetViewMatrix();
	m_hdrFBO.bind();
	if (!m_show_shadowmap) {
		if (!m_renderPBR) {
			renderScene(projection, view);
		}
		else {
			renderScenePBR(projection, view);
		}
		
	}
	if (m_show_shadowmap) {
		m_shaders[4].UseProgram();
		glBindVertexArray(qVAO);
		glDisable(GL_DEPTH_TEST);
		m_shaders[4].setUniformInt("screenTexture", 0);
		m_lights[0]->bindShadowMapTexture(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	m_hdrFBO.unbind();
	//blurring if bloom enabled
	if (m_bloomOn) {
		bloomBlur(m_shaders[6], m_numBloomIterations);
	}
	//post processing and render scene texture to screen
	renderToScreen();


	//Imgui
	//render pre render
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiID dockspaceId = ImGui::GetID("InvisibleWindowDockSpace"); 
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
	
	//imgui render
	ImGui::Begin("application", nullptr, windowFlags);
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Open...", "Ctrl+O")) { loadNewScene(); }
			if (ImGui::MenuItem("Save...", "Ctrl+S")) {}
			if (ImGui::MenuItem("Close", "Ctrl+W")) { my_tool_active = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//render scene into content panel
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
	ImGui::Begin("Scene");
	ImVec2 wsize = ImGui::GetContentRegionAvail();
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	unsigned int textureId = getTextureId();
	ImGui::Image((ImTextureID)textureId, wsize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();
	ImGui::PopStyleVar();
	
	//render control panel
	ImGui::Begin("control panel");

	
	ImGui::Text("frametime: %fms", m_fpsinfo.fps);
	ImGui::Text("%d", m_height);
	ImGui::Text("%d", m_width);
	ImGui::Checkbox("show rtimage", &show_rt);
	ImGui::Checkbox("show shadowmap", &m_show_shadowmap);
	ImGui::Checkbox("PBR shading", &m_renderPBR);
	ImGui::Text("%s", "Lights");

	if (ImGui::CollapsingHeader("light settings")){
		for (size_t i = 0; i < m_lights.size(); ++i) {
			m_lights[i]->ImGuiControls();
		}
	}
	if (ImGui::CollapsingHeader("general scene settings")){
		ImGui::SliderFloat("exposure", &exposure, 0.01f, 10.0f);
		ImGui::InputFloat("scale", &m_scale);
		ImGui::Checkbox("bloom", &m_bloomOn);
		if (m_bloomOn) {
			if (ImGui::CollapsingHeader("bloom settings")) {
				ImGui::InputInt("Number of blur iterations", &m_numBloomIterations);
				ImGui::SliderFloat("bloom threshold", &m_bloomThreshold, 0.2f, 1.5f);
				ImGui::Text("bloom brightness images");
				ImGui::Image((ImTextureID)m_hdrFBO.getBloomTexture(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
			}
		}
	}
	if (ImGui::CollapsingHeader("models")) {
		for (size_t i = 0; i < m_models.size(); ++i) {
			if (ImGui::CollapsingHeader(std::to_string(i).c_str())) {
				m_models[i].imGuiControls(std::to_string(i));
				if (ImGui::Button("delete")) { deleteModel(i); }
			}
		}
		
	}
	if (ImGui::BeginPopupContextItem("delete")) {
		if (ImGui::Button("add model")) { addModel(); }
		ImGui::EndPopup();
	}
	
	//add model popup
	if (ImGui::BeginPopupContextItem("add items")) {
		if (ImGui::Button("add model")) { addModel(); }
		ImGui::EndPopup();
	}
	ImGui::OpenPopupOnItemClick("add items", ImGuiPopupFlags_MouseButtonRight);
	if (ImGui::CollapsingHeader("rtProps")) {
		ImGui::Checkbox("draw rt-props", &m_drawrtprops);
		if (m_drawrtprops) {
			if (ImGui::TreeNode("rt-lights")) {
				ImGui::Text("moro");
				ImGui::Separator();
				ImGui::SliderFloat3("area light pos", &m_rtArealight.m_position[0], -5.0f, 5.0f);
				ImGui::SliderFloat3("area light normal", &m_rtArealight.m_normal[0], -5.0f, 5.0f);
				ImGui::SliderFloat2("area light size", &m_rtArealight.m_size[0], 0.3f, 5.0f);
				ImGui::TreePop();
			}

		}
		ImGui::Text("bvh stuff");
	}
	
	ImGui::End();
	ImGui::End();	
	//imgui post render
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	ImGuiContentResizeCallback(wsize);
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void Application::update()
{
	//update state of scene objects etc...
	//no dynamic objects yet...
}

void Application::bloomBlur(Shader& shader, int iterations)
{
	bool horizontal = true, first_iteration = true;
	shader.UseProgram();
	glBindVertexArray(qVAO);
	//glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	for (unsigned int i = 0; i < iterations; ++i) {
		m_pingpongBuffer.bindFrameBuffer(horizontal);
		shader.setUniformInt("horizontal", horizontal);
		if (first_iteration) {
			glBindTexture(GL_TEXTURE_2D, m_hdrFBO.getBloomTexture());
		}
		else{
			m_pingpongBuffer.bindTexture(!horizontal);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);
		horizontal = !horizontal;
		if (first_iteration) {
			first_iteration = false;
		}
	}
}

void Application::updateShadowMaps()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < m_lights.size(); ++i) {
		if (!m_lights[i]->castsShadows) continue;
		if (m_lights[i]->type == LightType::PointLight) {
			m_shaders[8].UseProgram();
			m_lights[i]->prepareShadowMap(m_shaders[8]);
			for (auto& model : m_models) {
				model.simpleDraw(m_shaders[8]);
			}
			
		}
		else{
			m_shaders[3].UseProgram();
			m_lights[i]->prepareShadowMap(m_shaders[3]);
			for (auto& model : m_models) {
				model.simpleDraw(m_shaders[3]);
			}
		}
	}
	//glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
}

void Application::renderScene(const glm::mat4& projection, const glm::mat4& view)
{
	m_hdrFBO.bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw skybox
	m_skybox.draw(m_shaders[1], projection, view);
	//draw models
	m_shaders[0].UseProgram();
	m_lights[0]->setUniforms(m_shaders[0]);
	m_lights[1]->setUniforms(m_shaders[0]);
	m_lights[2]->setUniforms(m_shaders[0]);
	//m_shaders[0].setUniform1f("exposure", exposure);
	m_shaders[0].setUniformMat4f("lightSpaceMatrixDirLight", m_lights[0]->getLightSpaceMatrix());
	m_shaders[0].setUniformMat4f("lightSpaceMatrixSpotLight", m_lights[1]->getLightSpaceMatrix());
	//m_shadowmap.bindDepthTexture(3);
	//m_shaders[0].setUniformMat4f("lightSpaceMatrixSpotLight", m_lights[1]->getLightSpaceMatrix());
	//m_shaders[0].setUniformMat4f("model", model);
	m_shaders[0].setUniformMat4f("projection", projection);
	m_shaders[0].setUniformMat4f("view", view);
	m_shaders[0].setUniformVec3("viewPos", m_glcamera.Position);
	m_shaders[0].setUniform1f("bloomThreshold", m_bloomThreshold);
	//m_shaders[0].setUniformInt("shadowMap", 3);
	//m_shadowmap.bindDepthTexture(3);
	for (auto& model : m_models) {
		model.Draw(m_shaders[0]);
	}
	if (m_drawrtprops) {
		//glDisable(GL_CULL_FACE);
		drawrtLights(projection, view);
	}
}

void Application::renderScenePBR(const glm::mat4& projection, const glm::mat4& view)
{

	std::vector<glm::vec3> lightpos = { glm::vec3(2, 2, 2), glm::vec3(-2, 2, 2), glm::vec3(-2, 2, -2), glm::vec3(2, 2, -2) };


	//render scene using PBR Shader and material
	m_hdrFBO.bind();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw skybox
	m_skybox.draw(m_shaders[1], projection, view);
	m_shaders[9].UseProgram();
	//set common uniforms
	m_lights[0]->setUniformsPBR(m_shaders[9]);
	m_shaders[9].setUniformMat4f("lightSpaceMatrixDirLight", m_lights[0]->getLightSpaceMatrix());
	m_lights[2]->setUniformsPBR(m_shaders[9]);
	m_shaders[9].setUniformMat4f("projection", projection);
	m_shaders[9].setUniformMat4f("view", view);
	m_shaders[9].setUniformVec3("viewPos", m_glcamera.Position);
	for (int i = 0; i < lightpos.size(); ++i) {
		m_shaders[9].setUniformVec3(("lightPositions[" + std::to_string(i) + "]").c_str(), lightpos[i]);
		m_shaders[9].setUniformVec3(("lightColors[" + std::to_string(i) + "]").c_str(), glm::vec3(100.0f));
	}
	for (auto& model : m_models) {
		model.DrawPBR(m_shaders[9]);
	}
}

void Application::renderToScreen()
{
	m_fbo.bind();
	m_shaders[5].UseProgram();
	m_shaders[5].setUniform1f("exposure", exposure);
	glBindVertexArray(qVAO);
	glDisable(GL_DEPTH_TEST);
	m_shaders[5].setUniformInt("screenTexture", 0);
	m_hdrFBO.bindTextures();
	m_shaders[5].setUniformInt("bloomOn", m_bloomOn);
	if (m_bloomOn) {
		m_shaders[5].setUniformInt("bloomBlur", 1);
		m_pingpongBuffer.bindTexture(0, 1);
	}
	//m_pingpongBuffer.bindTexture(0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	m_fbo.unbind();
}

void Application::reloadShaders()
{
	Shader blingPhongShader("shaders/BlinnPhongShader.vert", "shaders/BlinnPhongShader.frag");
	Shader showShadowShader("shaders/ShowShadowMap.vert", "shaders/ShowShadowMap.frag");
	Shader PBRShader("PBRShader.vert", "PBRShader.frag");
	m_shaders[0] = blingPhongShader;
	m_shaders[4] = showShadowShader;
	m_shaders[9] = PBRShader;
}

auto Application::getNativeWindow()
{
	return glfwGetWin32Window(m_window);
}

void Application::drawrtLights(const glm::mat4& projection, const glm::mat4& view)
{
	//initially just one arealight
	//later with proper rtLight interface all rtLight
	glBindVertexArray(threeDqVAO);
	m_shaders[7].UseProgram();
	m_shaders[7].setUniformMat4f("projection", projection);
	m_shaders[7].setUniformMat4f("view", view);
	m_rtArealight.calculateModelMatrix();
	glm::mat4 model = m_rtArealight.getModelMatrix();
	m_shaders[7].setUniformMat4f("model", model);
	m_shaders[7].setUniformVec3("color", m_rtArealight.getColor());
	glDrawArrays(GL_TRIANGLES, 0, 6);

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
	//currently not in use because resizing happens according imgui window size!!!

	//m_height = height;
	//m_width = width;
	//glViewport(0, 0, m_width, m_height);
	//m_fbo.Resize(m_width, m_height);
	//m_hdrFBO.resize(m_width, m_height);
	//app.m_rtimage.resize(height, width);
	//should also update rtImage size???
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
		app.m_renderer.m_arealight.setNormal(m_rtArealight.m_normal);
		app.m_renderer.m_arealight.setPosition(m_rtArealight.m_position);
		app.m_renderer.m_arealight.calculateModelMatrix();
		app.m_renderer.m_pointlight.setPosition(*m_lights[2]->getPosition());
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
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		m_rtArealight.setPosition(m_glcamera.Position);
		m_rtArealight.setNormal(m_glcamera.Front);
		m_rtArealight.calculateModelMatrix();
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
		float pdf;
		glm::vec3 point;
		m_rtArealight.sample(pdf, point, m_rng);
		std::cout << point[0] << " " << point[1] << " " << point[2] << "\n";
	}

	//update rtCamera vectors
	app.updateCamera(m_glcamera.Position, m_glcamera.Position + m_glcamera.Front);
}


