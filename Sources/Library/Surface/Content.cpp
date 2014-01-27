// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#include <glm/gtc/matrix_transform.hpp>
#include "Content.h"
#include "Surface.h"



glm::mat4 ViewportTransform(bounds2f viewport, glm::vec2 translate, float rotate)
{
	glm::vec2 viewport_center = viewport.center();
	glm::vec2 viewport_scale = 2.0f / viewport.size();

	glm::mat4x4 result = glm::scale(glm::mat4x4(), glm::vec3(viewport_scale.x, viewport_scale.y, 1.0f))
		* glm::translate(glm::mat4x4(), glm::vec3(translate.x - viewport_center.x, translate.y - viewport_center.y, 0.0f));

	if (rotate != 0)
		result = result * glm::rotate(glm::mat4x4(), rotate * 180 / (float)M_PI, glm::vec3(0, 0, 1));

	return result;
}



Content::Content() :
_surface(nullptr),
_container(nullptr),
_visible(true),
_frame(),
_anchor(),
_rotate(0),
_scale(1, 1),
_translate(),
_flip(false)
{
}


Content::~Content()
{
	SetContainer(nullptr, nullptr);
}


Surface* Content::GetSurface() const
{
	return _surface;
}


Container* Content::GetContainer() const
{
	return _container;
}


void Content::SetContainer(Container* value, Content* behindContent)
{
	SetContentContainer(this, value, behindContent);
}


bool Content::IsVisible() const
{
	return _visible;
}


void Content::SetVisible(bool value)
{
	_visible = value;
}


bounds2f Content::GetViewport() const
{
	return _viewport;
}


void Content::SetViewport(bounds2f value)
{
	_viewport = value;
}


void Content::UseViewport()
{
	bounds2f viewport;

	for (Content* c = this; c != nullptr && viewport.is_empty(); c = c->GetContainer())
		viewport = c->GetViewport();

	viewport = viewport * GetSurface()->GetGraphicsContext()->get_pixeldensity();
	glViewport((GLint)viewport.min.x, (GLint)viewport.min.y, (GLsizei)viewport.size().x, (GLsizei)viewport.size().y);
}



bounds2f Content::GetFrame() const
{
	return _frame;
}


void Content::SetFrame(bounds2f value)
{
	SetFrameValue(value);
}


void Content::OnFrameChanged()
{
}


glm::vec2 Content::GetAnchor() const
{
	return _anchor;
}


void Content::SetAnchor(glm::vec2 value)
{
	_anchor = value;
}


glm::vec2 Content::GetPosition() const
{
	return glm::mix(_frame.min, _frame.max, _anchor);
}


void Content::SetPosition(glm::vec2 value)
{
	glm::vec2 delta = value - GetPosition();
	SetFrameValue(_frame + delta);
}


glm::vec2 Content::GetSize() const
{
	return _frame.size();
}


void Content::SetSize(glm::vec2 value)
{
	glm::vec2 p = GetPosition();
	SetFrameValue(bounds2f(p - value * _anchor,  p + value * (glm::vec2(1, 1) - _anchor)));
}


bounds2f Content::GetBounds() const
{
	return _bounds;
}


void Content::SetBounds(const bounds2f& value)
{
	_bounds = value;
	//OnBoundsChanged();
}


float Content::GetRotate() const
{
	return _rotate;
}


void Content::SetRotate(float value)
{
	_rotate = value;
}


glm::vec2 Content::GetScale() const
{
	return _scale;
}


void Content::SetScale(glm::vec2 value)
{
	_scale = value;
}


glm::vec2 Content::GetTranslate() const
{
	return _translate;
}


void Content::SetTranslate(glm::vec2 value)
{
	_translate = value;
}


glm::mat4 Content::GetTransform() const
{
	return _transform;
}


void Content::SetTransform(const glm::mat4& value)
{
	_transform = value;
}


