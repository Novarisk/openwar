// Copyright (C) 2013 Felix Ungman
//
// This file is part of the openwar platform (GPL v3 or later), see LICENSE.txt

#ifndef BattleSimulator_H
#define BattleSimulator_H

#include <map>
#include <set>
#include <string>
#include "../Library/Algorithms/quadtree.h"
#include "MovementRules.h"
#import "GroundMap.h"

class BattleCommander;
class GroundMap;
class HeightMap;
struct Fighter;
struct Unit;


enum class PlatformType
{
	None,
	Infantry,
	Cavalry
};


enum class MissileType
{
	None,
	Bow,
	Arq
};


enum UnitMode
{
	UnitMode_Initializing,
	UnitMode_Standing,
	UnitMode_Moving,
	UnitMode_Turning
};


struct Projectile
{
	glm::vec2 position1;
	glm::vec2 position2;
	float delay;

	Projectile() :
	position1(),
	position2(),
	delay(0)
	{
	}
};


struct Shooting
{
	Unit* unit;
	MissileType missileType;
	float timeToImpact;
	glm::vec2 target;
	bool released;
	std::vector<Projectile> projectiles;

	Shooting() :
	unit(nullptr),
	missileType(MissileType::None),
	timeToImpact(0),
	released(false),
	projectiles()
	{
	}
};


enum ReadyState
{
	ReadyState_Unready,
	ReadyState_Readying,
	ReadyState_Prepared,
	ReadyState_Striking,
	ReadyState_Stunned
};


struct FighterState
{
	// dynamic attributes
	glm::vec2 position;
	float position_z;
	ReadyState readyState;
	float readyingTimer;
	float strikingTimer;
	float stunnedTimer;
	Fighter* opponent;

	// intermediate attributes
	glm::vec2 destination;
	glm::vec2 velocity;
	float bearing;
	Fighter* meleeTarget;

	FighterState() :
	position(),
	position_z(0),
	readyState(ReadyState_Unready),
	readyingTimer(0),
	strikingTimer(0),
	stunnedTimer(0),
	opponent(nullptr),
	destination(),
	velocity(),
	bearing(0),
	meleeTarget(nullptr)
	{
	}
};


struct Fighter
{
	// static attributes
	Unit* unit;

	// dynamic attributes
	FighterState state;

	// optimization attributes
	bool terrainForest;
	bool terrainImpassable;
	glm::vec2 terrainPosition;

	// intermediate attributes
	FighterState nextState;
	bool casualty;


	Fighter() :
	unit(nullptr),
	state(),
	terrainForest(false),
	terrainImpassable(false),
	terrainPosition(),
	nextState(),
	casualty(false)
	{
	}
};


struct UnitStats
{
	PlatformType platformType;
	MissileType missileType;

	float weaponReach;
	float trainingLevel;
	float strikingDuration;
	float readyingDuration;
	float fireRate; // shots per minute
	float minimumRange; // minimum fire range in meters
	float maximumRange; // maximum fire range in meters
	float fireAccuracy; // percentage of hit at half range
	float walkingSpeed; // meters per second
	float runningSpeed; // meters per second
	glm::vec2 fighterSize; // x = side-to-side, y = front-to-back
	glm::vec2 spacing;  // x = side-to-side, y = front-to-back

	UnitStats() :
	platformType(PlatformType::None),
	missileType(MissileType::None),
	weaponReach(0),
	trainingLevel(0),
	strikingDuration(0),
	readyingDuration(0),
	fireRate(25.0f), // rounds per minute
	minimumRange(0),
	maximumRange(0),
	fireAccuracy(0.7f),
	walkingSpeed(0),
	runningSpeed(0),
	fighterSize(glm::vec2(10, 10)),
	spacing(glm::vec2(10, 10))
	{
	}
};


struct UnitRange
{
	glm::vec2 center;
	float angleStart;
	float angleLength;
	float minimumRange;
	float maximumRange;
	std::vector<float> actualRanges;

	UnitRange() :
	center(),
	angleStart(0),
	angleLength(0),
	minimumRange(0),
	maximumRange(0),
	actualRanges()
	{

	}

	bool IsWithinRange(glm::vec2 p) const;
};


struct UnitState
{
	// dynamic attributes
	float loadingTimer;
	float loadingDuration;
	int shootingCounter;
	float morale;

	// optimization attributes
	UnitMode unitMode;
	glm::vec2 center;
	float bearing;
	float influence;

	// intermediate attributes
	int recentCasualties;

	glm::vec2 waypoint;


	bool IsRouting() const
	{
		float x = morale + influence;
		return x <= 0;
	}

	float GetRoutingBlinkTime() const
	{
		float x = morale + influence;
		return 0 <= x && x < 0.33f ? 0.1f + x * 3 : 0;
	}

	UnitState() :
	loadingTimer(0),
	loadingDuration(0),
	shootingCounter(0),
	morale(1),
	unitMode(UnitMode_Initializing),
	center(),
	bearing(0),
	influence(0),
	recentCasualties(0),
	waypoint()
	{
	}
};



struct UnitCommand
{
	std::vector<glm::vec2> path;
	bool running;
	float bearing;
	Unit* meleeTarget;
	Unit* missileTarget; // set to self to hold fire
	bool missileTargetLocked;

