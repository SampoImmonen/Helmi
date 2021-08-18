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
#include "Random.h"
#include "rtAreaLight.h"



namespace helmirt {
	
	static float Pi = 3.14159265358979323846;
	static float invPi = 1 / Pi;

	//modes for bvh node creation
	enum {
		SURFACE_AREA_HEURISTIC,
		SPATIAL_MEDIAN,
	};

	struct ShadingResult {
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 normal;
		float glossiness;
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

		ShadingResult getShadingParameters(const RayhitResult& rt);

		//shading methods
		glm::vec3 headlightShading(const RayhitResult& rt);
		glm::vec3 normalShading(const RayhitResult& rt);
		glm::vec3 ambientOcclusionShading(const RayhitResult& rt, const Camera& cam, Random& rng);
		glm::vec3 whittedRayTracing(const RayhitResult& rt, const Camera& cam, Random& rng, int maxdepth = 3);

		rtAreaLight m_arealight;
		rtPointLight m_pointlight;

	private:
		std::vector<RTTriangle> m_triangles;
		Bvh m_bvh;
		unsigned int m_maxWhittedDepth = 3;
		unsigned int m_numofshadowsrays = 64;
	};

}

