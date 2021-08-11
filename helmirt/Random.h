#pragma once


/*
File contains all utilities for generating random numbers and vectors
*/

#include <chrono>
#include <random>
#include "glm/glm.hpp"

class Random
{
public:
	Random();
	Random(float seed);
	uint32_t getU32() { return m_distribution(m_engine); };
	uint32_t getU32(int min, int max) { (max <= min) ? min : (getU32() % (max - min) + min); };
	float getFloat() { return (float)getU32() * (1.0f / 4294967296.0f); };
	float getFloat(float min, float max) { return getFloat() * (max - min) + min; }
	glm::vec2 getVec2() { return glm::vec2(getFloat(), getFloat()); }
	glm::vec2 getVec2(float min, float max) { return glm::vec2(getFloat(min, max), getFloat(min, max)); };
	glm::vec3 getVec3() { return glm::vec3(getFloat(), getFloat(), getFloat()); }
	glm::vec3 getVec3(float min, float max) { return glm::vec3(getFloat(min, max), getFloat(min, max), getFloat(min, max)); };

private:
	//not not change order of variables
	std::random_device m_r;
	std::mt19937 m_engine;
	std::uniform_int_distribution<uint32_t> m_distribution;
};

