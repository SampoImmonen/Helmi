#include "Renderer.h"


namespace helmirt {

	int longestdim(const BoundingBox& bb) {
		//returns the index of the longest dimension of a bounding box
		glm::vec3 dims = bb.max - bb.min;
		float max = 0;
		int index=0;
		for (int i = 0; i < 3; ++i) {
			if (dims[i] > max) {
				max = dims[i];
				index = i;
			}
		}
		return index;
	}

	inline glm::ivec2 getTexelCoords(const glm::vec2& uv, const glm::vec2& size) {
		float u, v, n;
		//uv coordinates flipped for opengl in model loading
		u = std::modf(uv[1], &n);
		v = std::modf(uv[0], &n);

		u = size[0] * abs(u);
		v = size[1] * abs(v);
		return glm::ivec2(u, v);
	}

	BoundingBox combineBoundingBoxes(const BoundingBox& bb_1, const BoundingBox& bb_2) {
		//create a bounding box for two bounding boxes
		glm::vec3 min(std::min(bb_1.min.x, bb_2.min.x), std::min(bb_1.min.y, bb_2.min.y), std::min(bb_1.min.z, bb_2.min.z));
		glm::vec3 max(std::max(bb_1.max.x, bb_2.max.x), std::max(bb_1.max.y, bb_2.max.y), std::max(bb_1.max.z, bb_2.max.z));
		return BoundingBox(min, max);
	}

	BoundingBox centroidBox(const BoundingBox& bb, const glm::vec3& v) {
		glm::vec3 min(std::min(bb.min.x, v.x), std::min(bb.min.y, v.y), std::min(bb.min.z, v.z));
		glm::vec3 max(std::max(bb.max.x, v.x), std::max(bb.max.y, v.y), std::max(bb.max.z, v.z));
		return BoundingBox(min, max);
	}


	struct BucketInfo {
		int count = 0;
		BoundingBox bb;
	};

	inline glm::vec3 bbOffset(const BoundingBox& bb, const glm::vec3& vec) {
		// calculates the offset of the vector from the min corner to the max corner
		// (0,0,0) means min corner and (1,1,1) means max corner
		glm::vec3 o = vec - bb.min;
		if (bb.max.x > bb.min.x) o.x /= bb.max.x - bb.min.x;
		if (bb.max.y > bb.min.y) o.y /= bb.max.y - bb.min.y;
		if (bb.max.z > bb.min.z) o.z /= bb.max.z - bb.min.z;
		return o;

	}


	helmirt::Renderer::Renderer()
	{
	}

	helmirt::Renderer::~Renderer()
	{
	}

	void helmirt::Renderer::loadScene(const std::string& filepath)
	{
		std::vector<RTTriangle> triangles;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		//std::cout << MODELS << "\n";
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str(), MODELS);

		if (!warn.empty()) {
			std::cout << warn << std::endl;
		}

		if (!err.empty()) {
			std::cerr << err << std::endl;
		}

		if (!ret) {
			exit(1);
		}

		std::vector<std::shared_ptr<CMaterial>> custom_materials;

		//init own material structs;
		for (auto& mat : materials) {
			auto cmat = std::make_shared<CMaterial>();
			cmat->ambient = listToVec3(mat.ambient);
			cmat->diffuse = listToVec3(mat.diffuse);
			cmat->specular = listToVec3(mat.specular);
			cmat->emission = listToVec3(mat.emission);
			cmat->glossiness = mat.shininess;
			cmat->ambient_tex = mat.ambient_texname;
			cmat->diffuse_tex = mat.diffuse_texname;
			cmat->specular_tex = mat.specular_texname;
			cmat->normal_tex = mat.bump_texname;
			custom_materials.push_back(cmat);
		}

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				std::array<glm::vec3, 3> vertices{};
				std::array<glm::vec2, 3> txCoordinates{};
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					vertices[v] = glm::vec3(vx, vy, vz);

