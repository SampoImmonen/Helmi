#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Texture2D.h"



struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
};


struct glMaterial {
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(0.0f);
	glm::vec3 specular = glm::vec3(0.0f);

	std::string diffuse_map_name;
	std::string specular_map_name;
	std::string normal_map_name;

	std::shared_ptr<Texture2D> m_diffuse_map;
	std::shared_ptr<Texture2D> m_specular_map;
	std::shared_ptr<Texture2D> m_normal_map;

};

class Mesh
{
public:
	
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, glMaterial mat);
	~Mesh();
	
	void Draw(Shader shader);
	void SimpleDraw(Shader shader);

	glMaterial m_material;
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};
