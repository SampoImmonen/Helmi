#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Timer.h"
#include "CubeMap.h"


#include "Model.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Camera cam = Camera();

const char* glsl_version = "#version 330";

int WIDTH = 800;
int HEIGHT = 600;

// define global variables;
float deltatime = 0;
float lastFrame = 0;

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;



struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float shininess;

	Texture2D diffuseMap;
	Texture2D specularMap;
	Texture2D emissiveMap;

	bool hasDiffuse = false;
	bool hasSpecular = false;
	bool hasEmissive = false;

	void bindDiffuse() {
		diffuseMap.bind(0);
	}

	void bindSpecular() {
		specularMap.bind(1);
	}

	void bindEmission() {
		emissiveMap.bind(2);
	}
	void bindTextures() {
		bindDiffuse();
		bindSpecular();
		bindEmission();
	}
};

struct Light {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 position = glm::vec3(0.0f);

	float constant = 1.0f;
	float linear = 0.001f;
	float quadratic = 0.0001f;
};

struct DirLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 direction;

    void setUniforms(Shader& shader) {
        shader.setUniformVec3("dirLight.ambient", ambient);
        shader.setUniformVec3("dirLight.diffuse", diffuse);
        shader.setUniformVec3("dirLight.specular", specular);
        shader.setUniformVec3("dirLight.direction", direction);
    }
};

struct SpotLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 position = glm::vec3(0.0f, 20.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);

    float constant = 1.0f;
    float linear = 0.0f;
    float quadratic = 0.0f;

    float cutOff = 12.5f;
    float outerCutOff = 17.5f;

    void setUniforms(Shader& shader) {
        shader.setUniformVec3("spotLight.ambient", ambient);
        shader.setUniformVec3("spotLight.diffuse", diffuse);
        shader.setUniformVec3("spotLight.specular", specular);

        shader.setUniformVec3("spotLight.position", position);
        shader.setUniformVec3("spotLight.direction", direction);

        shader.setUniform1f("spotLight.cutOff", glm::cos(glm::radians(cutOff)));
        shader.setUniform1f("spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));

        shader.setUniform1f("spotLight.constant", constant);
        shader.setUniform1f("spotLight.linear", linear);
        shader.setUniform1f("spotLight.quadratic", quadratic);
    }
};


void setMaterialUniforms(Shader &shader, Material &mat) {
    shader.setUniformVec3("material.ambient", mat.ambient);
    shader.setUniformVec3("material.diffuse", mat.diffuse);
    shader.setUniformVec3("material.specular", mat.specular);

	shader.setUniformInt("material.hasDiffuse", mat.hasDiffuse);
	shader.setUniformInt("material.DiffuseMap", 0);

	shader.setUniformInt("material.hasSpecular", mat.hasSpecular);
	shader.setUniformInt("material.specularMap", 1);
	
	mat.bindTextures();
	

    shader.setUniform1f("material.shininess", mat.shininess);

}

