#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glad/glad.h>

namespace helmirt {

	class RTImage
	{
	public:
		RTImage() {}
		RTImage(int height, int width);
		void setColor(int i, int j, const glm::vec3& color);
		void toPPMFile(const std::string& filepath = "image.ppm");
		int getHeight() const;
		int getWidth() const;
		void updateTexture();
		void bindTexture(int textureunit=0);
		unsigned int* getTexture();
		void createTexture();
		void resize(int height, int width);

	private:
		int m_height, m_width;
		std::vector<glm::vec3> m_data;
		unsigned int m_textureId;
		
		void vec3toStream(std::ostream& ostream, const glm::vec3& vec);
	};
}

