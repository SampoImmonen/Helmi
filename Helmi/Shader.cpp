#include "Shader.h"

Shader::Shader(const std::string& vpath, const std::string& fpath)
{
	
	vertexPath = vpath;
	fragmentPath = fpath;
	compileShaders();
	
}

Shader::~Shader()
{
}

void Shader::UseProgram()
{
	glUseProgram(program);
}

void Shader::compileShaders(void)
{
	const char* vsource = readfromFile(vertexPath);
	const char* fsource = readfromFile(fragmentPath);
	std::cout << "\n---------------------------------\n";
	std::cout << "compiling shader:" << vertexPath << "\n";
	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &vsource, NULL);
	glCompileShader(VertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(VertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	std::cout << "compiling shader:" << fragmentPath << "\n";
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &fsource, NULL);
	glCompileShader(FragmentShader);

	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	program = glCreateProgram();
	glAttachShader(program, VertexShader);
	glAttachShader(program, FragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
}

void Shader::setUniform4f(const char* name, float v0, float v1, float v2, float v3)
{
	UseProgram();
	int loc = glad_glGetUniformLocation(program, name);
	glUniform4f(loc, v0, v1, v2, v3);
}

void Shader::setInt(const char* name, int value)
{
	int loc = glad_glGetUniformLocation(program, name);
	glUniform1i(loc, value);
}

void Shader::setUniformMat4f(const char* name, const glm::mat4& mat)
{
	unsigned int transformLoc = glGetUniformLocation(program, name);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

const char *Shader::readfromFile(const std::string& path)
{
	std::ifstream ifs(path.c_str());
	std::string str(std::istreambuf_iterator<char>{ifs}, {});
	const char* source = str.c_str();
	return source;
}
