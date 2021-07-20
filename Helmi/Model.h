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

class Model
{
public:
	Model(const char* path);

	void Draw(Shader shader);
	std::vector<helmirt::RTTriangle> trianglesToRT();


	std::vector<Mesh> meshes;
private:
	
	std::string directory;

	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene *scene);
	Mesh processMesh(aiMesh* mesh, const aiScene *scene);
	
	//transform mesh data to raytracing format
	

};

inline glm::vec3 color3DtoVec3(const aiColor3D& color) {
	return glm::vec3(color[0], color[1], color[2]);
}