#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h> 
#include "stb_image.h"
#include "Shader.h"



class CubeMap
{
private:
	unsigned int textureID;
	int width, height, nrChannels;
	unsigned int VAO, VBO, EBO;

public:
	CubeMap(){}
	CubeMap(const std::string& dir);
	void bind();
	void draw(Shader shader, glm::mat4 projection, glm::mat4 view);
};