void setLightUniforms(Shader &shader, const Light& light) {
    shader.setUniformVec3("light.ambient", light.ambient);
    shader.setUniformVec3("light.diffuse", light.diffuse);
    shader.setUniformVec3("light.specular", light.specular);

    shader.setUniformVec3("light.position", light.position);
	
	shader.setUniform1f("light.constant", light.constant);
	shader.setUniform1f("light.linear", light.linear);
	shader.setUniform1f("light.quadratic", light.quadratic);

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    WIDTH = width;
    HEIGHT = height;
   
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    cam.ProcessMouseMovement(xpos - lastX, lastY - ypos);
    
    lastX = xpos;
    lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cam.ProcessMouseScroll(yoffset);
    
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.ProcessKeyboard(FORWARD, deltatime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.ProcessKeyboard(LEFT, deltatime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.ProcessKeyboard(RIGHT, deltatime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.ProcessKeyboard(BACKWARD, deltatime);
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        std::cout << "TraceRays:... to be done\n";
        std::cout << "CameraPos: " << cam.Position.x << ' ' <<cam.Position.y << ' ' <<cam.Position.z;
    }

}
 


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Helmi", NULL, NULL);


    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::cout << "Helmi rendering engine(2020)\n";
    const unsigned char* name = glGetString(GL_VENDOR | GL_VERSION);
    std::cout << "OPENGL:" << name <<"\n";

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    /*
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    */
    glm::mat4 projection;
    

    Shader shader("Shader.vert", "Shader.frag");
    
	Model Sphere("models/untitled.obj");
    Model plane("models/pwt.obj");
    //Model cornell("models/cornell.obj");
    
	shader.UseProgram();
    shader.setUniformMat4f("projection", projection);
    
	Texture2D tex2("textures/vangogh.jpg");
	Texture2D tex("textures/tree.jpg");
	//Texture2D tex3("texture/emission.jpg");
    CubeMap cubemap("textures/skybox/");
    Shader skyboxShader("SkyboxShader.vert", "SkyboxShader.frag");


    Material mat1;
    mat1.ambient = glm::vec3(0.0f);
    mat1.diffuse = glm::vec3(0.5f, 0.5f, 0.0f);
    mat1.specular = glm::vec3(0.6f, 0.6f, 0.5f);
    mat1.shininess = 0.25f*128;

	mat1.diffuseMap = tex;
	mat1.hasDiffuse = false;

    Material mat2;
    mat2.ambient = glm::vec3(0.0f);
    mat2.diffuse = glm::vec3(0.44f, 0.1f, 0.0f);
    mat2.specular = glm::vec3(1.0f);
    mat2.shininess = 8.0f;

	mat2.diffuseMap = tex;
	mat2.hasDiffuse = true;
	
	mat2.specularMap = tex2;
	mat2.hasSpecular = true;

    Light light;
    light.ambient = glm::vec3(0.2f);
    light.diffuse = glm::vec3(0.5f);
    light.specular = glm::vec3(1.0f);

    DirLight dirLight;
    dirLight.ambient = glm::vec3(0.2f);
    dirLight.diffuse = glm::vec3(0.5f);
    dirLight.specular = glm::vec3(1.0f);
    dirLight.direction = glm::vec3(0.5f, 0.5f, 0.0f);

    SpotLight spotLight;
    spotLight.ambient = glm::vec3(0.2f);
    spotLight.diffuse = glm::vec3(0.5f);
    spotLight.specular = glm::vec3(1.0f);
    

    float lastTime = 0.0;
    int nbFrames = 0;
    float fps = 0.0f;

    
    //Main loop
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {

        processInput(window);
        glfwPollEvents();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        deltatime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        nbFrames++;
        if (currentFrame - lastTime >= 1.0) {
            
            lastTime += 1.0;
            fps = 1000.0 / double(nbFrames);
            nbFrames = 0;
        }

        cubemap.draw(skyboxShader, projection, cam.GetViewMatrix());

        setLightUniforms(shader, light);
        dirLight.setUniforms(shader);
        spotLight.setUniforms(shader);
        
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(10.0f));
        
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 15.0f, 0.0f));
        model2 = glm::scale(model2, glm::vec3(5.0f));

        projection = glm::perspective(glm::radians(cam.Zoom), (float)WIDTH/HEIGHT, 0.1f, 500.0f);
        shader.setUniformMat4f("model", model);
        shader.setUniformMat4f("projection", projection);
        shader.setUniformMat4f("view", cam.GetViewMatrix());
        shader.setUniformVec3("viewPos", cam.Position);

        setMaterialUniforms(shader, mat1);
        Sphere.Draw(shader);
        shader.setUniformMat4f("model", model2);
        Sphere.Draw(shader);
        
        glm::mat4 model3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f));
        model3 = glm::scale(model3, glm::vec3(30.0f));
        shader.setUniformMat4f("model", model3);
        
        setMaterialUniforms(shader, mat2);
		plane.Draw(shader);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        

        ImGui::Begin("Control Panel");
        if (ImGui::Button("Ray Trace")) {
            mat1.specular = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        ImGui::Text("RenderTime %fms", fps);
		ImGui::SliderFloat3("mat 1 ambient", &mat1.ambient[0], 0.0f, 1.0f);
		ImGui::SliderFloat3("mat 1 diffuse", &mat1.diffuse[0], 0.0f, 1.0f);
		ImGui::SliderFloat3("mat 1 specular", &mat1.specular[0], 0.0f, 1.0f);
		ImGui::SliderFloat3("Light pos", &light.position[0], -20.0f, 20.0f);
        ImGui::SliderFloat3("dirLight", &dirLight.direction[0], -1.0f, 1.0f);
		ImGui::SliderFloat2("Attenuation", &light.linear, 0.0f, 0.2f);
        ImGui::SliderFloat("Mat2 shininess", &mat2.shininess, 0.0f, 512.0f);
        ImGui::SliderFloat3("Spotlight pos", &spotLight.position[0], -30.0f, 30.0f);
        ImGui::SliderFloat3("Spotlight dir", &spotLight.direction[0], -1.0f, 1.0f);
        ImGui::SliderFloat2("Spotlight cutOff", &spotLight.cutOff, 0.0f, 90.0f);
		ImGui::End();
        	
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	
    glfwTerminate();
    return 0;
}

