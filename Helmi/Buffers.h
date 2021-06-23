#pragma once

#include <iostream>
#include "glad/glad.h"

class FrameBuffer {
	
public:
	FrameBuffer() = default;
	FrameBuffer(int width, int height);
	~FrameBuffer();

	void bind();
	void unbind();
	void bindColorTexture();
	unsigned int* getTexture();
	void Resize(int width, int height);

	unsigned int m_fbo, m_color, m_rbo;
private:

	bool checkCompleteness();
};

class VertexBuffer {

};

class IndexBuffer {

};

