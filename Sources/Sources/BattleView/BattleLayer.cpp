#include "../../BattleModel/BattleScenario.h"
#include "../../BattleModel/BattleSimulator.h"
#include "../../Library/Audio/SoundPlayer.h"
#include "BattleGesture.h"
#include "BattleView.h"
#include "BattleLayer.h"
#include "UnitCounter.h"
#include "EditorGesture.h"
#include "EditorModel.h"
#include "../SmoothTerrain/SmoothTerrainWater.h"
#include "../SmoothTerrain/SmoothTerrainSky.h"
#include "../TerrainView/TerrainGesture.h"
#include "Surface.h"

#include <glm/gtc/matrix_transform.hpp>


BattleLayer::BattleLayer() :
_renderers(nullptr),
_playing(false),
_editing(false),
_scenario(nullptr),
_editorGesture(nullptr),
_editorModel(nullptr)
{
	if (renderers::singleton == nullptr)
		renderers::singleton = new renderers();

	_renderers = renderers::singleton;

	SoundPlayer::Initialize();
	SoundPlayer::singleton->Pause();
}


BattleLayer::~BattleLayer()
{
}


void BattleLayer::ResetBattleViews(BattleScenario* scenario, const std::vector<BattleCommander*>& commanders)
{
	delete _editorGesture;
	_editorGesture = nullptr;
	delete _editorModel;
	_editorModel = nullptr;

	_scenario = scenario;
	_commanders = commanders;

	int count = scenario != nullptr ? (int)commanders.size() : 0;

	while ((int)_battleViews.size() > count)
		RemoveBattleView(_battleViews.back());

	for (int i = 0; i < count; ++i)
	{
		BattleCommander* commander = _commanders[i];
		if (i < (int)_battleViews.size())
			ResetBattleView(_battleViews[i], commander);
		else
			CreateBattleView(commander);
	}

	UpdateBattleViewSize();
}


void BattleLayer::ResetEditor(BattleScenario* scenario)
{
	delete _editorGesture;
	_editorGesture = nullptr;
	delete _editorModel;
	_editorModel = nullptr;

	_scenario = scenario;
	_commanders.clear();

	while ((int)_battleViews.size() > 1)
		RemoveBattleView(_battleViews.back());

	if (_battleViews.empty())
		CreateBattleView(nullptr);
	else
		ResetBattleView(_battleViews.front(), nullptr);

	_editorModel = new EditorModel(_battleViews.front(), _battleViews.front()->GetSmoothTerrainRenderer());
	_editorGesture = new EditorGesture(_battleViews.front(), _editorModel);

	_editorModel->AddObserver(this);

	UpdateBattleViewSize();
}


void BattleLayer::ResetCameraPosition()
{
	for (BattleView* battleView : _battleViews)
		battleView->InitializeCameraPosition();
}


void BattleLayer::SetPlaying(bool value)
{
	_playing = value;

	if (_playing)
		SoundPlayer::singleton->Resume();
	else
		SoundPlayer::singleton->Pause();

	for (BattleGesture* gesture : _battleGestures)
		gesture->SetEnabled(_playing);
}


void BattleLayer::SetEditing(bool value)
{
	_editing = value;

	for (TerrainGesture* gesture : _terrainGestures)
		gesture->SetEnabled(!_editing);
}


void BattleLayer::OnFrameChanged()
{
	Container::OnFrameChanged();
	UpdateBattleViewSize();
}


/* EditorModelObserver */

void BattleLayer::OnEditorModeChanged(EditorModel* editorModel)
{
	SetEditing(editorModel->GetEditorMode() != EditorMode::Hand);
}


void BattleLayer::OnTerrainFeatureChanged(EditorModel* editorModel)
{
}


/***/


void BattleLayer::Update(double secondsSinceLastUpdate)
{
	if (_scenario != nullptr)
	{
		if (_playing)
			_scenario->Tick(secondsSinceLastUpdate);
		else
			_scenario->Tick(0);

		if (_playing)
			UpdateSoundPlayer();
	}

	Container::Update(secondsSinceLastUpdate);

	for (BattleView* battleView : _battleViews)
	{
		battleView->AnimateMarkers((float)secondsSinceLastUpdate);
	}
}


bool BattleLayer::NeedsRender() const
{
	return true;
}


