#include "RTImage.h"

namespace helmirt {

helmirt::RTImage::RTImage(int height, int width) : m_height(height), m_width(width)
{
	m_data.resize(m_width * m_height, glm::vec3(0.0f));
	//createTexture();
}

void helmirt::RTImage::setColor(int i, int j, const glm::vec3& color)
{
	m_data.at(j * m_width + i) = color;
}

void helmirt::RTImage::toPPMFile(const std::string& filepath)
{
	std::ofstream imagefile(filepath);
	imagefile << "P3\n" << m_width << ' ' << m_height << "\n255\n";
	for (int j = m_height - 1; j >= 0; --j) {
		for (int i = 0; i < m_width; ++i) {
			vec3toStream(imagefile, m_data[j * m_width + i]);
		}
	}
}

int helmirt::RTImage::getHeight() const
{
	return m_height;
}

int helmirt::RTImage::getWidth() const
{
	return m_width;
}

void RTImage::updateTexture()
{
	glBindTexture(GL_TEXTURE_2D, m_textureId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, &m_data[0]);
}

void RTImage::bindTexture(int textureunit)
{
	glActiveTexture(GL_TEXTURE0 + textureunit);
	glBindTexture(GL_TEXTURE_2D, m_textureId);
}

unsigned int* RTImage::getTexture()
{
	return &m_textureId;
}

void helmirt::RTImage::vec3toStream(std::ostream& ostream, const glm::vec3& vec)
{
	ostream << static_cast<int>(255.999 * vec.x) << ' '
		<< static_cast<int>(255.999 * vec.y) << ' '
		<< static_cast<int>(255.999 * vec.z) << '\n';
}
void RTImage::createTexture()
{
	glGenTextures(1, &m_textureId);
	glBindTexture(GL_TEXTURE_2D, m_textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, &m_data[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
}