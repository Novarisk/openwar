#include "stringshape.h"
#include "../Algebra/image.h"
#include <glm/gtc/matrix_transform.hpp>



#if !defined(ENABLE_BIDIRECTIONAL_TEXT)
#define ENABLE_BIDIRECTIONAL_TEXT 0
//#error ENABLE_BIDIRECTIONAL_TEXT not defined
#endif

#if ENABLE_BIDIRECTIONAL_TEXT


#include "unicode/ubidi.h"


static unichar* ReserveSrcBuffer(int required)
{
	static unichar* buffer = nullptr;
	static int reserved = 0;

	if (buffer == nullptr || required > reserved)
	{
		delete buffer;
		buffer = new unichar[required];
		reserved = required;
	}

	return buffer;
}


static unichar* ReserveDstBuffer(int required)
{
	static unichar* buffer = nullptr;
	static int reserved = 0;

	if (buffer == nullptr || required > reserved)
	{
		delete buffer;
		buffer = new unichar[required];
		reserved = required;
	}

	return buffer;
}



static bool MayNeedReorder(unichar c)
{
	return c > 255;
}


static bool CanSkipReorder(NSString* string)
{
	NSUInteger length = string.length;
	if (length > 16)
		return false;

	static unichar buffer[16];
	[string getCharacters:buffer];

	for (NSUInteger i = 0; i < length; ++i)
		if (MayNeedReorder(buffer[i]))
			return false;

	return true;
}



static NSString* ReorderToDisplayDirection(NSString* string)
{
	if (CanSkipReorder(string))
		return string;

	UErrorCode error = U_ZERO_ERROR;
	int length = (int)string.length;

	unichar* src = ReserveSrcBuffer(length);
	unichar* dst = ReserveDstBuffer(length * 2);

	UBiDi* ubidi = ubidi_openSized(length, 0, &error);
    if (error != 0)
        NSLog(@"%04x", error);

	[string getCharacters:src];
    ubidi_setPara(ubidi, src, length, UBIDI_DEFAULT_LTR, NULL, &error);
    if (error != 0)
        NSLog(@"%04x", error);

	length = ubidi_writeReordered(ubidi, dst, length * 2, UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &error);
    if (error != 0)
        NSLog(@"%04x", error);

    NSString* result = [NSString stringWithCharacters:dst length:(NSUInteger)length];

    ubidi_close(ubidi);

	return result;
}

#endif




image* stringfont::_image = nullptr;



stringfont::stringfont(const char* name, float size, float pixelDensity) :
#ifndef OPENWAR_USE_SDL
_font(nil),
#endif
_pixelDensity(pixelDensity),
_items(),
_next(),
_dirty(false)
{
	initialize();

	size *= _pixelDensity;

#ifndef OPENWAR_USE_SDL
#if TARGET_OS_IPHONE
	_font = [[UIFont fontWithName:[NSString stringWithUTF8String:name] size:size] retain];
#else
	_font = [[NSFont fontWithName:[NSString stringWithUTF8String:name] size:size] retain];
#endif
#endif
}



stringfont::stringfont(bool bold, float size, float pixelDensity) :
#ifndef OPENWAR_USE_SDL
_font(nil),
#endif
_pixelDensity(pixelDensity),
_items(),
_next(),
_dirty(false)
{
	initialize();

	size *= _pixelDensity;

#ifndef OPENWAR_USE_SDL
#if TARGET_OS_IPHONE
	if (bold)
		_font = [[UIFont boldSystemFontOfSize:size] retain];
	else
		_font = [[UIFont systemFontOfSize:size] retain];
#else
	if (bold)
		_font = [[NSFont boldSystemFontOfSize:size] retain];
	else
		_font = [[NSFont systemFontOfSize:size] retain];
#endif
#endif
}



stringfont::~stringfont()
{
#ifndef OPENWAR_USE_SDL
	[_font release];
#endif
}



