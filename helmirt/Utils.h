#pragma once

#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include "Bvh.h"


namespace helmirt {

inline std::ofstream& operator<<(std::ofstream& os, const glm::vec3& v) {
	os << v.x << " " << v.y << "  " << v.z << " ";
	return os;
}

inline std::ifstream& operator>>(std::ifstream& is, glm::vec3& v) {
	is >> v.x >> v.y >> v.z;
	return is;
}


template <typename T>
void printline(T x) {
	std::cout << x << "\n";
}

inline glm::vec3 listToVec3(const float* l) {
	return glm::vec3(l[0], l[1], l[2]);
}

inline void printVec3(const glm::vec3& vec) {
	std::cout << vec[0] << " " << vec[1] << " " << vec[2] << "\n";
}

}
