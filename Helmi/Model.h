#pragma once

#include <array>
#include <memory>
#include <map>

#include "Mesh.h"
#include "RTTriangle.h"
#include "rtTexture2D.h"
#include "stb_image.h"
#include "Texture2D.h"
#include "Animation.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"




class Model
{
public:
	Model(const char* path);

	void Draw(Shader& shader);
	void DrawPBR(Shader& shader);
	void imGuiControls(const std::string& name);
	void simpleDraw(Shader& shader);
	std::vector<helmirt::RTTriangle> trianglesToRT();
	auto& getBoneInfoMap() { return m_boneInfoMap; }
	int& getBoneCount() { return m_bonecounter; }

	std::vector<Mesh> meshes;

private:
	void loadModel(const char* path);
	void processNode(aiNode* node, const aiScene *scene);
	Mesh processMesh(aiMesh* mesh, const aiScene *scene);
	void setUniforms(Shader& shader);
	void setVertexBoneDataToDefault(Vertex& vertex);
	void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
	void SetVertexBoneData(Vertex& vertex, int boneID, float weight);

	//transform mesh data to raytracing format
	glm::vec3 position, scale, rotation;
	std::string m_path;

	//skeletal animation
	std::map<std::string, BoneInfo> m_boneInfoMap;
	int m_bonecounter = 0;
};


class Animation {
public:
	Animation() = default;
	Animation(const std::string& animationPath, Model* model);

	Bone* findBone(const std::string& name);

	size_t getBoneCount() const { 
		for (int i = 0; i < m_bones.size(); ++i) {
			std::cout << m_bones[i].getBoneID() << "\n";
		}
		return 3;
	}
	inline float getTicksPerSecond() const { return m_ticksPerSecond; }
	inline float getDuration() const { return m_duration;}
	inline const AssimpNodeData& getRootNode() { return m_rootNode; }
	inline const std::map<std::string, BoneInfo>& getBoneIDMap() { return m_boneInfoMap; }
private:

	void readMissingBones(const aiAnimation* animation, Model& model);
	void readHierarchyData(AssimpNodeData& dest, const aiNode* src);

	float m_duration;
	int m_ticksPerSecond;
	std::vector<Bone> m_bones;
	AssimpNodeData m_rootNode;
	std::map<std::string, BoneInfo> m_boneInfoMap;
};


class Animator {
public:
	Animator(Animation* animation);
	void updateAnimation(float dt);
	void playAnimation(Animation* pAnimation);
	void calculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
	std::vector<glm::mat4> getfinalBoneMatrices() { return m_finalBoneMatrices; };

private:

	std::vector<glm::mat4> m_finalBoneMatrices;
	Animation* m_currentAnimation = nullptr;
	float m_currentTime;
	float m_deltatime;
};


inline glm::vec3 color3DtoVec3(const aiColor3D& color) {
	return glm::vec3(color[0], color[1], color[2]);
}

