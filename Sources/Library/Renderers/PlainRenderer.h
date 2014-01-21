// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef LineRenderer_H
#define LineRenderer_H

#include "../Graphics/renderer.h"


class PlainRenderer
{
protected:
	typedef vertex1<glm::vec3> vertex;

	struct uniforms
	{
		glm::mat4x4 _transform;
		GLfloat _point_size;
		glm::vec4 _color;

		uniforms() : _point_size(1) { }
	};

	vertexbuffer<vertex> _vbo;
	renderer1<glm::vec3>* _renderer;

public:
	PlainRenderer();
	virtual ~PlainRenderer();

	virtual void Reset() = 0;
	void Draw(const glm::mat4x4& transform, const glm::vec4& color);
};


class PlainLineRenderer : public PlainRenderer
{
public:
	virtual void Reset();
	void AddLine(const glm::vec3& p1, const glm::vec3& p2);
};


class PlainTriangleRenderer : public PlainRenderer
{
public:
	virtual void Reset();
	void AddVertex(const glm::vec3& p);
};


#endif
