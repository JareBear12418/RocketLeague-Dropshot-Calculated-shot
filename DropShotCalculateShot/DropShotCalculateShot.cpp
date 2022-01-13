#include "pch.h"
#define LINMATH_H //Conflicts with linmath.h if we done declare this here
#include "DropShotCalculateShot.h"

#include "DropShotTile.h"
#include "VectorUtilities.h"
#include "RenderingTools/Objects/Frustum.h"
#include "RenderingTools/Objects/Line.h"
#include <algorithm>

BAKKESMOD_PLUGIN(DropShotCalculateShot, "Calculates the best tile to shoot at in dropshot.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;


/**
* Get the neighboring tiles depending on the balls state,
* @param tile - Postion of the tile
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::GetTileNeighbours(const DropShotTile& tile) const
{
	std::vector<int> neighbours;
	int search_radius;
	if (ball_state == 1)
	{
		search_radius = 1000;
	}
	else if (ball_state == 2)
	{
		search_radius = 1650;
	}
	else
	{
		//Invalid state?
		return neighbours;
	}
	for (auto& h : all_tiles)
	{
		if (DistanceTo(tile.center_position, h.center_position) < search_radius)
		{
			neighbours.push_back(h.id);
		}
	}


	return neighbours;
}

/**
* Get the neighboring tiles depending on 'ball_state',
* @param tile - Postion of the tile
* @param ball_state the pre set state of the ball for simulating ahead.
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::GetTileNeighbours(const DropShotTile& tile, int ball_state)
{
	std::vector<int> neighbours;
	int search_radius;
	if (ball_state == 1)
	{
		search_radius = 1000;
	}
	else if (ball_state == 2)
	{
		search_radius = 1650;
	}
	else
	{
		//Invalid state?
		return neighbours;
	}
	for (auto& h : all_tiles)
	{
		if (DistanceTo(tile.center_position, h.center_position) < search_radius)
		{
			neighbours.push_back(h.id);
		}
	}


	return neighbours;
}

/**
* This only runs when damage event has been called, we get the balls position and check if its within 
* the shape of the hexagon.
* @param ball_position - Position of the ball
*/
void DropShotCalculateShot::FindUpdatedTile(const Vector& ball_position)
{
	for (auto& tile : all_tiles)
	{
		// Building up the corners from the center position is repeated multiple times.
		// As far as I can see they're identical.
		// Extract to function for code reuse!
		Vector point1 = {tile.center_position.X, tile.center_position.Y + 443.41f, 0.0f};
		Vector point2 = {tile.center_position.X + 384.0f, tile.center_position.Y + 221.7f, 0.0f};
		Vector point3 = {tile.center_position.X + 384.0f, tile.center_position.Y - 221.7f, 0.0f};
		Vector point4 = {tile.center_position.X, tile.center_position.Y - 443.41f, 0.0f};
		Vector point5 = {tile.center_position.X - 384.0f, tile.center_position.Y - 221.7f, 0.0f};
		Vector point6 = {tile.center_position.X - 384.0f, tile.center_position.Y + 221.7f, 0.0f};

		std::vector<Vector> tile_corners[6] = {
			{point1, point2},
			{point2, point3},
			{point3, point4},
			{point4, point5},
			{point5, point6},
			{point6, point1}
		};
		if (IsPointInsidePolygon(ball_position, tile_corners))
		{
			if (ball_state == 0)
			{
				if (tile.state <= 2)
					tile.state++;
			}
			else
			{
				std::vector<int> neighbors = GetTileNeighbours(tile);
				for (int j : neighbors)
				{
					DropShotTile& neighbour = all_tiles[j];
					if (tile.state <= 2)
					{
						neighbour.state++;
					}
				}
			}
		}
	}
}

/**
* Calculates the best shot based on a number of factors.
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::FindBestShot()
{
	std::vector<int> tiles_with_most_damaged_neighbors;
	for (auto& tile : all_tiles)
	{
		std::vector<int> neighbours;
		if (ball_state == 0)
		{
			neighbours = GetTileNeighbours(tile, 1); // We need to act as the ball is charged with the 1
		}
		else
		{
			neighbours = GetTileNeighbours(tile);
		}
		float normal_counter = 0;
		float damaged_counter = 0;
		float opened_counter = 0;
		for (int j : neighbours)
		{
			if (all_tiles[j].state == 0)
				normal_counter++;
			else if (all_tiles[j].state == 1)
				damaged_counter++;
			else if (all_tiles[j].state == 2)
				opened_counter++;
		}
		if (normal_counter == 0)
			normal_counter = 1.0f;
		float ratio = damaged_counter / normal_counter;
		if (!(std::find(tiles_to_avoid.begin(), tiles_to_avoid.end(), tile.id) != tiles_to_avoid.end()) && tile.state != 2)
		{
			if (ball_state != 0)
			{
				if (ratio >= 1.2f && ball_state == 2 || ratio >= 4.0f && ball_state == 1 && tile.state != 2)
				{
					tiles_with_most_damaged_neighbors.push_back(tile.id);
				}
			}
			else
			{
				if (ratio >= 0.34f && ratio < 1.4f && tile.state != 1 && tile.state != 2)
				{
					tiles_with_most_damaged_neighbors.push_back(tile.id);
				}
			}
		}
	}
	return tiles_with_most_damaged_neighbors;
}

/**
* Find all tiles that have a state of 2, or are open nets.
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::FindOpenNets() const
{
	float opened_counter = 0;
	std::vector<int> tiles_with_most_opened_neighbors;
	for (auto& tile : all_tiles)
	{
		if (tile.state >= 2) // >= 2 because of reasons..
		{
			tiles_with_most_opened_neighbors.push_back(tile.id);
		}
	}
	return tiles_with_most_opened_neighbors;
}

/**
* Resets all the global variables, i don't know if you should change this but this works, and it took long enough to make it work properly
*/
void DropShotCalculateShot::ResetVariables()
{
	team_tiles.clear();
	tiles_to_avoid.clear();
	all_tiles.clear();
	if (is_on_blue_team)
	{
		tiles_to_avoid = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 24, 25, 35, 36, 45, 46, 54, 55, 62, 63, 64, 65, 66, 67, 68, 69};
		team_tiles = GetOrangeTiles();
	}
	else
	{
		tiles_to_avoid = {0, 1, 2, 3, 4, 5, 6, 7, 14, 15, 23, 24, 33, 34, 44, 45, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69};
		team_tiles = GetBlueTiles();
	}
	for (int i = 0; i < 70; i++)
	{
		DropShotTile h{i, 0, {team_tiles[i].X, team_tiles[i].Y}};
		all_tiles.push_back(h);
	}
	LOG("Variables reset");
}

