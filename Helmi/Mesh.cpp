#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices){
	this->vertices = vertices;
	this->indices = indices;

	setupMesh();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, glMaterial mat)
{
	this->vertices = vertices;
	this->indices = indices;
	this->m_material = mat;

	setupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::Draw(Shader& shader)
{


	bool hasDiffusemap = !m_material.diffuse_map_name.empty();
	bool hasSpecularmap = !m_material.specular_map_name.empty();
	bool hasNormalmap = !m_material.normal_map_name.empty();
	bool hasEmissionMap = !m_material.emission_map_name.empty();

	shader.UseProgram();
	shader.setUniformVec3("material.ambient", m_material.diffuse);
	shader.setUniformVec3("material.diffuse", m_material.diffuse);
	shader.setUniformVec3("material.specular", m_material.specular);
	shader.setUniformVec3("material.specular", m_material.emission);
	shader.setUniformInt("material.hasDiffuse", hasDiffusemap);
	shader.setUniformInt("material.diffuseMap", 0);
	shader.setUniformInt("material.hasSpecular", hasSpecularmap);
	shader.setUniformInt("material.specularMap", 1);
	shader.setUniformInt("material.hasNormal", hasNormalmap);
	shader.setUniformInt("material.normalMap", 2);
	shader.setUniformInt("material.hasEmission", hasEmissionMap);
	shader.setUniformInt("material.emissionMap", 3);
	shader.setUniform1f("material.shininess", m_material.shininess);

	if (hasDiffusemap) {
		m_material.m_diffuse_map->bind(0);
	}
	if (hasSpecularmap) {
		m_material.m_specular_map->bind(1);
	}
	if (hasNormalmap) {
		m_material.m_normal_map->bind(2);
	}
	if (hasEmissionMap) {
		m_material.m_emission_map->bind(3);
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::SimpleDraw(Shader& shader)
{
	shader.UseProgram();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::DrawPBR(Shader& shader)
{
	shader.setUniformVec3("material.albedo", m_material.diffuse);
	shader.setUniform1f("material.metallic", m_material.metallic);
	shader.setUniform1f("material.roughness", m_material.roughness);
	shader.setUniform1f("material.ao", m_material.ao);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));


	glBindVertexArray(0);

}