void stringfont::initialize()
{
	if (_image == nullptr)
		_image = new image(512, 512);

	_renderer = new shaderprogram3<glm::vec2, glm::vec2, float>(
		"position", "texcoord", "alpha",
		VERTEX_SHADER
		({
			attribute vec2 position;
			attribute vec2 texcoord;
			attribute float alpha;
			uniform mat4 transform;
			varying vec2 _texcoord;
			varying float _alpha;

			void main()
			{
				vec4 p = transform * vec4(position.x, position.y, 0, 1);

				_texcoord = texcoord;
				_alpha = alpha;

				gl_Position = vec4(p.x, p.y, 0.5, 1.0);
			}
		}),
		FRAGMENT_SHADER
		({
			uniform sampler2D texture;
			uniform vec4 color;
			varying vec2 _texcoord;
			varying float _alpha;

			void main()
			{
				vec4 result;
				result.rgb = color.rgb;
				result.a = texture2D(texture, _texcoord).a * color.a * clamp(_alpha, 0.0, 1.0);

				gl_FragColor = result;
			}
		})
	);
	_renderer->_blend_sfactor = GL_SRC_ALPHA;
	_renderer->_blend_dfactor = GL_ONE_MINUS_SRC_ALPHA;
}



float stringfont::font_size() const
{
#ifdef OPENWAR_USE_SDL

	return 14;

#else

	return (float)_font.pointSize / _pixelDensity;

#endif
}



float stringfont::shadow_offset() const
{
	return 1 / _pixelDensity;
}




void stringfont::add_character(wchar_t character)
{
	if (_items.find(character) != _items.end())
		return;

#ifndef OPENWAR_USE_SDL

	unichar uc = (unichar)character;

	NSString* text = [NSString stringWithCharacters:&uc length:1];

#if TARGET_OS_IPHONE
	CGSize size = [text sizeWithFont:_font];
#else
	NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:_font, NSFontAttributeName, nil];
	CGSize size = [text sizeWithAttributes:attributes];
#endif

	if (_next.x + size.width > _image->width())
	{
		_next.x = 0;
		_next.y += size.height + 1;
		if (_next.y + size.height > _image->height())
			_next.y = 0; // TODO: handle texture resizing
	}

	item item;
	item._character = character;
	item._bounds_origin = _next;
	item._bounds_size = glm::vec2(size.width, size.height);
	item._u0 = (float)item._bounds_origin.x / _image->width();
	item._u1 = (float)(item._bounds_origin.x + item._bounds_size.x) / _image->width();
	item._v0 = 1 - (float)(item._bounds_origin.y + item._bounds_size.y) / _image->height();
	item._v1 = 1 - (float)item._bounds_origin.y / _image->height();

	_items[character] = item;

	_next.x += floorf((float)item._bounds_size.x + 1) + 1;
	_dirty = true;

#endif
}



stringfont::item stringfont::get_character(wchar_t character) const
{
	auto i = _items.find(character);
	if (i != _items.end())
		return i->second;

	return _items.find('.')->second;
}



void stringfont::update_texture()
{
	if (!_dirty)
		return;

#ifdef OPENWAR_USE_SDL

#else

#if TARGET_OS_IPHONE
	UIGraphicsPushContext(_image->CGContext());
#else
	NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithGraphicsPort:_image->CGContext() flipped:YES];
	[NSGraphicsContext saveGraphicsState];
	[NSGraphicsContext setCurrentContext:gc];
#endif

	CGContextClearRect(_image->CGContext(), CGRectMake(0, 0, _image->width(), _image->height()));

	for (std::map<wchar_t, item>::iterator i = _items.begin(); i != _items.end(); ++i)
	{
		item item = (*i).second;

		unichar uc = (unichar)item._character;
		NSString *text = [NSString stringWithCharacters:&uc length:1];

		CGContextSetRGBFillColor(_image->CGContext(), 1, 1, 1, 1);

#if TARGET_OS_IPHONE
	    [text drawAtPoint:CGPointMake(item._bounds_origin.x, item._bounds_origin.y) withFont:_font];
#else
		NSDictionary* attributes = [NSDictionary dictionaryWithObjectsAndKeys:_font, NSFontAttributeName, nil];
		[text drawAtPoint:CGPointMake(item._bounds_origin.x, item._bounds_origin.y) withAttributes:attributes];
#endif
	}

	_texture.load(*_image);

#if TARGET_OS_IPHONE
	UIGraphicsPopContext();
#else
	[NSGraphicsContext restoreGraphicsState];
#endif

#endif

	_dirty = false;
}



glm::vec2 stringfont::measure(const char* s)
{
	float w = 0;
	float h = 0;

#ifndef OPENWAR_USE_SDL

	NSString* string = [NSString stringWithUTF8String:s];

	//return vector2([string sizeWithFont:_font]) * size / _font.pointSize;

	for (NSUInteger i = 0; i < string.length; ++i)
	{
		wchar_t character = [string characterAtIndex:i];
		add_character(character);
	}

	for (NSUInteger i = 0; i < string.length; ++i)
	{
		wchar_t character = [string characterAtIndex:i];
		item item = get_character(character);
		glm::vec2 s = get_size(item);
		w += s.x;
		h = fmaxf(h, s.y);
	}

#endif

	return glm::vec2(w, h);
}