void BattleLayer::CreateBattleView(BattleCommander* commander)
{
	BattleSimulator* simulator = _scenario->GetSimulator();

	BattleView* battleView = new BattleView(GetSurface()->GetGraphicsContext(), _renderers);
	battleView->SetContainer(this);
	battleView->SetCommander(commander);
	battleView->SetSimulator(simulator);

	//if (commander->GetConfiguration()[0] == '-')
	//{
	//	battleView->SetFlip(true);
	//	battleView->SetCameraFacing((float)M_PI);
	//}

	_battleViews.push_back(battleView);
	UpdateBattleViewSize();

	battleView->Initialize();

	BattleGesture* battleGesture = new BattleGesture(battleView);
	_battleGestures.push_back(battleGesture);

	TerrainGesture* terrainGesture = new TerrainGesture(battleView);
	_terrainGestures.push_back(terrainGesture);
}


void BattleLayer::ResetBattleView(BattleView* battleView, BattleCommander* commander)
{
	battleView->SetCommander(commander);
	battleView->SetSimulator(_scenario->GetSimulator());

	for (auto i = _battleGestures.begin(); i != _battleGestures.end(); ++i)
		if ((*i)->GetBattleView() == battleView)
		{
			delete *i;
			*i = new BattleGesture(battleView);
		}

	for (auto i = _terrainGestures.begin(); i != _terrainGestures.end(); ++i)
		if ((*i)->GetTerrainView() == battleView)
		{
			delete *i;
			*i = new TerrainGesture(battleView);
		}
}


void BattleLayer::RemoveBattleView(BattleView* battleView)
{
	for (auto i = _battleGestures.begin(); i != _battleGestures.end(); )
	{
		if ((*i)->GetBattleView() == battleView)
		{
			delete *i;
			i = _battleGestures.erase(i);
		}
		else
		{
			++i;
		}
	}

	for (auto i = _terrainGestures.begin(); i != _terrainGestures.end(); )
	{
		if ((*i)->GetTerrainView() == battleView)
		{
			delete *i;
			i = _terrainGestures.erase(i);
		}
		else
		{
			++i;
		}
	}

	for (auto i = _battleViews.begin(); i != _battleViews.end(); )
	{
		if (*i == battleView)
			i = _battleViews.erase(i);
		else
			++i;
	}

	battleView->GetSimulator()->RemoveObserver(battleView);
	delete battleView;
}


void BattleLayer::UpdateBattleViewSize()
{
	//SetFrame(bounds2f(0, 0, GetSize()));

	if (!_battleViews.empty())
	{
		glm::vec2 size = GetSize();
		float h = size.y / _battleViews.size();
		float y = 0;

		for (BattleView* battleView : _battleViews)
		{
			bounds2f frame = bounds2f(0, y, size.x, y + h);
			battleView->SetFrame(frame);
			battleView->SetViewport(frame);
			y += h;
		}
	}
}


void BattleLayer::UpdateSoundPlayer()
{
	if (_playing && !_battleViews.empty())
	{
		int horseGallop = 0;
		int horseTrot = 0;
		int fighting = 0;
		int infantryMarching = 0;
		int infantryRunning = 0;

		for (UnitCounter* unitMarker : _battleViews.front()->GetUnitCounters())
		{
			Unit* unit = unitMarker->_unit;
			if (glm::length(unit->command.GetDestination() - unit->state.center) > 4.0f)
			{
				if (unit->stats.platformType == PlatformType::Cavalry)
				{
					if (unit->command.running)
						++horseGallop;
					else
						++horseTrot;
				}
				else
				{
					if (unit->command.running)
						++infantryRunning;
					else
						++infantryMarching;
				}
			}

			if (unit->command.meleeTarget != nullptr)
				++fighting;
		}

		SoundPlayer::singleton->UpdateInfantryWalking(infantryMarching != 0);
		SoundPlayer::singleton->UpdateInfantryRunning(infantryRunning != 0);

		SoundPlayer::singleton->UpdateCavalryWalking(horseTrot != 0);
		SoundPlayer::singleton->UpdateCavalryRunning(horseGallop != 0);

		SoundPlayer::singleton->UpdateFighting(_scenario->GetSimulator()->IsMelee());
	}
}
