
#include "Model.h"

Model::Model(const char* path): position(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), rotation(glm::vec3(0.0f))
{
	loadModel(path);
	initAnimation();
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

void Model::update(float dt)
{

	if (m_playAnimation) {
		m_animator->updateAnimation(dt*12.0f);
	}
}

void Model::loadModel(const char *path)
{

	
	m_path = std::string(path);
	std::cout << "Loading Model: " << path << "\n";
	Assimp::Importer import;
	//aiPreTransformVertices flag gives wrong number of bones!!!!!!!!!!!
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	//AI_CONFIG_PP_PTV_NORMALIZE;
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
	
	//std::cout << mesh->mNumBones << "\n";
	for (int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;
		setVertexBoneDataToDefault(vertex);
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

	ExtractBoneWeightForVertices(vertices, mesh, scene);
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
	if (m_playAnimation) {
		setAnimationUniforms(shader);
	}
}

void Model::setVertexBoneDataToDefault(Vertex& vertex)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		vertex.BoneIDs[i] = -1;
		vertex.Weights[i] = 0.0f;
	}
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (m_boneInfoMap.find(boneName) == m_boneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_bonecounter;
			newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
				mesh->mBones[boneIndex]->mOffsetMatrix);
			m_boneInfoMap[boneName] = newBoneInfo;
			boneID = m_bonecounter;
			m_bonecounter++;
		}
		else
		{
			boneID = m_boneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		int max = 0;
		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			if (vertexId > max) {
				max = vertexId;
			}
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			//otherwise sets vertexId==0 weigths to 0sd
			if (vertexId == 0 && weight==0.0f) continue;
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
	{
		if (vertex.BoneIDs[i] < 0)
		{
			vertex.Weights[i] = weight;
			vertex.BoneIDs[i] = boneID;
			break;
		}
	}
}

void Model::setAnimationUniforms(Shader& shader)
{
	auto transforms = m_animator->getfinalBoneMatrices();
	for (int i = 0; i < transforms.size(); ++i) {
		shader.setUniformMat4f(("finalBonesMatrices[" + std::to_string(i) + "]").c_str(), transforms[i]);
	}
}

void Model::initAnimation()
{
	if (m_bonecounter != 0) {
		Animation animation(m_path, m_boneInfoMap, m_bonecounter);
		m_animations.push_back(animation);
		m_animator = std::make_unique<Animator>(Animator(&m_animations[0]));
		m_playAnimation = true;
	}
}

void Model::simpleDraw(Shader& shader)
{
	setUniforms(shader);
	for (auto& mesh : meshes) {
		mesh.SimpleDraw(shader);
	}
}

Animation::Animation(const std::string& animationPath, std::map<std::string, BoneInfo>& boneinfomap, int bonecount)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	assert(scene && scene->mRootNode);
	auto animation = scene->mAnimations[0];
	m_duration = animation->mDuration;
	m_ticksPerSecond = animation->mTicksPerSecond;
	aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
	globalTransformation = globalTransformation.Inverse();
	readHierarchyData(m_rootNode, scene->mRootNode);
	readMissingBones(animation, boneinfomap, bonecount);
}

Bone* Animation::findBone(const std::string& name)
{
	auto iter = std::find_if(m_bones.begin(), m_bones.end(),
		[&](const Bone& Bone)
		{
			return Bone.getBoneName() == name;
		}
	);
	if (iter == m_bones.end()) return nullptr;
	else return &(*iter);
}

void Animation::readMissingBones(const aiAnimation* animation, std::map<std::string, BoneInfo>& boneinfomap, int bonecount)
{
	int size = animation->mNumChannels;

	auto& boneInfoMap = boneinfomap;//getting m_BoneInfoMap from Model class
	int& boneCount = bonecount; //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			boneInfoMap[boneName].id = boneCount;
			boneCount++;
		}
		m_bones.push_back(Bone(channel->mNodeName.data,
			boneInfoMap[channel->mNodeName.data].id, channel));
	}

	m_boneInfoMap = boneInfoMap;
}

void Animation::readHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
	assert(src);

	dest.name = src->mName.data;
	dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		readHierarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}

Animator::Animator(Animation* animation) {
	m_currentTime = 0.0f;
	m_currentAnimation = animation;
	m_finalBoneMatrices.reserve(100);

	for (int i = 0; i < 100; ++i) {
		m_finalBoneMatrices.push_back(glm::mat4(1.0f));
	}
}

void Animator::updateAnimation(float dt) {
	m_deltatime = dt;
	if (m_currentAnimation) {
		m_currentTime += m_currentAnimation->getTicksPerSecond() * dt;
		m_currentTime = fmod(m_currentTime, m_currentAnimation->getDuration());
		calculateBoneTransform(&m_currentAnimation->getRootNode(), glm::mat4(1.0f));
	}
}

void Animator::playAnimation(Animation* pAnimation) {
	m_currentAnimation = pAnimation;
	m_currentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
	

	std::string nodeName = node->name;
	
	glm::mat4 nodeTransform = node->transformation;

	Bone* Bone = m_currentAnimation->findBone(nodeName);

	//std::cout << Bone->getBoneID() << "\n";
	if (Bone)
	{
		Bone->Update(m_currentTime);
		nodeTransform = Bone->getLocalTransform();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = m_currentAnimation->getBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		m_finalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
		calculateBoneTransform(&node->children[i], globalTransformation);
}