	UnitCommand() :
	path(),
	running(false),
	bearing(0),
	meleeTarget(nullptr),
	missileTarget(nullptr),
	missileTargetLocked(false)
	{
	}

	void UpdatePath(glm::vec2 curr, glm::vec2 dest)
	{
		MovementRules::UpdateMovementPath(path, curr, dest);
	}

	void ClearPathAndSetDestination(glm::vec2 p)
	{
		path.clear();
		path.push_back(p);
	}

	glm::vec2 GetDestination() const
	{
		return !path.empty() ? path.back() : glm::vec2(512, 512);
	}
};


struct Unit
{
	// static attributes
	BattleCommander* commander;
	std::string unitClass;
	UnitStats stats;
	Fighter* fighters;

	// dynamic attributes
	UnitState state; // updated by AssignNextState()
	int fightersCount; // updated by RemoveCasualties()
	int shootingCounter; // updated by ResolveMissileCombat()
	Formation formation; // updated by UpdateFormation()
	float timeUntilSwapFighters;

	// intermediate attributes
	UnitState nextState; // updated by ComputeNextState()
	UnitRange unitRange;

	// control attributes
	UnitCommand command;
	UnitCommand nextCommand;
	float nextCommandTimer;

	Unit() :
	commander(nullptr),
	stats(),
	fighters(nullptr),
	state(),
	fightersCount(0),
	shootingCounter(0),
	formation(),
	timeUntilSwapFighters(0),
	nextState(),
	unitRange(),
	command(),
	nextCommand(),
	nextCommandTimer(0)
	{
	}

	glm::vec2 CalculateUnitCenter();

	float GetSpeed();
	const UnitCommand& GetCommand() const { return nextCommandTimer > 0 ? nextCommand : command; }

	static int GetFighterRank(Fighter* fighter);
	static int GetFighterFile(Fighter* fighter);
	static Fighter* GetFighter(Unit* unit, int rank, int file);
};



class BattleObserver
{
public:
	virtual ~BattleObserver();

	virtual void OnSetGroundMap(GroundMap* groundMap) = 0;
	virtual void OnAddUnit(Unit* unit) = 0;
	virtual void OnRemoveUnit(Unit* unit) = 0;
	virtual void OnCommand(Unit* unit, float timer) = 0;
	virtual void OnShooting(const Shooting& shooting, float timer) = 0;
	virtual void OnRelease(const Shooting& shooting) = 0;
	virtual void OnCasualty(const Fighter& fighter) = 0;
	virtual void OnRouting(Unit* unit) = 0;
};


class BattleSimulator
{
	std::set<BattleObserver*> _observers;
	quadtree<Fighter*> _fighterQuadTree;
	quadtree<Fighter*> _weaponQuadTree;

	HeightMap* _heightMap;
	GroundMap* _groundMap;

	std::vector<Unit*> _units;
	std::vector<std::pair<float, Shooting>> _shootings;
	std::map<int, int> _kills;

	float _secondsSinceLastTimeStep;
	float _timeStep;
	bool _practice;
	int _winnerTeam;

public:
	BattleSimulator();
	~BattleSimulator();

	float GetTimeStep() const { return _timeStep; }
	float GetTimerDelay() const { return 0.25f; }

	void SetPractice(bool value) { _practice = value; }
	int GetKills(int team) { return _kills[team]; }

	void AddObserver(BattleObserver* observer);
	void RemoveObserver(BattleObserver* observer);

	HeightMap* GetHeightMap() { return _heightMap; }

	void SetGroundMap(GroundMap* groundMap);
	GroundMap* GetGroundMap() { return _groundMap; }

	const std::vector<Unit*>& GetUnits() { return _units; }
	Unit* AddUnit(BattleCommander* commander, const char* unitClass, int numberOfFighters, UnitStats stats, glm::vec2 position);
	void RemoveUnit(Unit* unit);

	void SetUnitCommand(Unit* unit, const UnitCommand& command, float timer);

	void AddShooting(const Shooting& shooting, float timer);

	void AdvanceTime(float secondsSinceLastTime);

	bool IsMelee() const;
	int GetWinnerTeam() const { return _winnerTeam; }

private:
	void SimulateOneTimeStep();

	void RebuildQuadTree();

	void ComputeNextState();
	void AssignNextState();
	void UpdateUnitRange(Unit* unit);

	void ResolveMeleeCombat();
	void ResolveMissileCombat();

	void TriggerShooting(Unit* unit);
	void ResolveProjectileCasualties();

	void RemoveCasualties();
	void RemoveDeadUnits();
	void RemoveFinishedShootings();

	UnitState NextUnitState(Unit* unit);
	UnitMode NextUnitMode(Unit* unit);
	float NextUnitDirection(Unit* unit);

	FighterState NextFighterState(Fighter* fighter);
	glm::vec2 NextFighterPosition(Fighter* fighter);
	glm::vec2 NextFighterVelocity(Fighter* fighter);

	Fighter* FindFighterStrikingTarget(Fighter* fighter);

	bool IsWithinLineOfFire(Unit* unit, glm::vec2 position);
	Unit* ClosestEnemyWithinLineOfFire(Unit* unit);
};



#endif
