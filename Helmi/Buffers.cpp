#include "Buffers.h"

FrameBuffer::FrameBuffer(int width, int height)
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

void FrameBuffer::Resize(int width, int height)
{
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
