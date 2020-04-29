#pragma once

#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(const char* path);

	void Draw(Shader shader);

private:
	std::vector<Mesh> meshes;
	std::string directory;

	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene *scene);
	Mesh processMesh(aiMesh* mesh, const aiScene *scene);


};