glm::mat4 Content::GetViewportTransform() const
{
	bounds2f viewport;

	for (const Content* c = this; c != nullptr && viewport.is_empty(); c = c->GetContainer())
		viewport = c->GetViewport();

	glm::mat4 result;

	if (!viewport.is_empty())
	{
		glm::vec2 size = viewport.size();

		result = glm::translate(result, glm::vec3(-1, -1, 0));
		result = glm::scale(result, glm::vec3(2 / size.x, 2 / size.y, 1));
		result = glm::translate(result, glm::vec3(-viewport.min, 0));
	}

	return result;
}


glm::mat4 Content::GetContainerTransform() const
{
	glm::mat4 result;

	/*for (const Content* c = this; c != nullptr; c = c->GetContainer())
	{
		result = c->GetContentTransform() * result;
	}*/

	return result;
}


glm::mat4 Content::GetContentTransform() const
{
	bounds2f frame = GetFrame();

	glm::mat4 result;

	result = glm::translate(result, glm::vec3(frame.min, 0));

	glm::vec3 offset = glm::vec3(frame.size() * _anchor, 0);
	result = glm::translate(result, offset);
	result = glm::translate(result, glm::vec3(_translate, 0));
	result = glm::rotate(result, _rotate, glm::vec3(0, 0, 1));
	result = glm::scale(result, glm::vec3(_scale, 1));
	result = glm::translate(result, -offset);

	return result;
}


glm::vec2 Content::SurfaceToContent(glm::vec2 value) const
{
	bounds2f viewport = GetFrame();
	return 2.0f * (value - viewport.p11()) / viewport.size() - 1.0f;
}


glm::vec2 Content::ContentToSurface(glm::vec2 value) const
{
	bounds2f viewport = GetFrame();
	return viewport.p11() + (value + 1.0f) / 2.0f * viewport.size();
}


void Content::SetFrameValue(const bounds2f& value)
{
	/*glm::vec2 oldPosition = GetPosition();
	glm::vec2 oldSize = GetSize();
	float oldLeft = GetLeft();
	float oldRight = GetRight();
	float oldBottom = GetBottom();
	float oldTop = GetTop();
	float oldWidth = GetWidth();
	float oldHeight = GetHeight();*/

	_frame = value;

	OnFrameChanged();

	//if (_container != nullptr)
	//	_container->OnFrameChanged(this);

	/*if (oldPosition != GetPosition())
		position.CallObserver();
	if (oldSize != GetSize())
		size.CallObserver();
	if (oldLeft != GetLeft())
		left.CallObserver();
	if (oldRight != GetRight())
		right.CallObserver();
	if (oldBottom != GetBottom())
		bottom.CallObserver();
	if (oldTop != GetTop())
		top.CallObserver();
	if (oldWidth != GetWidth())
		width.CallObserver();
	if (oldHeight != GetHeight())
		height.CallObserver();*/
}


void Content::RenderSolid(const glm::mat4& transform, bounds2f bounds, glm::vec4 color) const
{
	vertexbuffer<plain_vertex> shape;
	shape._mode = GL_TRIANGLE_STRIP;
	shape._vertices.push_back(plain_vertex(bounds.p11()));
	shape._vertices.push_back(plain_vertex(bounds.p12()));
	shape._vertices.push_back(plain_vertex(bounds.p21()));
	shape._vertices.push_back(plain_vertex(bounds.p22()));

	renderers::singleton->_plain_renderer->get_uniform<glm::mat4>("transform").set_value(transform);
	renderers::singleton->_plain_renderer->get_uniform<glm::vec4>("color").set_value(color);
	renderers::singleton->_plain_renderer->render(shape);
}


void Content::RenderOutline(const glm::mat4& transform, bounds2f bounds, glm::vec4 color) const
{
	vertexbuffer<plain_vertex> shape;
	shape._mode = GL_LINE_LOOP;
	shape._vertices.push_back(plain_vertex(bounds.p11()));
	shape._vertices.push_back(plain_vertex(bounds.p12()));
	shape._vertices.push_back(plain_vertex(bounds.p22()));
	shape._vertices.push_back(plain_vertex(bounds.p21()));

	renderers::singleton->_plain_renderer->get_uniform<glm::mat4>("transform").set_value(transform);
	renderers::singleton->_plain_renderer->get_uniform<glm::vec4>("color").set_value(color);
	renderers::singleton->_plain_renderer->render(shape);
}
