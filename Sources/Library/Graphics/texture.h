// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef OPENWAR_USE_XCODE_FRAMEWORKS
#if TARGET_OS_IPHONE
#include <OpenGLES/ES2/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#else
#if OPENWAR_USE_GLEW
#include <GL/glew.h>
#endif
#ifdef OPENWAR_USE_GLES2
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "../resource.h"

class image;


struct texture
{
	GLuint id;

	texture();
	explicit texture(const resource& r);
	explicit texture(const image& image);
	~texture();

	void init();

	void load(const resource& r);
	void load(const image& image);

private:
	texture(const texture&) {}
	texture& operator=(const texture&) { return *this; }
};


#endif
