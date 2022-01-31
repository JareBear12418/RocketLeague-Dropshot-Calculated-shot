#pragma once
#pragma comment(lib, "pluginsdk.lib")
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "DropShotTile.h"

#include "Version.h"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

class DropShotCalculateShot : public BakkesMod::Plugin::BakkesModPlugin {
	bool is_on_blue_team = false;
	bool did_blue_score_last = false;
	std::vector<Vector> team_tiles; // The tiles to use depending on what team you're on.
	std::vector<int32_t> tiles_to_avoid; // The tiles to avoid depending on what team you're on.
	std::vector<DropShotTile> all_tiles;
	std::vector<BreakOutActorPlatformWrapper> all_platforms;
	int32_t ball_state = 0; // The balls current charge state.
	Vector ballLocation;
	float ball_charge = 0.0f; // The balls current charge level
	float carVelocity = 0.0f;
	float ball_speed = 0.0f;
	float ballLastHitTime = 0.0f;
	float ballhitDelta = 0.0f;
	float remainingEnergy;
	float distanceFromCarToBall = 0.0f;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();
	void Render(CanvasWrapper canvas);

	std::vector<int32_t> GetTileNeighbours(const DropShotTile& tile) const;
	std::vector<int32_t> GetTileNeighbours(const DropShotTile& tile, int32_t ball_state) const;
	void FindUpdatedTile(const Vector& ball_position);
	BreakOutActorPlatformWrapper FindBreakOutPlatformFromPosition(const Vector& ball_position);
	DropShotTile FindTileFromPostion(const Vector& position);
	bool DoesTileExist(const Vector& position);
	bool DoesBreakOutPlatformExist(const Vector& position);

	double DistanceTo(const Vector p1, const Vector p2);
	void UpdateAllTiles();
	int GetPlayerTeam();
	std::vector<int32_t> FindBestShot();
	std::vector<int32_t> FindOpenNets() const;
	std::vector<std::pair<Vector, Vector>> GetHexagonCornors(const DropShotTile& h);
	std::vector<std::pair<Vector, Vector>> GetHexagonCornors(const Vector& CenterPosition);
	std::vector<Vector> GetHexagonConnectors(const DropShotTile& h);
	std::vector<std::pair<Vector, Vector>> GetFilledHexagonCoordinates(const Vector& CenterPosition, const int numberOfLines);
	std::string ConvertToString(float& value);
	float CarVelocityTowardsBall(CarWrapper& car, BallWrapper& ball);
	void ResetVariables();
};