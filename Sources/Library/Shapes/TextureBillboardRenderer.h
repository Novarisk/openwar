// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef TextureBillboardRenderer_H
#define TextureBillboardRenderer_H

#include "graphicscontext.h"
#include "shaderprogram.h"
#include "BillboardTexture.h"


struct Billboard
{
	glm::vec3 position;
	float facing;
	float height;
	int shape;

	Billboard() {}
	Billboard(glm::vec3 p, float f, float h, int s) : position(p), facing(f), height(h), shape(s) {}
};


struct BillboardModel
{
	BillboardTexture* texture;
	std::vector<Billboard> staticBillboards;
	std::vector<Billboard> dynamicBillboards;

	int _billboardTreeShapes[16];
	int _billboardShapeCasualtyAsh[8];
	int _billboardShapeCasualtySam[8];
	int _billboardShapeCasualtyCav[16];
	int _billboardShapeFighterSamBlue;
	int _billboardShapeFighterSamRed;
	int _billboardShapeFighterAshBlue;
	int _billboardShapeFighterAshRed;
	int _billboardShapeFighterCavBlue;
	int _billboardShapeFighterCavRed;
	int _billboardShapeSmoke[8];
};


class TextureBillboardRenderer
{
public:
	typedef vertex4<glm::vec3, float, glm::vec2, glm::vec2> vertex;
	typedef shaderprogram4<glm::vec3, float, glm::vec2, glm::vec2> shaderprogram;

	shaderprogram* _shaderprogram;
	vertexbuffer<vertex> _vbo;

public:
	TextureBillboardRenderer(graphicscontext* gc);
	~TextureBillboardRenderer();

	void Reset();
	void AddBillboard(glm::vec3 position, float height, affine2 texcoords);
	void Draw(texture* tex, const glm::mat4x4& transform, const glm::vec3& cameraUp, float cameraFacingDegrees, float viewportHeight, bounds1f sizeLimit = bounds1f(0, 1024));

	void Render(BillboardModel* billboardModel, const glm::mat4x4& transform, const glm::vec3& cameraUp, float viewportHeight, float cameraFacingDegrees, bool flip);
};



#endif
