// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef BattleScript_H
#define BattleScript_H

#include "BattleSimulator.h"
#include "lua.hpp"


class BattleScript
{
	struct UnitStatus
	{
		glm::vec2 position;
		float heading;
		int state;
		int strength;
		float morale;

		UnitStatus(Unit* unit);
	};

	BattleSimulator* _battleSimulator;
	//GradientLineRenderer* _renderer;
	lua_State* _state;

public:
	BattleScript();
	~BattleScript();

	void SetGlobalNumber(const char* name, double value);
	void SetGlobalString(const char* name, const char* value);
	void AddStandardPath();
	void AddPackagePath(const char* path);
	void Execute(const char* script, size_t length);

	BattleSimulator* GetBattleSimulator() const { return _battleSimulator; }

	void CreateBattleSimulator();

	void Tick(double secondsSinceLastTick);
	//void RenderHints(GradientLineRenderer* renderer);

private:
	int NewUnit(int player, int team, const char* unitClass, int strength, glm::vec2 position, float bearing);
	void SetUnitMovement(int unitId, bool running, std::vector<glm::vec2> path, int chargeId, float heading);

	static int openwar_terrain_init(lua_State* L);
	static int openwar_simulator_init(lua_State* L);

	static int openwar_render_hint_line(lua_State* L);
	static int openwar_render_hint_circle(lua_State* L);

	static int battle_message(lua_State* L);
	static int battle_get_time(lua_State* L);
	static int battle_new_unit(lua_State* L);
	static int battle_set_unit_movement(lua_State* L);
	static int battle_get_unit_status(lua_State* L);

	static int battle_set_terrain_tile(lua_State* L);
	static int battle_set_terrain_height(lua_State* L);
	static int battle_add_terrain_tree(lua_State* L);

	static void ToPath(std::vector<glm::vec2>& result, lua_State* L, int index);
};


#endif
