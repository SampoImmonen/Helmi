#pragma once
#pragma once

#include <vector>
#include <algorithm>

#include "tiny_obj_loader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "Bvh.h"
#include "RTImage.h"
#include "RTTriangle.h"
#include "Camera.h"
#include "RayhitResult.h"
#include "Utils.h"



namespace helmirt {
	
	//modes for bvh node creation
	enum {
		SURFACE_AREA_HEURISTIC,
		SPATIAL_MEDIAN,
	};

	class Renderer
	{
	public:

		Renderer();
		~Renderer();

		void loadScene(const std::string& filepath);
		void loadTriangles(const std::vector<RTTriangle>& triangles);
		void transformTriangles(const glm::mat4& mat);
		void constructBVH(int mode=SURFACE_AREA_HEURISTIC, int leaf_node_max_tris=8);
		void constructBVHNode(BvhNode* N, std::vector<uint32_t>& indices, const std::vector<RTTriangle>& triangles, int start, int end, int& nodecount, int mode, int leaf_node_max_tris);
		RayhitResult rayTrace(const Ray& ray);
		RayhitResult naiveRayTrace(const Ray& ray);
		RayhitResult treeRayTrace(const Ray& ray);
		void render(RTImage& image, const Camera& cam);
		
		void bvhTraverse(BvhNode* N, const Ray& ray, int& imin, float& tmin, float& umin, float& vmin, const glm::vec3& invD);
		void printTris();


		//shading methods
		glm::vec3 headlightShading(const RayhitResult& rt);
		glm::vec3 normalShading(const RayhitResult& rt);


	private:
		std::vector<RTTriangle> m_triangles;
		Bvh m_bvh;

	};

}

