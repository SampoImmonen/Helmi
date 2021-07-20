#include "Buffers.h"

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height): m_width(width), m_height(height)
{

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_color);
	glBindTexture(GL_TEXTURE_2D, m_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

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