					// Check if `normal_index` is zero or positive. negative = no normal data
					/*
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					}
					*/

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
						txCoordinates[v] = glm::vec2(tx, ty);
					}
					// Optional: vertex colors
					// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
				}

				index_offset += fv;

				// per-face material
				auto mat = shapes[s].mesh.material_ids[f];
				RTTriangle t(vertices, txCoordinates);
				if (custom_materials.size() > 0){
					t.m_material = custom_materials[mat];
				}
				triangles.push_back(t);
			}
		}

		m_triangles = triangles;
		std::cout << triangles.size() << "\n";
	}

	void Renderer::loadTriangles(const std::vector<RTTriangle>& triangles)
	{
		m_triangles = triangles;
		//constructBVH(SPATIAL_MEDIAN, 8);

	}


	void helmirt::Renderer::transformTriangles(const glm::mat4& mat)
	{
		for (auto& t : m_triangles) {
			t.applyTransform(mat);
		}
	}

	void helmirt::Renderer::constructBVH(int mode, int leaf_node_max_tris)
	{
		std::cout << "constructing BVH\n";
		uint32_t num_triangles = m_triangles.size();
		std::vector<uint32_t> indices(num_triangles);
		for (uint32_t i = 0; i < num_triangles; ++i) {
			indices[i] = i;
		}

		m_bvh = Bvh();
		m_bvh.rootNode = new BvhNode();
		int nodecount = 0;
		//recursively build bvh
		constructBVHNode(m_bvh.rootNode, indices, m_triangles, 0, num_triangles, nodecount, mode, leaf_node_max_tris);
		std::cout << nodecount << "\n";
		m_bvh.n_nodes = nodecount;
		m_bvh.indices = indices;

		//Flatten bvh tree for faster tracing
		m_bvh.initFlatBvh();
		int offset = 0;
		m_bvh.FlattenBvhTree(m_bvh.rootNode, &offset);

	}



	void helmirt::Renderer::constructBVHNode(BvhNode* N, std::vector<uint32_t>& indices, const std::vector<RTTriangle>& triangles, int start, int end, int& nodecount, int mode, int leaf_node_max_tris = 8)
	{
		nodecount++;
		std::shared_ptr<BvhNode> B = std::make_shared<BvhNode>();
		// construct BB for triangles in node;
		// also consruct BB for triangle centroids
		N->bb = BoundingBox();
		B->bb = BoundingBox();
		for (uint32_t i = start; i < end; ++i) {
			N->bb = combineBoundingBoxes(N->bb, triangles[indices[i]].boundingbox());
			B->bb = centroidBox(B->bb, triangles[indices[i]].boundingbox().centroid());
		}

		//set Node parameters
		N->start = start;
		N->end = end;

		//find longest dim
		int dim = longestdim(B->bb);
		N->splitAxis = dim;
		//if not leaf partition triangles according to spatial median
		if ((end - start) > leaf_node_max_tris) {
			//std::cout << (end - start) << "\n";
			uint32_t* splitIndex = nullptr;
			//partition triangle indices according spatial median
			float spatialMedian = B->bb.min[dim] + (B->bb.max[dim] - B->bb.min[dim]) / 2.0f;
			if (mode == SPATIAL_MEDIAN) {
				//Spatial Median
				splitIndex = std::partition(&indices[start], &indices[start] + (end - start), [&triangles, spatialMedian, dim](int i) {return triangles[i].boundingbox().centroid()[dim] > spatialMedian; });
			}
			else if (mode == SURFACE_AREA_HEURISTIC) {
				// SAH (Surface Area Heuristic)
				constexpr int nBuckets = 12;
				BoundingBox centroidBB = B->bb;
				// create buckets uniformingly separated inside the centroid bb
				BucketInfo buckets[nBuckets];
				// for each bucket calculate num of triangles and and bb of triangles
				for (uint32_t i = start; i < end; ++i) {
					int b = nBuckets * bbOffset(centroidBB, triangles[indices[i]].boundingbox().centroid())[dim];
					if (b == nBuckets) b = nBuckets - 1;
					buckets[b].count++;
					buckets[b].bb = combineBoundingBoxes(buckets[b].bb, triangles[indices[i]].boundingbox());
				}
				// loop through buckets calculate sah score
				float cost[nBuckets - 1];
				for (int i = 0; i < nBuckets - 1; ++i) {
					BoundingBox bb1, bb2;
					int count1 = 0, count2 = 0;
					// calculate bb and count of left tris
					for (int j = 0; j <= i; ++j) {
						bb1 = combineBoundingBoxes(bb1, buckets[j].bb);
						count1 += buckets[j].count;
					}
					// calculate bb and count of right tris
					for (int j = i + 1; j < nBuckets; ++j) {
						bb2 = combineBoundingBoxes(bb2, buckets[j].bb);
						count2 += buckets[j].count;
					}
					//calculate SAH score
					//cost[i] = 0.125f * (count1 * bb1.area()+count2*bb2.area())/N->bb.area();
					cost[i] = .125f + (count1 * bb1.area() + count2 * bb2.area()) / N->bb.area();
					//cost[i] = count1 * bb1.area() + count2 * bb2.area();
				}
				// select separation with best sah score
				float minCost = cost[0];
				int minIndex = 0;
				for (int i = 1; i < nBuckets - 1; ++i) {
					if (cost[i] < minCost) {
						minCost = cost[i];
						minIndex = i;
					}
				}

				float leafCost = end - start;
				//partition tris along best split
				splitIndex = std::partition(&indices[start], &indices[start] + (end - start), [=](uint32_t i) {
					int b = nBuckets * bbOffset(centroidBB, triangles[i].boundingbox().centroid())[dim];
					if (b == nBuckets) b = nBuckets - 1;
					return b <= minIndex;
					});

				if (leafCost < minCost) {
					return;
				}

			}

			//create child nodes
			uint32_t newEnd = std::distance(&indices[start], splitIndex);
			N->left = new BvhNode();
			constructBVHNode(N->left, indices, triangles, N->start, N->start + newEnd, nodecount, mode);
			N->right = new BvhNode();
			constructBVHNode(N->right, indices, triangles, N->start + newEnd, N->end, nodecount, mode);
		}
	}

	helmirt::RayhitResult helmirt::Renderer::rayTrace(const Ray& ray)
	{
		float tmin = 5000.0f, umin = 0.0f, vmin = 0.0f;
		int imin = -1;

		RayhitResult result;

		//init variable needed in calculations
		glm::vec3 invD = glm::vec3(1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z);
		int dirisNeg[3] = { invD.x < 0, invD.y < 0, invD.z < 0 };
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[128];

		while (true) {
			const FlatBvhNode* node = &m_bvh.flatnodes[currentNodeIndex];
			if (AABBintersect2(node->bb, ray.orig, invD)) {
				if (node->num_triangles > 0) {
					//leaf node intersect with tris
					float t, u, v;
					for (int i = node->start; i < node->start + node->num_triangles; ++i) {
						uint32_t t_index = m_bvh.getIndex(i);
						if (m_triangles[t_index].intersect(ray, t, u, v)) {
							if (t > 0.1f && t < tmin) {
								imin = t_index;
								tmin = t;
								umin = u;
								vmin = v;
							}
						}
					}
					if (toVisitOffset == 0) break;
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}
				else {
					if (dirisNeg[node->axis]) {
						nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
						currentNodeIndex = node->childOffset;
					}
					else {
						nodesToVisit[toVisitOffset++] = node->childOffset;
						currentNodeIndex = currentNodeIndex + 1;
					}

				}
			}
			else {
				if (toVisitOffset == 0) break;
				currentNodeIndex = nodesToVisit[--toVisitOffset];
			}
		}

		if (imin != -1) {
			result = RayhitResult(&m_triangles[imin], tmin, umin, vmin, ray.at(tmin), ray);
		}

		return result;
	}

	RayhitResult Renderer::naiveRayTrace(const Ray& ray)
	{
		float tmin = 5000.0f, umin = 0.0f, vmin = 0.0f;
		int imin = -1;
		RayhitResult result;

		for ( int i = 0; i < m_triangles.size(); ++i ){
			float t, u, v;
			if ( (m_triangles)[i].intersect(ray, t, u, v ) )
				{
					if ( t > 0.0f && t < tmin )
					{
						imin = i;
						tmin = t;
						umin = u;
						vmin = v;
						}
			}
		}
		if (imin != -1) {
			result = RayhitResult(&m_triangles[imin], tmin, umin, vmin, ray.at(tmin), ray);
		}

		return result;
	}

	RayhitResult Renderer::treeRayTrace(const Ray& ray)
	{
		float tmin = 5000.0f, umin = 0.0f, vmin = 0.0f;
		int imin = -1;
		RayhitResult result;
		glm::vec3 invD = glm::vec3(1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z);
		bvhTraverse(m_bvh.rootNode, ray, imin, tmin, umin, vmin, invD);
		if (imin != -1) {
			result = RayhitResult(&m_triangles[imin], tmin, umin, vmin, ray.at(tmin), ray);
		}

		return result;
	}



	void helmirt::Renderer::render(RTImage& image, const Camera& cam)
	{

		int height = image.getHeight();
		int width = image.getWidth();

		#pragma omp parallel for
		for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {

				auto u = float(i) / (width - 1);
				auto v = float(j) / (height - 1);
				Ray ray = cam.get_ray(u, v);

				//RayhitResult result = rayTrace(ray);
				RayhitResult result = rayTrace(ray);

				glm::vec3 color = glm::vec3(0.1f, 0.7f, 0.1f);
				if (result.tri != nullptr) {
					color = headlightShading(result);
					//color = normalShading(result);
				}

				image.setColor(i, j, color);

			}
		}

	}

	void Renderer::bvhTraverse(BvhNode* N, const Ray& ray, int& imin, float& tmin, float& umin, float& vmin, const glm::vec3& invD)
	{
		if ((AABBintersect2(N->bb, ray.orig, invD)) && (N->left != nullptr)) {
			bvhTraverse(N->left, ray, imin, tmin, umin, vmin, invD);
			bvhTraverse(N->right, ray, imin, tmin, umin, vmin, invD);
		}
		if ((N->left == nullptr) && (N->right == nullptr)) {
			//go through primitives
			for (int i = N->start; i < N->end; ++i) {
				float t, u, v;
				uint32_t j = m_bvh.getIndex(i);
				if ((m_triangles)[j].intersect(ray, t, u, v))
				{
					if (t > 0.1f && t < tmin)
					{
						imin = j;
						tmin = t;
						umin = u;
						vmin = v;
					}
				}

			}
		}
	}

	void Renderer::printTris()
	{
		for (auto& tri : m_triangles) {
			for (auto& pos : tri.m_vertices) {
				printVec3(pos);
			}
			std::cout << "-------------\n";
		}
	}

	glm::vec3 helmirt::Renderer::headlightShading(const RayhitResult& rt)
	{
		glm::vec3 n = rt.tri->m_normal;
		//make methods to get these properties!!!!
		glm::vec3 diffuse = rt.tri->m_material->diffuse;
		
		//get uv coords
		float u = rt.u;
		float v = rt.v;
		glm::vec2 uv = (1 - u - v) * rt.tri->m_txCoordinates[0] + u * rt.tri->m_txCoordinates[1] + v * rt.tri->m_txCoordinates[2];
		
		
		//currently using nearest pixel sampling
		if (rt.tri->hasTextureType(normalTexture)) {
			glm::ivec2 txCoords = getTexelCoords(uv, rt.tri->m_material->normal_map->getSize());
			glm::vec3 sNormal = rt.tri->m_material->normal_map->getPixel(txCoords[0], txCoords[1]);
			sNormal = glm::normalize(2.0f * sNormal - 1.0f);
			sNormal = rt.tri->getTBN() * sNormal;
			n = sNormal;
		}

		if (rt.tri->hasTextureType(diffuseTexture)) {
			glm::ivec2 txCoords = getTexelCoords(uv, rt.tri->m_material->diffuse_map->getSize());
			diffuse = rt.tri->m_material->diffuse_map->getPixel(txCoords[0], txCoords[1]);
		}

		float d = std::abs(glm::dot(n, glm::normalize(rt.point - rt.ray.orig)));
		return d * diffuse;
	}

	glm::vec3 Renderer::normalShading(const RayhitResult& rt)
	{
		return rt.tri->m_normal;
	}

}