
#include "Model.h"

Model::Model(const char* path)
{
	loadModel(path);
}

void Model::Draw(Shader shader)
{
	
	for (int i = 0; i < meshes.size(); ++i) {
		meshes[i].Draw(shader);
	}
}

std::vector<helmirt::RTTriangle> Model::trianglesToRT()
{
	//calculate number of triangles
	int num_triangles = 0;
	for (auto& m : meshes) {
		num_triangles += m.indices.size();
	}

	num_triangles = num_triangles / 3;
	std::vector<helmirt::RTTriangle> triangles;
	triangles.reserve(num_triangles);
	for (auto& m : meshes) {
		std::shared_ptr<helmirt::CMaterial> mat = std::make_shared<helmirt::CMaterial>();
		mat->ambient = m.m_material.ambient;
		mat->diffuse = m.m_material.diffuse;
		mat->specular = m.m_material.specular;
		mat->diffuse_tex = m.m_material.diffuse_map_name;
		mat->specular_tex = m.m_material.specular_map_name;
		mat->bump_tex = m.m_material.normal_map_name;
		for (int i = 0; i*3 < m.indices.size(); ++i) {
			std::array<glm::vec3, 3> vertices;
			std::array<glm::vec2, 3> texCoordinates;
			for (int j = 0; j < 3; ++j) {
				Vertex vertex = m.vertices[m.indices[3*i + j]];
				vertices[j] = vertex.Position;
				texCoordinates[j] = vertex.TexCoords;
			}

			helmirt::RTTriangle tri(vertices, texCoordinates);
			tri.m_material = mat;
			triangles.push_back(tri);
		}
	}

	return triangles;
}



void Model::loadModel(const char *path)
{

	
	std::cout << "Loading Model: " << path << "\n";
	

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene | scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR LOADING MODEL: " << path << "\n";
		std::cout << "ASSIMP::ERROR " << import.GetErrorString() << "\n";
	}

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	
	for (int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);

	}

	glMaterial mat;
	aiColor3D color(0.0f, 0.0f,0.0f);
	aiString textureName;
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat.diffuse = color3DtoVec3(color);
		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		mat.specular = color3DtoVec3(color);
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		mat.ambient = color3DtoVec3(color);

		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
		mat.diffuse_map_name = textureName.C_Str();

		material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), textureName);
		mat.specular_map_name = textureName.C_Str();
	}

	return Mesh(vertices, indices, mat);
}
