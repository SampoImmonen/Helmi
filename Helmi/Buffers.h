#pragma once

#include <iostream>
#include "glad/glad.h"

class FrameBuffer {
	
public:
	FrameBuffer() = default;
	FrameBuffer(unsigned int width, unsigned int height);
	~FrameBuffer();

	void bind();
	void unbind();
	void bindColorTexture();
	unsigned int* getTexture();
	void Resize(unsigned int width, unsigned int height);

	unsigned int m_fbo, m_color, m_rbo;
private:
	unsigned int m_width, m_height;
	bool checkCompleteness();
};


class ShadowMapBuffer {
public:
	ShadowMapBuffer() = default;
	ShadowMapBuffer(unsigned int height, unsigned int width);
	void bind();

	void unbind();
	void bindDepthTexture(int textureunit);
	unsigned int* getTexture();

private:
	unsigned int m_width, m_height;
	unsigned int m_framebufferId, m_depthmapId;
	bool checkCompleteness();
};


class VertexBuffer {

};

class IndexBuffer {

};

