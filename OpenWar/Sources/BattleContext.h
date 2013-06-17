// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef BATTLECONTEXT_H
#define BATTLECONTEXT_H

class BattleModel;
class SimulationRules;
class SimulationState;
class SmoothTerrainModel;
class TerrainFeatureModel;
class TerrainModel;
class TiledTerrainModel;

class BattleContext
{
public:
	BattleContext();
	~BattleContext();

	SimulationState* simulationState;
	SimulationRules* simulationRules;

	SmoothTerrainModel* smoothTerrainModel;
	TiledTerrainModel* tiledTerrainModel;

	TerrainFeatureModel* terrainFeatureModel;
	BattleModel* battleModel;

	TerrainModel* GetTerrainModel() const { return (TerrainModel*)smoothTerrainModel ?: (TerrainModel*)tiledTerrainModel; }
};



#endif