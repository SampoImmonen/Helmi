#pragma once

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include <glad/glad.h> 
#include <glm/glm.hpp>





class Image {
public:

	const int height;
	const int width;
	unsigned int textureID;
	
	ScreenVAO VAO;

	std::vector<glm::vec3> data;

	Image(int height, int width) : height(height), width(width) {
		//data.reserve(width * height);
		data.resize(width * height, glm::vec3(0.4f));

		// if you need this somewhere else make other arrangements
		VAO = ScreenVAO();
	}

	void setColor(int i, int j, glm::vec3 color) {
		//std::cout << i << " " << j << "\n";
		data.at(j * width + i) = color;
	}

	void vec3tostream(std::ostream& out, const glm::vec3& vec) {
		out << static_cast<int>(255.999 * vec.x) << ' '
			<< static_cast<int>(255.999 * vec.y) << ' '
			<< static_cast<int>(255.999 * vec.z) << '\n';
	}

	void printppm() {
		//used in console
		std::cout << "P3\n" << width << ' ' << height << "\n255\n";
		for (int j = height - 1; j >= 0; --j) {
			for (int i = 0; i < width; ++i) {
				vec3tostream(std::cout, data[j * width + i]);
			}
		}
	}

	void tofile(const std::string& filepath = "image.ppm") {
		//save image to ppm file 
		//openfile stream push stuff in it
		std::ofstream image_file;
		image_file.open(filepath);
		image_file << "P3\n" << width << ' ' << height << "\n255\n";
		for (int j = height - 1; j >= 0; --j) {
			for (int i = 0; i < width; ++i) {
				vec3tostream(image_file, data[j * width + i]);
			}
		}
		image_file.close();

	}

	void fill_image(const glm::vec3& color) {
		std::fill(data.begin(), data.end(), color);
	}

	void createTexture() {
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, &data[0]);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void updateTexture() {
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexSubImage2D(GL_TEXTURE_2D,0, 0, 0, width, height, GL_RGB, GL_FLOAT, &data[0]);
	}

	void bindTexture(int textureunit) {
		glActiveTexture(GL_TEXTURE0 + textureunit);
		glBindTexture(GL_TEXTURE_2D, textureID);
	}

	void drawImage(Shader& shader) {
		shader.UseProgram();
		glBindVertexArray(VAO.VAO);
		glDisable(GL_DEPTH_TEST);
		updateTexture();
		bindTexture(0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);
	}
	//overloading []????
};