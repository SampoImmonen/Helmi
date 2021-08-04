#pragma once

#include <iostream>
#include "glad/glad.h"


class FrameBuffer {
	
public:
	FrameBuffer() = default;
	FrameBuffer(unsigned int width, unsigned int height, bool isHdr = false);
	~FrameBuffer();

	void bind();
	void unbind();
	void bindColorTexture();
	unsigned int* getTexture();
	void Resize(unsigned int height, unsigned int width);

	unsigned int m_fbo, m_color, m_rbo;
private:
	unsigned int m_width, m_height;
	bool m_isHdr = false;
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


class HDRFrameBuffer {
//used for HDR Bloom
public:
	HDRFrameBuffer() = default;
	HDRFrameBuffer(unsigned int height, unsigned int width);
	~HDRFrameBuffer() {}
	void bind();
	void unbind();
	void resize(unsigned int height, unsigned int width);

	void bindTextures();
	unsigned int* getTexture(int i);
	unsigned int getBloomTexture();
private:
	unsigned int m_fbo, m_rbo;
	unsigned int m_colorBuffers[2];
	unsigned int m_height, m_width;
	unsigned int m_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	bool checkCompleteness();

};


class PingPongFrameBuffer {
public:
	PingPongFrameBuffer() = default;
	PingPongFrameBuffer(unsigned int height, unsigned int width);
	void resize(unsigned int height, unsigned int width);
	void bindTexture(int texture);
	void bindTexture(int texture, int textureslot);
	void bindFrameBuffer(int framebuffer);
private:
	unsigned int m_fbos[2];
	unsigned int m_colorBuffers[2];
	unsigned int m_height, m_width;
	bool checkCompleteness();
};



class VertexBuffer {

};

class IndexBuffer {

};

