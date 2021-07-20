#pragma once

#include <vector>
#include <string>

#include "glm/glm.hpp"

class rtTexture2D
{
public:
	rtTexture2D() {};
	rtTexture2D(unsigned char* data, int width, int height, int nChannels);
	glm::vec3 sample(float u, float v);
	glm::ivec2 getSize() const;
	glm::vec3 getPixel(int i, int j);

private:

	void initData();

	std::vector<unsigned char> m_data;
	int m_width, m_height, m_channels;
};

