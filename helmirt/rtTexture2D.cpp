#include "rtTexture2D.h"
#include <iostream>

rtTexture2D::rtTexture2D(unsigned char* data, int width, int height, int nChannels)
{
	std::vector<unsigned char> v(data, data + width * height * nChannels);
	m_data = v;
	m_width = width;
	m_height = height;
	m_channels = nChannels;
}

glm::vec3 rtTexture2D::sample(float u, float v)
{
	return glm::vec3();
}

glm::ivec2 rtTexture2D::getSize() const
{
	return glm::ivec2(m_height, m_width);
}

glm::vec3 rtTexture2D::getPixel(int i, int j)
{
	unsigned char r = m_data[(j + i * m_width) * m_channels + 0];
	unsigned char g = m_data[(j + i * m_width) * m_channels + 1];
	unsigned char b = m_data[(j + i * m_width) * m_channels + 2];

	return glm::vec3(r, g, b) / 255.0f;
}

void rtTexture2D::initData()
{
}
