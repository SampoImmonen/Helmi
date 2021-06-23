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

void Mesh::Draw(Shader shader)
{

	shader.UseProgram();
	shader.setUniformVec3("material.ambient", m_material.ambient);
	shader.setUniformVec3("material.diffuse", m_material.diffuse);
	shader.setUniformVec3("material.specular", m_material.specular);
	shader.setUniformInt("material.hasDiffuse", false);
	shader.setUniformInt("material.DiffuseMap", 0);
	shader.setUniformInt("material.hasSpecular", false);
	shader.setUniformInt("material.DiffuseMap", 1);
	shader.setUniform1f("material.shininess", 32.0f);

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

	glBindVertexArray(0);

}
