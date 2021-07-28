#include "Buffers.h"

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, bool isHdr): m_width(width), m_height(height), m_isHdr(isHdr)
{

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_color);
	glBindTexture(GL_TEXTURE_2D, m_color);
	if (m_isHdr) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	else{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color, 0);

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	if (!checkCompleteness()) {
		std::cout << "FrameBuffer is not complete";
	};

	unbind();
}

FrameBuffer::~FrameBuffer()
{
	//glDeleteFramebuffers(GL_FRAMEBUFFER, &m_fbo);
}

void FrameBuffer::bind()
{
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bindColorTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_color);
}

unsigned int* FrameBuffer::getTexture()
{
	return &m_color;
}

void FrameBuffer::Resize(unsigned int width, unsigned int height)
{
	m_width = width;
	m_height = height;
	bind();
	glBindTexture(GL_TEXTURE_2D, m_color);
	if (m_isHdr) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	else{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	unbind();
}

bool FrameBuffer::checkCompleteness()
{
	bind();
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

ShadowMapBuffer::ShadowMapBuffer(unsigned int height, unsigned int width) : m_width(width), m_height(height)
{

	glGenFramebuffers(1, &m_framebufferId);

	glGenTextures(1, &m_depthmapId);
	glBindTexture(GL_TEXTURE_2D, m_depthmapId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthmapId, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (!checkCompleteness()) {
		std::cout << "not complete";
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void ShadowMapBuffer::bind()
{
	//change viewport to suit shadowmap resolution
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMapBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapBuffer::bindDepthTexture(int textureunit)
{
	glActiveTexture(GL_TEXTURE0+textureunit);
	glBindTexture(GL_TEXTURE_2D, m_depthmapId);
}

unsigned int* ShadowMapBuffer::getTexture()
{
	return &m_depthmapId;
}

bool ShadowMapBuffer::checkCompleteness()
{
	bind();
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

HDRFrameBuffer::HDRFrameBuffer(unsigned int height, unsigned int width): m_height(height), m_width(width)
{
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glGenTextures(2, m_colorBuffers);
	for (unsigned int i = 0; i<2; ++i){
		glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, m_colorBuffers[i], 0);
	}

	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

	glDrawBuffers(2, m_attachments);
	if (!checkCompleteness()) {
		std::cout << "FrameBuffer is not complete";
	};

	unbind();
}

void HDRFrameBuffer::bind()
{
	glViewport(0, 0, m_width, m_height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void HDRFrameBuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDRFrameBuffer::resize(unsigned int height, unsigned int width)
{
	m_width = width;
	m_height = height;
	bind();
	for (unsigned int i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	unbind();
}

//currently binds into slots 1, 2;
void HDRFrameBuffer::bindTextures()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffers[0]);
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffers[1]);
}

unsigned int* HDRFrameBuffer::getTexture(int i)
{
	if (i > 1) return &m_colorBuffers[0];
	return &m_colorBuffers[i];
}

unsigned int HDRFrameBuffer::getBloomTexture()
{
	return m_colorBuffers[1];
}

bool HDRFrameBuffer::checkCompleteness()
{
	return false;
}

PingPongFrameBuffer::PingPongFrameBuffer(unsigned int height, unsigned int width): m_height(height), m_width(width)
{
	glGenFramebuffers(2, m_fbos);
	glGenTextures(2, m_colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);
		glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete! " << i <<  " \n";
	}
}

void PingPongFrameBuffer::resize(unsigned int height, unsigned int width)
{
	m_width = width;
	m_height = height;
	for (unsigned int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[i]);
		glBindTexture(GL_TEXTURE_2D, m_colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffers[i], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PingPongFrameBuffer::bindTexture(int texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffers[texture]);
}

void PingPongFrameBuffer::bindTexture(int texture, int textureslot)
{
	glActiveTexture(GL_TEXTURE0 + textureslot);
	glBindTexture(GL_TEXTURE_2D, m_colorBuffers[texture]);
}

void PingPongFrameBuffer::bindFrameBuffer(int framebuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbos[framebuffer]);
}



bool PingPongFrameBuffer::checkCompleteness()
{
	return false;
}
