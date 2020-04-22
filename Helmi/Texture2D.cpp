#include "Texture2D.h"

Texture2D::Texture2D(const std::string& path)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	if (data!=nullptr)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "\nFailed to load texture:" << path << "\n";
	}
	stbi_image_free(data);
}

Texture2D::Texture2D()
{
}

Texture2D::~Texture2D()
{
}

void Texture2D::bind(int textureunit)
{
	glActiveTexture(GL_TEXTURE0 + textureunit);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture2D::setTexParameteri(GLenum target, GLenum pname, GLint param)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(target, pname, param);
}
