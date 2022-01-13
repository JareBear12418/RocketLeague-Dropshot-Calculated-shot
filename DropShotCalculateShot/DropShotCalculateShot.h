#pragma once

#include "DropShotTile.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class DropShotCalculateShot : public BakkesMod::Plugin::BakkesModPlugin/*, public BakkesMod::Plugin::PluginSettingsWindow*/ /*, public BakkesMod::Plugin::PluginWindow*/
{
	bool is_on_blue_team = false;
	std::vector<DropShotTile> all_tiles;
	std::vector<Vector> team_tiles; // The tiles to use depending on what team you are on
	std::vector<int> tiles_to_avoid; // depends on selected team
	int32_t ball_state = 0; // the balls current state
	Vector ballLocation;

	//Boilerplate
	virtual void onLoad();
	virtual void onUnload();
	void Render(CanvasWrapper canvas);

	std::vector<int> GetTileNeighbours(const DropShotTile& tile) const;
	std::vector<int> GetTileNeighbours(const DropShotTile& tile, int ball_state);
	void FindUpdatedTile(const Vector& ball_position);
	std::vector<int> FindBestShot();
	std::vector<int> FindOpenNets() const;
	void ResetVariables();


	// Inherited via PluginSettingsWindow
	/*
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
	*/

	// Inherited via PluginWindow
	/*

	bool isWindowOpen_ = false;
	bool isMinimized_ = false;
	std::string menuTitle_ = "DropShotCalculateShot";

	virtual void Render() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual void SetImGuiContext(uintptr_t ctx) override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
	
	*/
};
