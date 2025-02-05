#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Texture2D.h"

constexpr unsigned int MAX_BONE_INFLUENCE = 4;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;

	//skeletal animation attributes
	int BoneIDs[MAX_BONE_INFLUENCE];
	float Weights[MAX_BONE_INFLUENCE];
};


struct glMaterial {
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
	glm::vec3 specular = glm::vec3(0.0f);
	glm::vec3 emission = glm::vec3(0.0f);

	std::string diffuse_map_name;
	std::string specular_map_name;
	std::string normal_map_name;
	std::string emission_map_name;
	std::string metalness_map_name;

	std::shared_ptr<Texture2D> m_diffuse_map;
	std::shared_ptr<Texture2D> m_specular_map;
	std::shared_ptr<Texture2D> m_normal_map;
	std::shared_ptr<Texture2D> m_emission_map;
	std::shared_ptr<Texture2D> m_metalness_map;

	//PBR
	float metallic = 0.7f;
	float roughness = 0.1f;
	float ao = 1.0f;

	float shininess = 1.0f;
};

class Mesh
{
public:
	
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, glMaterial mat);
	~Mesh();
	
	void Draw(Shader& shader);
	void SimpleDraw(Shader& shader);
	void DrawPBR(Shader& shader);

	glMaterial m_material;
private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};
