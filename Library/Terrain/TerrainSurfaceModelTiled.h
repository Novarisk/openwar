// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef TERRAINSURFACEMODELTILED_H
#define TERRAINSURFACEMODELTILED_H

#include "TerrainSurfaceModel.h"
#include "bounds.h"
#include "heightmap.h"
#include "texture.h"
#include "image.h"


class TerrainSurfaceModelTiled : public TerrainSurfaceModel
{
public:
	struct Tile
	{
		texture* texture;
		int rotate; // counterclockwise
		bool mirror;

		Tile() : texture(nullptr), rotate(0), mirror(false) { }
	};

private:
	bounds2f _bounds;
	glm::ivec2 _size;
	Tile* _tiles;
	heightmap* _heightmap;

	std::map<int, texture*> _textures;
	std::map<std::string, int> _textureNumber;
	int _nextTextureNumber;

public:
	TerrainSurfaceModelTiled(bounds2f bounds, glm::ivec2 size);
	~TerrainSurfaceModelTiled();

	virtual float GetHeight(glm::vec2 position) const;
	virtual glm::vec3 GetNormal(glm::vec2 position) const;
	virtual const float* Intersect(ray r);

	virtual bool IsWater(glm::vec2 position) const;
	virtual bool IsForest(glm::vec2 position) const;
	virtual bool IsImpassable(glm::vec2 position) const;

	virtual bool ContainsWater(bounds2f bounds) const;

	void SetHeight(int x, int y, float h);
	void SetTile(int x, int y, const std::string& texture, int rotate, bool mirror);

	bounds2f GetBounds() const { return _bounds; }
	glm::ivec2 GetSize() const { return _size; }

	Tile* GetTile(int x, int y);
};


#endif