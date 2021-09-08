#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h> 
#include "stb_image.h"
#include "Shader.h"


//cubemap to draw environment maps
class CubeMap
{
private:
	unsigned int textureID;
	int width, height, nrChannels;
	unsigned int VAO, VBO, EBO;

public:
	CubeMap() = default;
	CubeMap(const std::string& dir);
	void bind();
	void draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view);
};

//used for pointlight shadow mapping
class DepthCubeMapFBO {
public:
	DepthCubeMapFBO() = default;
	DepthCubeMapFBO(int width, int height);
	void bind();
	void unbind();
	void bindDepthTexture(int unit);
	int getHeight() const { return m_height; }
	int getWidth() const { return m_width; }


private:
	unsigned int m_textureId, m_fboId;
	int m_width, m_height;
};

//cubemap used for IBL in PBR shading
class HDRCubeMap {
public:
	HDRCubeMap() = default;
	HDRCubeMap(const std::string& filepath, Shader& shader);

	void draw(Shader& shader, const glm::mat4& projection, const glm::mat4& view);
	void bindRectTexture(int unit);
	void bindEnvMapTexture(int unit);
	void bindIBLTexture(int unit);
	void bindPrefilterTexture(int unit);
	void bindBRDFLUT(int unit);
	void createBRDFLUT(Shader& shader);
	void convoluteCubeMap(Shader& shader);
	void createPrefilterMap(Shader& shader);
private:

	bool createRectTexture(const std::string& filepath);
	bool createCubeMapFBO();
	void createCubeMap(Shader& shader);
	
	

	unsigned int m_textureId;
	unsigned int m_captureFBO, m_captureRBO;
	unsigned int m_envCubemap, m_IBLcubeMap;
	unsigned int m_cubeVAO, m_cubeVBO;
	unsigned int m_quadVAO, m_quadVBO;
	unsigned int m_prefilterMap;
	unsigned int m_BRDFLUTtex;

};