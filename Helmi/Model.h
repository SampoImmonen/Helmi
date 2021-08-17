#pragma once

#include <array>
#include <memory>

#include "Mesh.h"
#include "RTTriangle.h"
#include "rtTexture2D.h"
#include "stb_image.h"
#include "Texture2D.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Model
{
public:
	Model(const char* path);

	void Draw(Shader& shader);
	void imGuiControls(const std::string& name);
	void simpleDraw(Shader& shader);
	std::vector<helmirt::RTTriangle> trianglesToRT();
	std::vector<Mesh> meshes;

private:
	
	std::string directory;

	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene *scene);
	Mesh processMesh(aiMesh* mesh, const aiScene *scene);
	void setUniforms(Shader& shader);
	//transform mesh data to raytracing format
	glm::vec3 position, scale, rotation;
};

inline glm::vec3 color3DtoVec3(const aiColor3D& color) {
	return glm::vec3(color[0], color[1], color[2]);
}