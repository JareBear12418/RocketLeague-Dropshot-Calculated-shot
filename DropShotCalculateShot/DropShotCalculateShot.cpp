#define LINMATH_H //Conflicts with linmath.h if we done declare this here
#include "pch.h"
#include "DropShotCalculateShot.h"
#include "RenderingTools/Objects/Frustum.h"
#include "RenderingTools/Objects/Line.h"
#include "RenderingTools/Objects/Triangle.h"
#include "RenderingTools/Extra/WrapperStructsExtensions.h"
#include "RenderingTools/Extra/RenderingMath.h"
#include <algorithm>

BAKKESMOD_PLUGIN(DropShotCalculateShot, "Calculates the best tile to shoot at in dropshot.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
struct Hexagon {
	int id; // the tile id
	int state; // the tiles state from 0-2, 0 being normal, 1 being damaged, 2 being opened
	Vector2F center_position; // The center position of the tile.
};
std::vector<Hexagon> all_hexagons;

Vector orange_tiles[70] = {
	  {4608.0, 128.0, 0.0},
	  {3840.0, 128.0, 0.0},
	  {3072.0, 128.0, 0.0},
	  {2304.0, 128.0, 0.0},
	  {1536.0, 128.0, 0.0},
	  {768.0, 128.0, 0.0},
	  {-0.0, 128.0, 0.0},
	  {-768.0, 128.0, 0.0},
	  {-1536.0, 128.0, 0.0},
	  {-2304.0, 128.0, 0.0},
	  {-3072.0, 128.0, 0.0},
	  {-3840.0, 128.0, 0.0},
	  {-4608.0, 128.0, 0.0},
	  {4224.0, 793.1079711914062, 0.0},
	  {3456.0, 793.1079711914062, 0.0},
	  {2688.0, 793.1079711914062, 0.0},
	  {1920.0, 793.1079711914062, 0.0},
	  {1152.0, 793.1079711914062, 0.0},
	  {384.0, 793.1079711914062, 0.0},
	  {-384.0, 793.1079711914062, 0.0},
	  {-1152.0, 793.1079711914062, 0.0},
	  {-1920.0, 793.1079711914062, 0.0},
	  {-2688.0, 793.1079711914062, 0.0},
	  {-3456.0, 793.1079711914062, 0.0},
	  {-4224.0, 793.1079711914062, 0.0},
	  {3840.0, 1458.21484375, 0.0},
	  {3072.0, 1458.21484375, 0.0},
	  {2304.0, 1458.21484375, 0.0},
	  {1536.0, 1458.21484375, 0.0},
	  {768.0, 1458.21484375, 0.0},
	  {-0.0, 1458.21484375, 0.0},
	  {-768.0, 1458.21484375, 0.0},
	  {-1536.0, 1458.21484375, 0.0},
	  {-2304.0, 1458.21484375, 0.0},
	  {-3072.0, 1458.21484375, 0.0},
	  {-3840.0, 1458.21484375, 0.0},
	  {3456.0, 2123.322021484375, 0.0},
	  {2688.0, 2123.322021484375, 0.0},
	  {1920.0, 2123.322021484375, 0.0},
	  {1152.0, 2123.322021484375, 0.0},
	  {384.0, 2123.322021484375, 0.0},
	  {-384.0, 2123.322021484375, 0.0},
	  {-1152.0, 2123.322021484375, 0.0},
	  {-1920.0, 2123.322021484375, 0.0},
	  {-2688.0, 2123.322021484375, 0.0},
	  {-3456.0, 2123.322021484375, 0.0},
	  {3072.0, 2788.428955078125, 0.0},
	  {2304.0, 2788.428955078125, 0.0},
	  {1536.0, 2788.428955078125, 0.0},
	  {768.0, 2788.428955078125, 0.0},
	  {-0.0, 2788.428955078125, 0.0},
	  {-768.0, 2788.428955078125, 0.0},
	  {-1536.0, 2788.428955078125, 0.0},
	  {-2304.0, 2788.428955078125, 0.0},
	  {-3072.0, 2788.428955078125, 0.0},
	  {2688.0, 3452.322021484375, 0.0},
	  { 1920.0, 3452.322021484375, 0.0 },
	  { 1152.0, 3452.322021484375, 0.0 },
	  { 384.0, 3452.322021484375, 0.0 },
	  { -384.0, 3452.322021484375, 0.0 },
	  { -1152.0, 3452.322021484375, 0.0 },
	  { -1920.0, 3452.322021484375, 0.0 },
	  { -2688.0, 3452.322021484375, 0.0 },
	  { 2304.0, 4117.4287109375, 0.0 },
	  { 1536.0, 4117.4287109375, 0.0 },
	  { 768.0, 4117.4287109375, 0.0 },
	  { -0.0, 4117.4287109375, 0.0 },
	  { -768.0, 4117.4287109375, 0.0 },
	  { -1536.0, 4117.4287109375, 0.0 },
	  { -2304.0, 4117.4287109375, 0.0 }
};
Vector blue_tiles[70] = {
	{2304.0, -4117.4287109375, 2.5},
	{1536.0, -4117.4287109375, 2.5},
	{768.0, -4117.4287109375, 2.5},
	{-0.0, -4117.4287109375, 2.5},
	{-768.0, -4117.4287109375, 2.5},
	{-1536.0, -4117.4287109375, 2.5},
	{-2304.0, -4117.4287109375, 2.5},
	{2688.0, -3452.322021484375, 2.5},
	{1920.0, -3452.322021484375, 2.5},
	{1152.0, -3452.322021484375, 2.5},
	{384.0, -3452.322021484375, 2.5},
	{-384.0, -3452.322021484375, 2.5},
	{-1152.0, -3452.322021484375, 2.5},
	{-1920.0, -3452.322021484375, 2.5},
	{-2688.0, -3452.322021484375, 2.5},
	{3072.0, -2788.428955078125, 2.5},
	{2304.0, -2788.428955078125, 2.5},
	{1536.0, -2788.428955078125, 2.5},
	{768.0, -2788.428955078125, 2.5},
	{-0.0, -2788.428955078125, 2.5},
	{-768.0, -2788.428955078125, 2.5},
	{-1536.0, -2788.428955078125, 2.5},
	{-2304.0, -2788.428955078125, 2.5},
	{-3072.0, -2788.428955078125, 2.5},
	{3456.0, -2123.322021484375, 2.5},
	{2688.0, -2123.322021484375, 2.5},
	{1920.0, -2123.322021484375, 2.5},
	{1152.0, -2123.322021484375, 2.5},
	{384.0, -2123.322021484375, 2.5},
	{-384.0, -2123.322021484375, 2.5},
	{-1152.0, -2123.322021484375, 2.5},
	{-1920.0, -2123.322021484375, 2.5},
	{-2688.0, -2123.322021484375, 2.5},
	{-3456.0, -2123.322021484375, 2.5},
	{3840.0, -1458.21484375, 2.5},
	{3072.0, -1458.21484375, 2.5},
	{2304.0, -1458.21484375, 2.5},
	{1536.0, -1458.21484375, 2.5},
	{768.0, -1458.21484375, 2.5},
	{-0.0, -1458.21484375, 2.5},
	{-768.0, -1458.21484375, 2.5},
	{-1536.0, -1458.21484375, 2.5},
	{-2304.0, -1458.21484375, 2.5},
	{-3072.0, -1458.21484375, 2.5},
	{-3840.0, -1458.21484375, 2.5},
	{4224.0, -793.1079711914062, 2.5},
	{3456.0, -793.1079711914062, 2.5},
	{2688.0, -793.1079711914062, 2.5},
	{1920.0, -793.1079711914062, 2.5},
	{1152.0, -793.1079711914062, 2.5},
	{384.0, -793.1079711914062, 2.5},
	{-384.0, -793.1079711914062, 2.5},
	{-1152.0, -793.1079711914062, 2.5},
	{-1920.0, -793.1079711914062, 2.5},
	{-2688.0, -793.1079711914062, 2.5},
	{-3456.0, -793.1079711914062, 2.5},
	{-4224.0, -793.1079711914062, 2.5},
	{4608.0, -127.99998474121094, 2.5},
	{3840.0, -127.99998474121094, 2.5},
	{3072.0, -127.99998474121094, 2.5},
	{2304.0, -127.99998474121094, 2.5},
	{1536.0, -127.99998474121094, 2.5},
	{768.0, -127.99998474121094, 2.5},
	{-0.0, -127.99998474121094, 2.5},
	{-768.0, -127.99998474121094, 2.5},
	{-1536.0, -127.99998474121094, 2.5},
	{-2304.0, -128.0, 2.5},
	{-3072.0, -127.99998474121094, 2.5},
	{-3840.0, -127.99998474121094, 2.5},
	{-4608.0, -127.99998474121094, 2.5}
};

// This is the selected tiles used to calculate stuff
bool is_on_blue_team;
std::vector<Vector> team_tiles; // The tiles to use depending on what team you are on
std::vector<int> tiles_to_avoid; // depends on selected team
int32_t ball_state = 0; // the balls current state
Vector ballLocation;
/**
* Checks if the ball is inside the hexagon using ray-casting algorithm
* @param point - ball position
* @param hexagon_corners - the cornors for the hexagon
* @return boolean
*/
bool inside(Vector point, std::vector<Vector> hexagon_corners[6]) {
	float ball_x = point.X;
	float ball_y = point.Y;

	bool inside = false;
	for (int i = 0; i < 6; i++) {
		float x1 = hexagon_corners[i][0].X;
		float y1 = hexagon_corners[i][0].Y;
		float x2 = hexagon_corners[i][1].X;
		float y2 = hexagon_corners[i][1].Y;
		bool intersect = ((y1 >= ball_y) != (y2 >= ball_y)) && (ball_x <= (x2 - x1)* (ball_y - y1) / (y2 - y1) + x1);
		if (intersect) inside = !inside;
	}
	return inside;
};
/**
* Your standard distance calculation
* @param p1 - first positon
* @param p2 - second position
* @return distance as a double
*/
double distance(const Vector2F& p1, const Vector2F& p2){
	const double x_diff = static_cast<double>(p1.X) - static_cast<double>(p2.X);
	const double y_diff = static_cast<double>(p1.Y) - static_cast<double>(p2.Y);
	return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}
/**
* Get the neighboring tiles depending on the balls state,
* @param center_position - Postion of the tile
* @return A vector list of id's for each tile.
*/
std::vector<int> get_neighbors(Vector2F center_position) {
	std::vector<int> v;
	if (ball_state == 1) { // Ball state charged
		int r = 1000;
		for (int i = 0; i < all_hexagons.size(); i++) {
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	else if (ball_state == 2) { // Ball state super charged
		int r = 1650;
		for (int i = 0; i < all_hexagons.size(); i++) {
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	return v;
}
/**
* Get the neighboring tiles depending on 'ball_state',
* @param center_position - Postion of the tile
* @param ball_state the pre set state of the ball for simulating ahead.
* @return A vector list of id's for each tile.
*/
std::vector<int> get_neighbors(Vector2F center_position, int ball_state) {
	std::vector<int> v;
	if (ball_state == 1) {
		int r = 1000;
		for (int i = 0; i < all_hexagons.size(); i++) {
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	else if (ball_state == 2) {
		int r = 1650;
		for (int i = 0; i < all_hexagons.size(); i++) {
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	return v;
}
/**
* This only runs when damage event has been called, we get the balls position and check if its within 
* the shape of the hexagon.
* @param ball - Postion of the ball
*/
void find_updated_tile(Vector ball) {
	for (int i = 0; i < all_hexagons.size(); i++) {
		Hexagon* h = &all_hexagons[i];

		Vector point1 = { h->center_position.X, h->center_position.Y + 443.41f, 0.0f };
		Vector point2 = { h->center_position.X + 384.0f, h->center_position.Y + 221.7f, 0.0f };
		Vector point3 = { h->center_position.X + 384.0f, h->center_position.Y - 221.7f, 0.0f };
		Vector point4 = { h->center_position.X, h->center_position.Y - 443.41f, 0.0f };
		Vector point5 = { h->center_position.X - 384.0f, h->center_position.Y - 221.7f, 0.0f };
		Vector point6 = { h->center_position.X - 384.0f, h->center_position.Y + 221.7f, 0.0f };

		std::vector<Vector> hexagon[6] = {
			{point1, point2} ,
			{point2, point3},
			{point3, point4},
			{point4, point5},
			{point5, point6},
			{point6, point1}
		};
		if (inside(ball, hexagon)) {
			if (ball_state == 0) {
				if (h->state <= 2)
					h->state++;
			} else {
				std::vector<int> neighbor = get_neighbors(h->center_position);
				for (int i = 0; i < neighbor.size(); i++) {
					Hexagon* hexagon = &all_hexagons[neighbor[i]];
					if (h->state <= 2)
						hexagon->state++;
				}
			}
		}
	}
}
/**
* Calculates the best shot based on a number of factors.
* @return A vector list of id's for each tile.
*/
std::vector<int>find_best_shot() {
	float ratio = 0.0f;
	float normal_counter = 0;
	float damaged_counter = 0;
	float opened_counter = 0;
	std::vector<int> tiles_with_most_damaged_neighbors;
	for (int i = 0; i < all_hexagons.size(); i++) {
		std::vector<int> n;
		Hexagon h = all_hexagons[i];
		if (ball_state == 0)
			n = get_neighbors(h.center_position, 1); // We need to act as the ball is charged with the 1
		else
			n = get_neighbors(h.center_position);
		normal_counter = 0;
		damaged_counter = 0;
		opened_counter = 0;
		for (int j = 0; j < n.size(); j++) {
			if (all_hexagons[n[j]].state == 0)
				normal_counter++;
			else if (all_hexagons[n[j]].state == 1)
				damaged_counter++;
			else if (all_hexagons[n[j]].state == 2)
				opened_counter++;
		}
		if (normal_counter == 0)
			normal_counter = 1.0f;
		ratio = damaged_counter / normal_counter;
		if (!(std::find(tiles_to_avoid.begin(), tiles_to_avoid.end(), h.id) != tiles_to_avoid.end()) && h.state != 2) {
			if (ball_state != 0) {
				if ((ratio >= 1.2f && ball_state == 2) || (ratio >= 4.0f && ball_state == 1) && h.state != 2)
					tiles_with_most_damaged_neighbors.push_back(h.id);
			} else {
				if (ratio >= 0.34f && ratio < 1.4f && h.state != 1 && h.state != 2)
					tiles_with_most_damaged_neighbors.push_back(h.id);
			}
		}
	}
	return tiles_with_most_damaged_neighbors;
}
/**
* Find all tiles that have a state of 2, or are open nets.
* @return A vector list of id's for each tile.
*/
std::vector<int>find_open_nets() {
	float opened_counter = 0;
	std::vector<int> tiles_with_most_opened_neighbors;
	for (int i = 0; i < all_hexagons.size(); i++)
	{
		std::vector<int> n;
		Hexagon h = all_hexagons[i];
		if (all_hexagons[i].state >= 2) // >= 2 because of reasons..
			tiles_with_most_opened_neighbors.push_back(h.id);
	}
	return tiles_with_most_opened_neighbors;
}
/**
* Resets all the global variables, i don't know if you should change this but this works, and it took long enough to make it work properly
*/
void reset_variables() {
	team_tiles.clear();
	tiles_to_avoid.clear();
	all_hexagons.clear();
	if (is_on_blue_team) {
		static const int arr[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,24,25,35,36,45,46,54,55,62,63,64,65,66,67,68,69 };
		for (int i = 0; i < 30; i++)
			tiles_to_avoid.push_back(arr[i]);
		for (int i = 0; i < 70; i++)
			team_tiles.push_back(orange_tiles[i]);
	}
	else {
		static const int arr[] = { 0,1,2,3,4,5,6,7,14,15,23,24,33,34,44,45,56,57,58,59,60,61,62,63,64,65,66,67,68,69 };
		for (int i = 0; i < 30; i++)
			tiles_to_avoid.push_back(arr[i]);
		for (int i = 0; i < 70; i++)
			team_tiles.push_back(blue_tiles[i]);
	}
	for (int i = 0; i < 70; i++) {
		Hexagon h;
		h.id = 0;
		h.state = 0;
		h.center_position = { team_tiles[i].X, team_tiles[i].Y };
		all_hexagons.push_back(h);
	}	
	for (int i = 0; i < 70; i++) {
		all_hexagons[i].id = i;
		all_hexagons[i].state = 0;
		all_hexagons[i].center_position = { team_tiles[i].X, team_tiles[i].Y };
	}
	LOG("Variables reset");
}
void DropShotCalculateShot::onLoad()
{
	_globalCvarManager = cvarManager;
	cvarManager->log("Plugin loaded!");

	gameWrapper->RegisterDrawable(bind(&DropShotCalculateShot::Render, this, std::placeholders::_1));
	reset_variables();

	//Function TAGame.Ball_Breakout_TA.EventAppliedDamage
	struct EventAppliedDamageParams {
		uintptr_t Ball; // special breakout ball, castable to BallWrapper
		uint8_t Id; // maybe tile ids exist???
		Vector Location;
		int8_t DamageIndex;
		int8_t TotalDamage;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.EventAppliedDamage",
		[this](BallWrapper caller, void* params, std::string eventname) {
			EventAppliedDamageParams* param = (EventAppliedDamageParams*)params;
			//if the other team deals damage on your side very close to the middle of the field, it 'might' cause issues, not tested, but was a concern.
			if (is_on_blue_team && caller.GetLocation().Y > 0)
				find_updated_tile(caller.GetLocation());
			else if (!is_on_blue_team && caller.GetLocation().Y < 0)
				find_updated_tile(caller.GetLocation());
		}); 

	//Function TAGame.Ball_Breakout_TA.SetDamageIndex
	struct SetDamageIndexParams {
		int32_t InIndex;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.SetDamageIndex",
		[this](BallWrapper caller, void* params, std::string eventname) {
			SetDamageIndexParams* param = (SetDamageIndexParams*)params;
			int newIndex = param->InIndex;
			if (newIndex == 0) {
				gameWrapper->SetTimeout([this, newIndex](GameWrapper* gw) {
					ball_state = newIndex;
					}, 0.15f);
			}
			else {
				ball_state = newIndex;
			}
		});
	
	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function Engine.PlayerReplicationInfo.OnTeamChanged",
		[this](BallWrapper caller, void* params, std::string eventname) {

			ServerWrapper sw = gameWrapper->GetCurrentGameState();
			if (!sw) return;

			auto primary = sw.GetLocalPrimaryPlayer();

			if (primary.IsNull()) return;

			auto primaryPri = primary.GetPRI();

			if (primaryPri.IsNull()) return;

			int teamNumber = primaryPri.GetTeamNum2();

			if (teamNumber == 0 && !is_on_blue_team) {
				LOG("Changed team to blue team");
				is_on_blue_team = true;
				reset_variables();
			}
			else if (teamNumber == 1 && is_on_blue_team) {
				LOG("Changed team to orange team");
				is_on_blue_team = false;
				reset_variables();
			} else {
				LOG("Still on the same team. is on blue team: {}", is_on_blue_team);
			}
		});

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.GameEvent_Soccar_TA.EventGoalScored",
		[this] (BallWrapper caller, void* params, std::string eventname) {
			LOG("X: {} Y: {}", ballLocation.X, ballLocation.Y);
			if (ballLocation.Y > 0 && is_on_blue_team) {
				LOG("BLUE SCORED");
				gameWrapper->SetTimeout([this] (GameWrapper* gw) {
					reset_variables();
				}, 0.15f);
			}
			else if (ballLocation.Y < 0 && !is_on_blue_team) {
				LOG("ORANGE SCORED");
				gameWrapper->SetTimeout([this] (GameWrapper* gw) {
					reset_variables();
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
void DropShotCalculateShot::onUnload()
{
}


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
	RT::Frustum frust{ canvas, camera };
	if (ball.GetLocation().X != 0.0 && ball.GetLocation().Y != 0.0 && ball.GetLocation().Z >= 0)
		ballLocation = { ball.GetLocation().X, ball.GetLocation().Y, ball.GetLocation().Z };
	std::vector<int> best_shot_tiles = find_best_shot();
	LOG("SIZE: {}", best_shot_tiles.size());
	for (int i = 0; i < best_shot_tiles.size(); i++) {
		Hexagon h = all_hexagons[best_shot_tiles[i]];
		if (h.state != 2) {
			Vector point1 = { h.center_position.X, h.center_position.Y + 443.41f, 0.0f };
			Vector point2 = { h.center_position.X + 384.0f, h.center_position.Y + 221.7f, 0.0f };
			Vector point3 = { h.center_position.X + 384.0f, h.center_position.Y - 221.7f, 0.0f };
			Vector point4 = { h.center_position.X, h.center_position.Y - 443.41f, 0.0f };
			Vector point5 = { h.center_position.X - 384.0f, h.center_position.Y - 221.7f, 0.0f };
			Vector point6 = { h.center_position.X - 384.0f, h.center_position.Y + 221.7f, 0.0f };

			std::vector<Vector> hexagon[6] = {
				{point1, point2} ,
				{point2, point3},
				{point3, point4},
				{point4, point5},
				{point5, point6},
				{point6, point1}
			};
			canvas.SetColor(0, 208, 0, 130);
			for (int j = 0; j < 6; j++)
				RT::Line(hexagon[j][0], hexagon[j][1], 3.0f).DrawWithinFrustum(canvas, frust);
		}
	}
	std::vector<int> open_nets = find_open_nets();
	for (int i = 0; i < open_nets.size(); i++) {
		Hexagon h = all_hexagons[open_nets[i]];
		Vector point1 = { h.center_position.X, h.center_position.Y + 443.41f, 0.0f };
		Vector point2 = { h.center_position.X + 384.0f, h.center_position.Y + 221.7f, 0.0f };
		Vector point3 = { h.center_position.X + 384.0f, h.center_position.Y - 221.7f, 0.0f };
		Vector point4 = { h.center_position.X, h.center_position.Y - 443.41f, 0.0f };
		Vector point5 = { h.center_position.X - 384.0f, h.center_position.Y - 221.7f, 0.0f };
		Vector point6 = { h.center_position.X - 384.0f, h.center_position.Y + 221.7f, 0.0f };

		std::vector<Vector> hexagon[6] = {
			{point1, point2} ,
			{point2, point3},
			{point3, point4},
			{point4, point5},
			{point5, point6},
			{point6, point1}
		};
		canvas.SetColor(team.GetPrimaryColor().R*255, team.GetPrimaryColor().G * 255, team.GetPrimaryColor().B * 255, 150);
		for (int j = 0; j < 6; j++)
			RT::Line(hexagon[j][0], hexagon[j][1], 3.0f).DrawWithinFrustum(canvas, frust);
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