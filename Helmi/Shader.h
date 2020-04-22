#pragma once
#include <glad/glad.h> 

#include <string>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

	Shader(const std::string &vpath, const std::string &fpath);
	~Shader();
	
	void UseProgram();
	void compileShaders(void);

	void setUniform4f(const char* name, float v0, float v1, float v2, float v3);
	void setInt(const char* name, int value);
	void setUniformMat4f(const char* name, const glm::mat4& mat);

private:

	const char* readfromFile(const std::string &path);
	std::string vertexPath;
	std::string fragmentPath;

	unsigned int VertexShader;
	unsigned int FragmentShader;

	unsigned int program;
};

