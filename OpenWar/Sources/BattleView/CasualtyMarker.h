// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef CasualtyMarker_H
#define CasualtyMarker_H

#include "SimulationState.h"


class CasualtyMarker
{
public:
	struct Casualty
	{
		glm::vec3 position;
		Player player;
		UnitPlatform platform;
		float time;
		int seed;

		Casualty(glm::vec3 position_, Player player_, UnitPlatform platform_) :
		position(position_), player(player_), platform(platform_), time(0), seed(rand() & 0x7fff) { }
	};
	std::vector<Casualty> casualties;

public:
	CasualtyMarker();
	~CasualtyMarker();

	void AddCasualty(glm::vec3 position, Player player, UnitPlatform platform);
	bool Animate(float seconds);
};


#endif