void DropShotCalculateShot::onLoad()
{
	_globalCvarManager = cvarManager;
	cvarManager->log("Plugin loaded!");

	gameWrapper->RegisterDrawable(bind(&DropShotCalculateShot::Render, this, std::placeholders::_1));
	ResetVariables();

	//Function TAGame.Ball_Breakout_TA.EventAppliedDamage
	struct EventAppliedDamageParams
	{
		uintptr_t Ball; // special breakout ball, castable to BallWrapper
		uint8_t Id; // maybe tile ids exist???
		Vector Location;
		int8_t DamageIndex;
		int8_t TotalDamage;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.EventAppliedDamage",
		[this](BallWrapper caller, void* params, std::string eventname)
		{
			EventAppliedDamageParams* param = (EventAppliedDamageParams*)params;
			//if the other team deals damage on your side very close to the middle of the field, it 'might' cause issues, not tested, but was a concern.
			if (is_on_blue_team && caller.GetLocation().Y > 0)
				FindUpdatedTile(caller.GetLocation());
			else if (!is_on_blue_team && caller.GetLocation().Y < 0)
				FindUpdatedTile(caller.GetLocation());
		});

	//Function TAGame.Ball_Breakout_TA.SetDamageIndex
	struct SetDamageIndexParams
	{
		int32_t InIndex;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.SetDamageIndex",
		[this](BallWrapper caller, void* params, std::string eventname)
		{
			SetDamageIndexParams* param = (SetDamageIndexParams*)params;
			int newIndex = param->InIndex;
			if (newIndex == 0)
			{
				gameWrapper->SetTimeout([this, newIndex](GameWrapper* gw)
				{
					ball_state = newIndex;
				}, 0.15f);
			}
			else
			{
				ball_state = newIndex;
			}
		});

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function Engine.PlayerReplicationInfo.OnTeamChanged",
		[this](BallWrapper caller, void* params, std::string eventname)
		{
			ServerWrapper sw = gameWrapper->GetCurrentGameState();
			if (!sw) return;

			auto primary = sw.GetLocalPrimaryPlayer();

			if (primary.IsNull()) return;

			auto primaryPri = primary.GetPRI();

			if (primaryPri.IsNull()) return;

			int teamNumber = primaryPri.GetTeamNum2();

			if (teamNumber == 0 && !is_on_blue_team)
			{
				LOG("Changed team to blue team");
				is_on_blue_team = true;
				ResetVariables();
			}
			else if (teamNumber == 1 && is_on_blue_team)
			{
				LOG("Changed team to orange team");
				is_on_blue_team = false;
				ResetVariables();
			}
			else
			{
				LOG("Still on the same team. is on blue team: {}", is_on_blue_team);
			}
		});

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.GameEvent_Soccar_TA.EventGoalScored",
		[this](BallWrapper caller, void* params, std::string eventname)
		{
			LOG("X: {} Y: {}", ballLocation.X, ballLocation.Y);
			if (ballLocation.Y > 0 && is_on_blue_team)
			{
				LOG("BLUE SCORED");
				gameWrapper->SetTimeout([this](GameWrapper* gw)
				{
					ResetVariables();
				}, 0.15f);
			}
			else if (ballLocation.Y < 0 && !is_on_blue_team)
			{
				LOG("ORANGE SCORED");
				gameWrapper->SetTimeout([this](GameWrapper* gw)
				{
					ResetVariables();
				}, 0.15f);
			}
		});
	/*gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
			reset_variables();
		});*/
	//gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", [this](std::string eventName) {
	//	reset_variables();
	//	});
}

