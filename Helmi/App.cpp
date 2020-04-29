#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Timer.h"

#include "Model.h"


Camera cam = Camera();


// define global variables;
float deltatime = 0;
float lastFrame = 0;

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
   
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
    }


}

// global 


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
    
    /*
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    */
    glm::mat4 projection;
    

    Shader shader("Shader.vert", "Shader.frag");
    Model Sphere("models/untitled.obj");
    Model plane("models/Plane.obj");
    Model cornell("models/cornell.obj");
    shader.UseProgram();
    shader.setUniformMat4f("projection", projection);
 

    //Main loop
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        
        
        processInput(window);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians((float)glfwGetTime()*100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f));


        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 15.0f, 0.0f));

        model2 = glm::scale(model2, glm::vec3(5.0f));

        projection = glm::perspective(glm::radians(cam.Zoom), 800.0f / 600.0f, 0.1f, 500.0f);
        shader.setUniformMat4f("model", model);
        shader.setUniformMat4f("projection", projection);
        shader.setUniformMat4f("view", cam.GetViewMatrix());
        
        float currentFrame = glfwGetTime();
        deltatime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Sphere.Draw(shader);
        shader.setUniformMat4f("model", model2);
        Sphere.Draw(shader);
        
        glm::mat4 model3 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f));
        model3 = glm::scale(model3, glm::vec3(30.0f));
        shader.setUniformMat4f("model", model3);
        plane.Draw(shader);

        glm::mat4 model4 = glm::translate(glm::mat4(1.0f), glm::vec3(60.0f, 0.0f, 0.0f));
        model4 = glm::scale(model4, glm::vec3(25.0f));
        shader.setUniformMat4f("model", model4);
        cornell.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