glm::vec2 stringfont::get_size(const item& item) const
{
	return glm::vec2(item._bounds_size.x, item._bounds_size.y) / _pixelDensity;

	/*float h = size;
	float w = item._bounds_size.x * size / item._bounds_size.y;
	return vector2(w, h) / pixel_scale();*/
}


/***/



stringglyph::stringglyph() :
_string(),
_transform(),
_alpha(1),
_delta(0)
{
}


stringglyph::stringglyph(const char* string, glm::vec2 translate, float alpha, float delta) :
_string(string),
_transform(glm::translate(glm::mat4(), glm::vec3(translate, 0))),
_alpha(alpha),
_delta(delta)
{
}


stringglyph::stringglyph(const char* string, glm::mat4x4 transform, float alpha, float delta) :
_string(string),
_transform(transform),
_alpha(alpha),
_delta(delta)
{
}


vertexglyph3<glm::vec2, glm::vec2, float> stringglyph::glyph(stringfont* font)
{
	return vertexglyph3<glm::vec2, glm::vec2, float>([this, font](std::vector<vertex3<glm::vec2, glm::vec2, float>>& vertices) {
		generate(font, vertices);
	});
}


void stringglyph::generate(stringfont* font, std::vector<stringglyph::vertex_type>& vertices)
{
#ifndef OPENWAR_USE_SDL

	NSString* string = [NSString stringWithUTF8String:_string.c_str()];

#if ENABLE_BIDIRECTIONAL_TEXT
    string = ReorderToDisplayDirection(string);
#endif

	for (NSUInteger i = 0; i < string.length; ++i)
	{
		wchar_t character = [string characterAtIndex:i];
		font->add_character(character);
	}

	glm::vec2 p(0, 0);
	float alpha = _alpha;

	for (NSUInteger i = 0; i < string.length; ++i)
	{
		wchar_t character = [string characterAtIndex:i];
		stringfont::item item = font->get_character(character);

		glm::vec2 s = font->get_size(item);
		bounds2f bounds = bounds2_from_corner(p, s);
		bounds.min = (_transform * glm::vec4(bounds.min.x, bounds.min.y, 0, 1)).xy();
		bounds.max = (_transform * glm::vec4(bounds.max.x, bounds.max.y, 0, 1)).xy();

		float next_alpha = alpha + _delta * s.x;

		vertices.push_back(stringglyph::vertex_type(bounds.p11(), glm::vec2(item._u0, item._v0), alpha));
		vertices.push_back(stringglyph::vertex_type(bounds.p12(), glm::vec2(item._u0, item._v1), alpha));
		vertices.push_back(stringglyph::vertex_type(bounds.p22(), glm::vec2(item._u1, item._v1), next_alpha));

		vertices.push_back(stringglyph::vertex_type(bounds.p22(), glm::vec2(item._u1, item._v1), next_alpha));
		vertices.push_back(stringglyph::vertex_type(bounds.p21(), glm::vec2(item._u1, item._v0), next_alpha));
		vertices.push_back(stringglyph::vertex_type(bounds.p11(), glm::vec2(item._u0, item._v0), alpha));

		if (next_alpha < 0)
			break;

		p.x += s.x;
		alpha = next_alpha;
	}

#endif
}


/***/



stringshape::stringshape(stringfont* font) : _font(font)
{
	_vbo._mode = GL_TRIANGLES;
}



void stringshape::clear()
{
	for (stringglyph* g : _stringglyphs)
		delete g;
	_stringglyphs.clear();

	glyphs.clear();
}




void stringshape::add(const char* s, glm::mat4x4 transform, float alpha, float delta)
{
	stringglyph* g = new stringglyph(s, transform, alpha, delta);
	_stringglyphs.push_back(g);

	glyphs.push_back(vertexglyph3<glm::vec2, glm::vec2, float>([this, g](std::vector<vertex3<glm::vec2, glm::vec2, float>>& vertices) {
		g->generate(_font, vertices);
	}));
}


void stringshape::update(GLenum usage)
{
	_font->update_texture();
	update_vbo();
}
