// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#include <glm/gtc/constants.hpp>

#include "../../BattleModel/BattleSimulator.h"
#include "PlainShape3.h"
#include "../../Library/Shapes/TextureBillboardRenderer.h"
#include "TextureShape3.h"
#include "BattleView.h"
#include "UnitCounter.h"
#include "BattleCommander.h"


UnitCounter::UnitCounter(BattleView* battleView, Unit* unit) :
_battleView(battleView),
_unit(unit),
_routingTimer(0),
_weaponIndex(0)
{
	_samuraiWeapon = SamuraiModule::GetSamuraiWeapon(unit->unitClass.c_str());
	_samuraiPlatform = SamuraiModule::GetSamuraiPlatform(unit->unitClass.c_str());
}


UnitCounter::~UnitCounter()
{
}


bool UnitCounter::Animate(float seconds)
{
	float routingBlinkTime = _unit->state.GetRoutingBlinkTime();

	if (!_unit->state.IsRouting() && routingBlinkTime != 0)
	{
		_routingTimer -= seconds;
		if (_routingTimer < 0)
			_routingTimer = routingBlinkTime;
	}

	return true;
}


void UnitCounter::AppendUnitMarker(TextureBillboardRenderer* renderer, bool flip)
{
	bool routingIndicator = false;
	float routingBlinkTime = _unit->state.GetRoutingBlinkTime();

	if (_unit->state.IsRouting())
	{
		routingIndicator = true;
	}
	else if (routingBlinkTime != 0 && 0 <= _routingTimer && _routingTimer < 0.2f)
	{
		routingIndicator = true;
	}

	int color = 3;
	if (!routingIndicator)
	{
		if (_unit->commander->GetTeam() != _battleView->GetCommander()->GetTeam())
			color = 0;
		else if (_unit->commander != _battleView->GetCommander())
			color = 1;
		else
			color = 2;
	}

	int command = _unit->commander == _battleView->GetCommander() && _unit->commander->IsActive() ? 0 : 1;

	glm::vec3 position = _battleView->GetSimulator()->GetHeightMap()->GetPosition(_unit->state.center, 0);
	glm::vec2 texsize(0.1875, 0.1875); // 48 / 256
	glm::vec2 texcoord1 = texsize * glm::vec2(color, command);
	glm::vec2 texcoord2 = texsize * glm::vec2((int)_samuraiPlatform, 3);
	glm::vec2 texcoord3 = texsize * glm::vec2(4, (int)_samuraiWeapon);

	if (flip)
	{
		texcoord1 += texsize;
		texcoord2 += texsize;
		texcoord3 += texsize;
		texsize = -texsize;
	}

	renderer->AddBillboard(position, 32, affine2(texcoord1, texcoord1 + texsize));
	renderer->AddBillboard(position, 32, affine2(texcoord2, texcoord2 + texsize));
	renderer->AddBillboard(position, 32, affine2(texcoord3, texcoord3 + texsize));
}


void UnitCounter::AppendFacingMarker(TextureTriangleShape3* renderer, BattleView* battleView)
{
	const UnitCommand& command = _unit->GetCommand();

	if (_unit->state.unitMode != UnitMode_Standing
		|| command.meleeTarget != nullptr
		|| _unit->state.IsRouting())
	{
		return;
	}

	int xindex = 0;
	if (command.missileTarget == _unit)
	{
		xindex = 11;
	}
	else if (_unit->state.loadingDuration != 0)
	{
		xindex = 2 + (int)glm::round(9 * _unit->state.loadingTimer / _unit->state.loadingDuration);
		xindex = glm::min(10, xindex);
	}
	int yindex = 0;
	if (xindex >= 6)
	{
		xindex -= 6;
		yindex += 1;
	}

	float tx1 = xindex * 0.125f;
	float tx2 = tx1 + 0.125f;

	float ty1 = 0.75f + yindex * 0.125f;
	float ty2 = ty1 + 0.125f;

	bounds2f bounds = battleView->GetUnitCurrentFacingMarkerBounds(_unit);
	glm::vec2 p = bounds.center();
	float size = bounds.height();
	float direction = xindex != 0 || yindex != 0 ? -glm::half_pi<float>() : (_unit->state.bearing - battleView->GetCameraFacing());
	if (battleView->GetFlip())
		direction += glm::pi<float>();

	glm::vec2 d1 = size * vector2_from_angle(direction - glm::half_pi<float>() / 2.0f);
	glm::vec2 d2 = glm::vec2(d1.y, -d1.x);
	glm::vec2 d3 = glm::vec2(d2.y, -d2.x);
	glm::vec2 d4 = glm::vec2(d3.y, -d3.x);

	renderer->AddVertex(glm::vec3(p + d1, 0), glm::vec2(tx1, ty2));
	renderer->AddVertex(glm::vec3(p + d2, 0), glm::vec2(tx1, ty1));
	renderer->AddVertex(glm::vec3(p + d3, 0), glm::vec2(tx2, ty1));

	renderer->AddVertex(glm::vec3(p + d3, 0), glm::vec2(tx2, ty1));
	renderer->AddVertex(glm::vec3(p + d4, 0), glm::vec2(tx2, ty2));
	renderer->AddVertex(glm::vec3(p + d1, 0), glm::vec2(tx1, ty2));
}


void UnitCounter::AppendFighterWeapons(PlainLineShape3* renderer)
{
	if (_unit->stats.weaponReach > 0)
	{
		for (Fighter* fighter = _unit->fighters, * end = fighter + _unit->fightersCount; fighter != end; ++fighter)
		{
			glm::vec2 p1 = fighter->state.position;
			glm::vec2 p2 = p1 + _unit->stats.weaponReach * vector2_from_angle(fighter->state.bearing);

			renderer->AddLine(
				_battleView->GetSimulator()->GetHeightMap()->GetPosition(p1, 1),
				_battleView->GetSimulator()->GetHeightMap()->GetPosition(p2, 1));
		}
	}
}


void UnitCounter::AppendFighterBillboards(BillboardModel* billboardModel)
{
	bool isSameTeam = _unit->commander->GetTeam() == _battleView->GetCommander()->GetTeam();
	for (Fighter* fighter = _unit->fighters, * end = fighter + _unit->fightersCount; fighter != end; ++fighter)
	{
		float size = 2.0;
		int shape = 0;
		switch (_samuraiPlatform)
		{
			case SamuraiPlatform_Cav:
			case SamuraiPlatform_Gen:
				shape = isSameTeam ? billboardModel->_billboardShapeFighterCavBlue : billboardModel->_billboardShapeFighterCavRed;
				size = 3.0;
				break;

			case SamuraiPlatform_Sam:
				shape = isSameTeam ? billboardModel->_billboardShapeFighterSamBlue : billboardModel->_billboardShapeFighterSamRed;
				size = 2.0;
				break;

			case SamuraiPlatform_Ash:
				shape = isSameTeam ? billboardModel->_billboardShapeFighterAshBlue : billboardModel->_billboardShapeFighterAshRed;
				size = 2.0;
				break;
		}

		const float adjust = 0.5 - 2.0 / 64.0; // place texture 2 texels below ground
		glm::vec3 p = _battleView->GetSimulator()->GetHeightMap()->GetPosition(fighter->state.position, adjust * size);
		float facing = glm::degrees(fighter->state.bearing);
		billboardModel->dynamicBillboards.push_back(Billboard(p, facing, size, shape));
	}
}
