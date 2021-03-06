// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef TextureRenderer_H
#define TextureRenderer_H

#include "graphicscontext.h"
#include "shaderprogram.h"


class TextureShape3
{
protected:
	typedef vertex2<glm::vec3, glm::vec2> vertex;

	shaderprogram<vertex>* _shaderprogram;
	vertexbuffer<vertex> _vbo;

public:
	TextureShape3(graphicscontext* gc);
	virtual ~TextureShape3();

	virtual void Reset() = 0;
	void Draw(const glm::mat4x4& transform, const texture* texture);
};


class TextureTriangleShape3 : public TextureShape3
{
public:
	TextureTriangleShape3(graphicscontext* gc) : TextureShape3(gc) { }
	virtual ~TextureTriangleShape3();
	virtual void Reset();

	void AddVertex(glm::vec3 p, glm::vec2 t);
};


#endif
