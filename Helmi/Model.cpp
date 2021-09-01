
#include "Model.h"

Model::Model(const char* path): position(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), rotation(glm::vec3(0.0f))
{
	loadModel(path);
}

void Model::Draw(Shader& shader)
{
	shader.UseProgram();
	setUniforms(shader);
	for (int i = 0; i < meshes.size(); ++i) {
		meshes[i].Draw(shader);
	}
}

void Model::DrawPBR(Shader& shader)
{
	shader.UseProgram();
	setUniforms(shader);
	for (int i = 0; i < meshes.size(); ++i) {
		meshes[i].DrawPBR(shader);
	}
}

void Model::imGuiControls(const std::string& name)
{
	
	ImGui::Text("transform");
	ImGui::InputFloat3("translation", &position[0]);
	ImGui::InputFloat3("rotation", &rotation[0]);
	ImGui::InputFloat3("scale", &scale[0]);
	
	if (ImGui::CollapsingHeader("meshes")) {
		ImGui::Text("show model meshes imguiControls");
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
		mat->normal_tex = m.m_material.normal_map_name;
		mat->glossiness = m.m_material.shininess;

		//create rtTextures
		if (!m.m_material.diffuse_map_name.empty()) {
			std::cout <<"creating diffuse rtTexture: " << std::string(MODELS + m.m_material.diffuse_map_name)<<"\n";
			int width, height, nChannels;
			stbi_set_flip_vertically_on_load(false);
			unsigned char* data = stbi_load(std::string(MODELS + m.m_material.diffuse_map_name).c_str(), &width, &height, &nChannels, 0);
			if (data!=nullptr){
				std::shared_ptr<rtTexture2D> tex = std::make_shared<rtTexture2D>(rtTexture2D(data, width, height, nChannels));
				mat->diffuse_map = tex;
			}
			stbi_image_free(data);
		}

		//normal maps
		if (!m.m_material.normal_map_name.empty()) {
			std::cout << "creating Normal rtTexture: " << std::string(MODELS + m.m_material.normal_map_name) << "\n";
			int width, height, nChannels;
			stbi_set_flip_vertically_on_load(false);
			unsigned char* data = stbi_load(std::string(MODELS + m.m_material.normal_map_name).c_str(), &width, &height, &nChannels, 0);
			if (data != nullptr) {
				std::shared_ptr<rtTexture2D> tex = std::make_shared<rtTexture2D>(rtTexture2D(data, width, height, nChannels));
				mat->normal_map = tex;
			}
			stbi_image_free(data);
		}

		//specular map
		if (!m.m_material.specular_map_name.empty()) {
			std::cout << "creating Specular rtTexture: " << std::string(MODELS + m.m_material.specular_map_name) << "\n";
			int width, height, nChannels;
			stbi_set_flip_vertically_on_load(false);
			unsigned char* data = stbi_load(std::string(MODELS + m.m_material.specular_map_name).c_str(), &width, &height, &nChannels, 0);
			if (data != nullptr) {
				std::shared_ptr<rtTexture2D> tex = std::make_shared<rtTexture2D>(rtTexture2D(data, width, height, nChannels));
				mat->specular_map = tex;
			}
			stbi_image_free(data);
		}

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
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices);
	AI_CONFIG_PP_PTV_NORMALIZE;
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

		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;

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
		material->Get(AI_MATKEY_SHININESS, mat.shininess);

		//make a method to load textures (code duplication)?????

		//fetch diffuse texture
		textureName = "";
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
		mat.diffuse_map_name = textureName.C_Str();
		if (!mat.diffuse_map_name.empty()) {
			mat.m_diffuse_map = std::make_shared<Texture2D>(Texture2D(MODELS + mat.diffuse_map_name));
		}

		//fetch specular texture
		//textureName.Clear();
		textureName = "";
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), textureName);
		mat.specular_map_name = textureName.C_Str();
		if (!mat.specular_map_name.empty()) {
			mat.m_specular_map = std::make_shared<Texture2D>(Texture2D(MODELS + mat.specular_map_name));
		}

		//fetch normal texture
		//textureName.Clear();
		textureName = "";
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_HEIGHT, 0), textureName);
		mat.normal_map_name = textureName.C_Str();
		if (!mat.normal_map_name.empty()) {
			mat.m_normal_map = std::make_shared<Texture2D>(Texture2D(MODELS + mat.normal_map_name));
		}

		textureName = "";
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0), textureName);
		mat.emission_map_name = textureName.C_Str();
		if (!mat.emission_map_name.empty()) {
			mat.m_emission_map = std::make_shared<Texture2D>(Texture2D(MODELS + mat.emission_map_name));
		}

		textureName = "";
		material->Get(AI_MATKEY_TEXTURE(aiTextureType_SHININESS, 0), textureName);
		mat.metalness_map_name = textureName.C_Str();
		if (!mat.metalness_map_name.empty()) {
			mat.m_metalness_map = std::make_shared<Texture2D>(Texture2D(MODELS + mat.metalness_map_name));
		}

	}

	return Mesh(vertices, indices, mat);
}

void Model::setUniforms(Shader& shader)
{
	glm::mat4 translationMatrix = glm::mat4(1.0f);
	translationMatrix = glm::translate(translationMatrix, position);

	glm::quat quat(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));
	glm::mat4 rotationMatrix = glm::toMat4(quat);
	
	glm::mat4 scaleMatrix = glm::mat4(1.0f);
	scaleMatrix = glm::scale(scaleMatrix, scale);
	shader.setUniformMat4f("model", translationMatrix*rotationMatrix*scaleMatrix);
}

void Model::simpleDraw(Shader& shader)
{
	setUniforms(shader);
	for (auto& mesh : meshes) {
		mesh.SimpleDraw(shader);
	}
}