void DropShotCalculateShot::onUnload() {}


void DropShotCalculateShot::Render(CanvasWrapper canvas)
{
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) { return; }
	BallWrapper ball = server.GetBall();
	if (!ball) { return; }
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) { return; }
	auto camera = gameWrapper->GetCamera();
	if (camera.IsNull()) return;
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	if (teams.IsNull()) return;
	int team_number = 0;
	if (is_on_blue_team)
		team_number = 0;
	else
		team_number = 1;
	TeamWrapper team = teams.Get(team_number);
	if (!team) return;
	RT::Frustum frust{canvas, camera};
	if (ball.GetLocation().X != 0.0 && ball.GetLocation().Y != 0.0 && ball.GetLocation().Z >= 0)
		ballLocation = {ball.GetLocation().X, ball.GetLocation().Y, ball.GetLocation().Z};
	std::vector<int> best_shot_tiles = FindBestShot();
	LOG("SIZE: {}", best_shot_tiles.size());
	for (int best_shot_tile : best_shot_tiles)
	{
		const DropShotTile& h = all_tiles[best_shot_tile];
		if (h.state != 2)
		{
			Vector point1 = {h.center_position.X, h.center_position.Y + 443.41f, 0.0f};
			Vector point2 = {h.center_position.X + 384.0f, h.center_position.Y + 221.7f, 0.0f};
			Vector point3 = {h.center_position.X + 384.0f, h.center_position.Y - 221.7f, 0.0f};
			Vector point4 = {h.center_position.X, h.center_position.Y - 443.41f, 0.0f};
			Vector point5 = {h.center_position.X - 384.0f, h.center_position.Y - 221.7f, 0.0f};
			Vector point6 = {h.center_position.X - 384.0f, h.center_position.Y + 221.7f, 0.0f};

			std::vector<Vector> hexagon[6] = {
				{point1, point2},
				{point2, point3},
				{point3, point4},
				{point4, point5},
				{point5, point6},
				{point6, point1}
			};
			canvas.SetColor(0, 208, 0, 130);
			for (auto& j : hexagon)
			{
				RT::Line(j[0], j[1], 3.0f).DrawWithinFrustum(canvas, frust);
			}
		}
	}
	std::vector<int> open_nets = FindOpenNets();
	for (int open_net : open_nets)
	{
		const DropShotTile& h = all_tiles[open_net];
		Vector point1 = {h.center_position.X, h.center_position.Y + 443.41f, 0.0f};
		Vector point2 = {h.center_position.X + 384.0f, h.center_position.Y + 221.7f, 0.0f};
		Vector point3 = {h.center_position.X + 384.0f, h.center_position.Y - 221.7f, 0.0f};
		Vector point4 = {h.center_position.X, h.center_position.Y - 443.41f, 0.0f};
		Vector point5 = {h.center_position.X - 384.0f, h.center_position.Y - 221.7f, 0.0f};
		Vector point6 = {h.center_position.X - 384.0f, h.center_position.Y + 221.7f, 0.0f};

		std::vector<Vector> hexagon[6] = {
			{point1, point2},
			{point2, point3},
			{point3, point4},
			{point4, point5},
			{point5, point6},
			{point6, point1}
		};
		canvas.SetColor(team.GetPrimaryColor().R * 255, team.GetPrimaryColor().G * 255, team.GetPrimaryColor().B * 255, 150);
		for (auto& j : hexagon)
		{
			RT::Line(j[0], j[1], 3.0f).DrawWithinFrustum(canvas, frust);
		}
	}

	//canvas.SetColor(0, 208, 0, 100.0f);
	//canvas.DrawTexture(hexagon_image.get(), 1); // there are multiple functions in the canvaswrapper that accept ImageWrapper*


	//Rotator rotator(0, 0, x);
	//canvas.SetPosition(h.center_position);
	//canvas.Project(Vector{ h.center_position.X , h.center_position.Y, 0});

	/*canvas.DrawRotatedTile(hexagon_image.get(), rotator, 296, 340, 1, 1, 296, 340, 0.5f, 0.5f);
	
	// This is code to draw a hexagon with triangles... its pointless but Im keeping it around.. who knows if transparent triangles will come.
	if (frust.IsInFrustum(hexagon[0][0])) {
		RT::Triangle::Triangle(hexagon[0][0], hexagon[0][1], hexagon[2][0]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[2][0], hexagon[2][1], hexagon[5][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[5][1], hexagon[4][1], hexagon[2][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[3][0], hexagon[4][0], hexagon[4][1]).Draw(canvas);
	}*/
}
