#ifndef STRINGSHAPE_H
#define STRINGSHAPE_H

#include <map>
#include <glm/gtc/matrix_transform.hpp>

#ifndef OPENWAR_USE_SDL
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif
#endif

#include "../Algebra/bounds.h"
#include "shaderprogram.h"
#include "vertex.h"
#include "vertexshape.h"
#include "texture.h"


struct stringfont
{
	typedef vertex3<glm::vec2, glm::vec2, float> vertex_type;

	struct item
	{
		wchar_t _character;
		glm::vec2 _bounds_origin;
		glm::vec2 _bounds_size;
		float _u0, _u1;
		float _v0, _v1;
	};

	static image* _image;

#ifndef OPENWAR_USE_SDL
#if TARGET_OS_IPHONE
	UIFont* _font;
#else
	NSFont* _font;
#endif
#endif

	shaderprogram<vertex_type>* _renderer;
	float _pixelDensity;
	texture _texture;

	std::map<wchar_t, item> _items;
	glm::vec2 _next;
	bool _dirty;

public:
	stringfont(const char* name, float size, float pixelDensity);
	stringfont(bool bold, float size, float pixelDensity);
	~stringfont();

private:
	void initialize();

public:
	float font_size() const;
	float shadow_offset() const;

	void add_character(wchar_t character);
	item get_character(wchar_t character) const;

	void update_texture();

	glm::vec2 measure(const char* string);
	glm::vec2 get_size(const item& item) const;
};


class stringglyph
{
	glyphversion _version;
	std::string _string;
	glm::mat4x4 _transform;
	float _alpha;
	float _delta;

public:
	typedef vertex3<glm::vec2, glm::vec2, float> vertex_type;

	stringglyph();
	stringglyph(const char* string, glm::vec2 translate, float alpha = 1, float delta = 0);
	stringglyph(const char* string, glm::mat4x4 transform, float alpha = 1, float delta = 0);

	const char* get_string() const { return _string.c_str(); }
	void set_string(const char* value) { _string = value; _version.increase(); }

	const glm::mat4x4 get_transform() const { return _transform; }
	void set_transform(glm::mat4x4 value) { _transform = value; _version.increase(); }
	void set_translate(glm::vec2 value) { _transform = glm::translate(glm::mat4(), glm::vec3(value, 0)); _version.increase(); }

	const float get_alpha() const { return _alpha; }
	void set_alpha(float value) { _alpha = value; _version.increase(); }

	const float get_delta() const { return _delta; }
	void set_delta(float value) { _delta = value; _version.increase(); }

	vertexglyph3<glm::vec2, glm::vec2, float> glyph(stringfont* font);

	void generate(stringfont* font, std::vector<vertex_type>& vertices);
};



class stringshape : public vertexshape3<glm::vec2, glm::vec2, float>
{
	std::vector<stringglyph*> _stringglyphs;

public:
	stringfont* _font;

	stringshape(stringfont* font);

	void clear();
	void add(const char* string, glm::mat4x4 transform, float alpha = 1, float delta = 0);

	void update(GLenum usage);
};


#endif
