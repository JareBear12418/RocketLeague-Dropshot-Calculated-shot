#include "pch.h"
#define LINMATH_H //Conflicts with linmath.h if we done declare this here
#include "DropShotCalculateShot.h"
#include "DropShotTile.h"
#include "VectorUtilities.h"
#include "RenderingTools/Objects/Frustum.h"
#include "RenderingTools/Objects/Line.h"
#include <algorithm> // used for avoiding certain tiles
#include <sstream> // strings?
#include <chrono> // date stuff
#include <math.h> // math stuff

BAKKESMOD_PLUGIN(DropShotCalculateShot, "Calculates the best tile to shoot at in dropshot.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

// these stay here unless someone wants to move them
auto ballHitTime = std::chrono::high_resolution_clock::now();
auto ballDidDamageTime = std::chrono::high_resolution_clock::now();
auto lastDistanceCheckTime = std::chrono::high_resolution_clock::now();
auto currentTime = std::chrono::high_resolution_clock::now();

/**
* Get the neighboring tiles depending on the balls state,
* @param tile - Postion of the tile
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::GetTileNeighbours(const DropShotTile& tile) const {
	std::vector<int> neighbours;
	int search_radius;
	if (ball_state == 1){
		search_radius = 1000;
	} else if (ball_state == 2) {
		search_radius = 1650;
	} else {
		return neighbours;
	}
	for (auto& h : all_tiles) {
		if (DistanceTo(tile.center_position, h.center_position) < search_radius) {
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
std::vector<int> DropShotCalculateShot::GetTileNeighbours(const DropShotTile& tile, int ball_state) {
	std::vector<int> neighbours;
	int search_radius;
	if (ball_state == 1) {
		search_radius = 1000;
	} else if (ball_state == 2) {
		search_radius = 1650;
	} else {
		return neighbours;
	}
	for (auto& h : all_tiles) {
		if (DistanceTo(tile.center_position, h.center_position) < search_radius) {
			neighbours.push_back(h.id);
		}
	}
	return neighbours;
}

/**
 * .Returns a tile from a vector position
 *
 * \param Vector position
 * \return tile
 */
DropShotTile DropShotCalculateShot::FindTileFromPostion(const Vector& position) {
	for (auto& tile : all_tiles) {
		std::vector<std::vector<Vector>> tile_corners = GetHexagonCornors(Vector{ tile.center_position.X, tile.center_position.Y, 0 });
		if (IsPointInsidePolygon(position, tile_corners)) {
			return tile;
		}
	}
}

/**
 * .Checks if there is a tile given a vector location.
 *
 * \param Vector position
 * \return true if a tile exists there, else false
 */
bool DropShotCalculateShot::DoesTileExist(const Vector& position) {
	for (auto& tile : all_tiles) {
		std::vector<std::vector<Vector>> tile_corners = GetHexagonCornors(Vector{ tile.center_position.X, tile.center_position.Y, 0 });
		if (IsPointInsidePolygon(position, tile_corners)) {
			return true;
		}
	}
	return false;
}

/**
* Calculates the best shot based on a number of factors. This is the brain so to speak.
* We use simple statistics to determine which shot is better. If there are more damaged tiles around a tile depending what state the ball is at,
* then that is a good shot, again depending on what state the ball is at.
* @return A vector list of id's for each tile.
*/
std::vector<int> DropShotCalculateShot::FindBestShot() {
	std::vector<int> tiles_with_most_damaged_neighbors;
	for (auto& tile : all_tiles) {
		// We do this check because any outside tiles if the ball is not charged are a wasted shot
		if ((std::find(tiles_to_avoid.begin(), tiles_to_avoid.end(), tile.id) != tiles_to_avoid.end()) && ball_state == 0) {
			continue;
		}
		std::vector<int> neighbours;
		if (ball_state == 0) {
			neighbours = GetTileNeighbours(tile, 1); // We need to act as the ball is charged with the 1
		} else {
			neighbours = GetTileNeighbours(tile);
		}
		float normal_counter = 0;
		float damaged_counter = 0;
		float opened_counter = 0; // Does nothing, but in case of future.
		for (int neighbour : neighbours) {
			if (all_tiles[neighbour].state == 0) {
				normal_counter++;
			} else if (all_tiles[neighbour].state == 1){
				damaged_counter++;
			} else if (all_tiles[neighbour].state == 2){
				opened_counter++;
			}
		}
		// Zero division fool proof
		if (normal_counter == 0){
			normal_counter = 1.0f;
		}
		float ratio = damaged_counter / normal_counter;
		if (tile.state != 2) {
			if (ball_state != 0) { // Charged or Super Charged ball states
				if (ratio >= 0.6f && ball_state == 2 || ratio >= 4.0f && ball_state == 1 && tile.state != 2) {
					tiles_with_most_damaged_neighbors.push_back(tile.id);
				}
			} else { // Normal ball state
				if (ratio >= 0.34f && ratio < 1.4f && tile.state != 1 && tile.state != 2) {
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
std::vector<int> DropShotCalculateShot::FindOpenNets() const {
	std::vector<int> tiles_with_most_opened_neighbors;
	for (auto& tile : all_tiles) {
		if (tile.state == 2) {
			tiles_with_most_opened_neighbors.push_back(tile.id);
		}
	}
	return tiles_with_most_opened_neighbors;
}

/**
* @param h a dropshot tile
* @return A vector list of vectors.
*/
std::vector<std::vector<Vector>> DropShotCalculateShot::GetHexagonCornors(const Vector& center_position) {
	Vector point1 = { center_position.X + 384.0f, center_position.Y - 221.7f, 0.0f };
	Vector point2 = { center_position.X + 384.0f, center_position.Y + 221.7f, 0.0f };
	Vector point3 = { center_position.X, center_position.Y + 443.41f, 0.0f };
	Vector point4 = { center_position.X - 384.0f, center_position.Y + 221.7f, 0.0f };
	Vector point5 = { center_position.X - 384.0f, center_position.Y - 221.7f, 0.0f };
	Vector point6 = { center_position.X, center_position.Y - 443.41f, 0.0f };


	std::vector<std::vector<Vector>> corners = {
		{{point1}, {point2}},
		{{point2}, {point3}},
		{{point3}, {point4}},
		{{point4}, {point5}},
		{{point5}, {point6}},
		{{point6}, {point1}}
	};
	return corners;
}

/**
 * .Get Hexagon center lines out, No idea what to call this.. But this is responsible for 'collision' detection, or damaged neighbour checking.
 *
 * \param dropshot tile
 * \return
 */
std::vector<Vector> DropShotCalculateShot::GetHexagonConnectors(const DropShotTile& h) {
	Vector point1 = RotateLine(h.center_position, 768.0f, 0.0f*(3.141592653f/180.0f));
	Vector point2 = RotateLine(h.center_position, 768.0f, 60.0f*(3.141592653f/180.0f));
	Vector point3 = RotateLine(h.center_position, 768.0f, 120.0f*(3.141592653f/180.0f));
	Vector point4 = RotateLine(h.center_position, 768.0f, 180.0f*(3.141592653f/180.0f));
	Vector point5 = RotateLine(h.center_position, 768.0f, 240.0f*(3.141592653f/180.0f));
	Vector point6 = RotateLine(h.center_position, 768.0f, 300.0f*(3.141592653f/180.0f));
	std::vector<Vector> new_positions = {
		{point1},
		{point2},
		{point3},
		{point4},
		{point5},
		{point6}
	};
	return new_positions;
}

/**
 * .No idea what to call this.. But this is responsible for filling hexagonal shapes
 *
 * \param DropShotTile
 * \param numberOfLines
 * \return
 */
std::vector<std::vector<Vector>> DropShotCalculateShot::GetFilledHexagonCoordinates(const DropShotTile& h, const int numberOfLines) {
	float tileHeight = 886.82f;
	float triangleHeight = 221.705f;
	float width = 768.0f;
	float spacing = tileHeight / numberOfLines;
	float centerX = h.center_position.X;
	float centerY = h.center_position.Y;

	std::vector<std::vector<Vector>> points = { };

	float x1 = 0.0f;
	float x2 = 0.0f;
	float y = centerY-443.0f; // To get the top of the hexagon as our starting positon

	for (int i = 0; i < numberOfLines; i++) {
		// Making them bigger by +/- 1 just so they match together better.
		if (y < (centerY - 221.7f)) {
			x1 = centerX - (width * ((spacing * i) / triangleHeight) / 2.0f) - 1.0f;
			x2 = centerX + (width * ((spacing * i) / triangleHeight) / 2.0f) + 1.0f;
		} else if (y > (centerY - 221.7f)) {
			x1 = centerX - 385.0f;
			x2 = centerX + 385.0f;
		}
		if (y > (centerY + 221.7f)) {
			x1 = centerX - (width * ((tileHeight - spacing * i) / triangleHeight) / 2.0f) - 1.0f;
			x2 = centerX + (width * ((tileHeight - spacing * i) / triangleHeight) / 2.0f) + 1.0f;
		}
		points.push_back({Vector{ x1, y , 0}, Vector{ x2, y , 0}});
		y += spacing;
	}
	return points;
}

/**
 * .Calculates how fast the car is approaching the ball, taking into account how fast the ball is approaching the car. This is responsible for getting the coordinates to fix a hexagonal shape.
 *
 * \param car
 * \param ball
 * \return velocity
 */
float DropShotCalculateShot::CarVelocityTowardsBall(CarWrapper& car, BallWrapper& ball) {
	float carVelocityTowardsBall = Vector{ ball.GetVelocity().X - car.GetVelocity().X, ball.GetVelocity().Y - car.GetVelocity().Y, ball.GetVelocity().Z - car.GetVelocity().Z }.magnitude();
	if (distanceFromCarToBall < DistanceTo(car.GetLocation(), ball.GetLocation())) {
		carVelocityTowardsBall *= -1;
	}
	return carVelocityTowardsBall;
}

/**
* Resets all the global variables, i don't know if you should change this but this works, and it took long enough to make it work properly, This doesnt work half the time still but its cool
*/
void DropShotCalculateShot::ResetVariables() {
	ballHitTime = std::chrono::high_resolution_clock::now();
	team_tiles.clear();
	tiles_to_avoid.clear();
	all_tiles.clear();
	if (is_on_blue_team) {
		tiles_to_avoid = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 24, 25, 35, 36, 45, 46, 54, 55, 62, 63, 64, 65, 66, 67, 68, 69};
		team_tiles = GetOrangeTiles();
	} else {
		tiles_to_avoid = {0, 1, 2, 3, 4, 5, 6, 7, 14, 15, 23, 24, 33, 34, 44, 45, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69};
		team_tiles = GetBlueTiles();
	}
	for (int i = 0; i < 70; i++) {
		DropShotTile h{i, 0, {team_tiles[i].X, team_tiles[i].Y}};
		all_tiles.push_back(h);
	}
	LOG("Variables reset");
}

/**
 * .Updates all tile states thanks to the new DropShotWrapper :)
 *
 */
void DropShotCalculateShot::UpdateAllTiles() {
	static auto platforms = BreakOutActorPlatformWrapper::GetAllPlatforms();
	for (auto platform : platforms) {
		if (!platform) { return; }
		for (DropShotTile& tile : all_tiles) {
			if (DistanceTo(platform.GetLocation(), Vector{ tile.center_position.X , tile.center_position.Y, 0 }) < 100) {
				tile.state = platform.GetDamagestate().State;
			}
		}
	}
}

void DropShotCalculateShot::onLoad() {
	_globalCvarManager = cvarManager;
	cvarManager->log("Plugin loaded!");

	gameWrapper->RegisterDrawable(bind(&DropShotCalculateShot::Render, this, std::placeholders::_1));
	ResetVariables();
	UpdateAllTiles();

	//Function TAGame.Ball_Breakout_TA.GetDamageIndexForForce
	struct GetDamageIndexForForce {
		float force;
		int32_t return_value;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.GetDamageIndexForForce",
													  [this] (BallWrapper caller,
													  void* params,
													  std::string eventname) {
		GetDamageIndexForForce* param = (GetDamageIndexForForce*)params;
		ball_charge = param->force;
	});
	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_TA.OnCarTouch",
														  [this](BallWrapper caller,
														  void* params,
														  std::string eventname) {
		GetDamageIndexForForce* param = (GetDamageIndexForForce*)params;
		ballHitTime = std::chrono::high_resolution_clock::now();
	});

	//Function TAGame.Ball_Breakout_TA.EventAppliedDamage
	struct EventAppliedDamageParams {
		uintptr_t Ball; // special breakout ball, castable to BallWrapper
		uint8_t Id; // maybe tile ids exist???
		Vector Location;
		int8_t DamageIndex;
		int8_t TotalDamage;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.EventAppliedDamage",
													  [this](BallWrapper caller,
													  void* params,
													  std::string eventname) {
		EventAppliedDamageParams* param = (EventAppliedDamageParams*)params;
		UpdateAllTiles();
		ballDidDamageTime = std::chrono::high_resolution_clock::now();
	});

	//Function TAGame.Ball_Breakout_TA.SetDamageIndex
	struct SetDamageIndexParams {
		int32_t InIndex;
	};

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.SetDamageIndex",
													  [this](BallWrapper caller,
													  void* params,
													  std::string eventname) {
		SetDamageIndexParams* param = (SetDamageIndexParams*)params;
		int newIndex = param->InIndex;
		if (newIndex == 0) {
			gameWrapper->SetTimeout([this, newIndex](GameWrapper* gw) {
				ball_charge = 0.0f;
				ball_state = newIndex;
			}, 0.15f);
		} else {
			ball_state = newIndex;
		}
	});

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function Engine.PlayerReplicationInfo.OnTeamChanged",
													  [this](BallWrapper caller,
													  void* params,
													  std::string eventname) {
		ServerWrapper sw = gameWrapper->GetCurrentGameState();
		if (!sw) { return; }

		auto primary = sw.GetLocalPrimaryPlayer();

		if (primary.IsNull()) { return; }

		auto primaryPri = primary.GetPRI();

		if (primaryPri.IsNull()) { return; }

		int teamNumber = primaryPri.GetTeamNum2();

		if (teamNumber == 0 && !is_on_blue_team) {
			LOG("Changed team to blue team");
			is_on_blue_team = true;
			ResetVariables();
		} else if (teamNumber == 1 && is_on_blue_team) {
			LOG("Changed team to orange team");
			is_on_blue_team = false;
			ResetVariables();
		} else {
			LOG("Still on the same team. is on blue team: {}", is_on_blue_team);
		}
	});

	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.GameEvent_Soccar_TA.EventGoalScored",
													  [this](BallWrapper caller,
													  void* params,
													  std::string eventname) {
		LOG("X: {} Y: {}", ballLocation.X, ballLocation.Y);
		if (ballLocation.Y > 0 && is_on_blue_team) {
			LOG("BLUE SCORED");
			did_blue_score_last = true;
			ResetVariables();
		} else if (ballLocation.Y < 0 && !is_on_blue_team) {
			LOG("ORANGE SCORED");
			did_blue_score_last = false;
			ResetVariables();
		}
	});

	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState",
						   [this](std::string eventName) {
		gameWrapper->SetTimeout([this] (GameWrapper* gw) {
			if (is_on_blue_team && did_blue_score_last && ballLocation.Y > 0) {
				LOG("You are on blue team and blue scored");
				ResetVariables();
			} else if (!is_on_blue_team && !did_blue_score_last && ballLocation.Y < 0) {
				LOG("You are on orange team and orange scored");
				ResetVariables();
			}
		}, 0.15f);
	});
}

void DropShotCalculateShot::onUnload() {}

void DropShotCalculateShot::Render(CanvasWrapper canvas) {
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (gameWrapper->IsInOnlineGame()) { return; }
	if (!server) { return; }
	BallWrapper ball = server.GetBall();
	if (!ball || !ball.IsDropshotBall()) { return; }
	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) { return; }
	carVelocity = car.GetVelocity().magnitude();
	auto camera = gameWrapper->GetCamera();
	if (camera.IsNull()) { return; }
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	if (teams.IsNull()) { return; }
	int team_number = 0;
	if (is_on_blue_team) {
		team_number = 0;
	} else {
		team_number = 1;
	}
	TeamWrapper team = teams.Get(team_number);
	if (!team) { return; }
	RT::Frustum frust{canvas, camera};

	int canvas_y = 1000-(25*9);

	// Im not good with guis, so please help.
	canvas.SetPosition(Vector2{ 0, canvas_y });
	//-----------------DRAW BALL STATE-----------------
	canvas.SetColor(255, 255, 255, 255);
	if (ball_state == 0) {
		canvas.DrawString("Ball state: Normal", 2.0, 2.0);
	}else if (ball_state == 1) {
		canvas.DrawString("Ball state: Charged", 2.0, 2.0);
	} else if (ball_state == 2) {
		canvas.DrawString("Ball state: Super Charged", 2.0, 2.0);
	}
	canvas_y += 25;
	//-----------------DRAW BALLS CHARGE-----------------
	canvas.SetPosition(Vector2{ 0, canvas_y });
	canvas.DrawString("Current energy: " + std::to_string((int)ball_charge), 2.0, 2.0);

	canvas_y += 25;
	//-----------------DRAW BALL Z VELOCITY-----------------
	canvas.SetPosition(Vector2{ 0, canvas_y });
	if ( (int)ball.GetVelocity().Z >= -250) {
		canvas.SetColor(208, 0, 0, 255);
		canvas.DrawString("Ball down velocity: " + std::to_string((int)fabs(ball.GetVelocity().Z)), 2.0, 2.0);
	} else {
		canvas.SetColor(0, 208, 0, 255);
		canvas.DrawString("Ball down velocity: >250", 2.0, 2.0);
	}
	canvas_y += 25;
	//-----------------DRAW CAR VELOCITY-----------------
	currentTime = std::chrono::high_resolution_clock::now();
	if (((currentTime - lastDistanceCheckTime).count() / 1000000000.0f) > .2f) {
		distanceFromCarToBall = DistanceTo(car.GetLocation(), ball.GetLocation());
		lastDistanceCheckTime = std::chrono::high_resolution_clock::now();
	}
	float carVelocityTowardsBall = CarVelocityTowardsBall(car, ball);
	canvas.SetPosition(Vector2{ 0, canvas_y });
	if ((int)carVelocityTowardsBall > 500) {
		canvas.SetColor(0, 208, 0, 255);
	} else {
		canvas.SetColor(208, 0, 0, 255);
	}
	canvas.DrawString("Car velocity to ball: " + std::to_string((int)carVelocityTowardsBall), 2.0, 2.0);
	canvas_y += 25;

	//-----------------DRAW ACCUMULAED ABOSRBED ENERGY-----------------
	float force_accum_recent = 0.0f;
	canvas.SetPosition(Vector2{ 0, canvas_y });
	if ((2500 - (ballLastHitTime * 2500)) >= 0) {
		force_accum_recent = (2500 - (ballLastHitTime * 2500));
		canvas.DrawString("Accumulated absorbed energy: " + std::to_string((int)force_accum_recent), 2.0, 2.0);
		canvas_y += 25;
	}
	//-----------------DRAW THE CALCUALTED DAMAGE TRANSFER-----------------
	canvas.SetPosition(Vector2{ 0, canvas_y });
	int calcualtedAbsorbedForce = (int)((carVelocityTowardsBall - force_accum_recent));
	if (calcualtedAbsorbedForce >= 2500) {
		calcualtedAbsorbedForce = 2500;
	} else if (calcualtedAbsorbedForce <= 0) {
		calcualtedAbsorbedForce = 0;
	}
	if (ball_state == 0) {
		if (calcualtedAbsorbedForce >= (2500 - (int)ball_charge)) {
			canvas.SetColor(0, 208, 0, 255);
		} else {
			canvas.SetColor(255, 255, 255, 255);
		}
	} else if (ball_state == 1) {
		if (calcualtedAbsorbedForce >= (11000 - (int)ball_charge)) {
			canvas.SetColor(0, 208, 0, 255);
		} else {
			canvas.SetColor(255, 255, 255, 255);
		}
	}
	canvas.DrawString("Calculated absorbed force: " + std::to_string(calcualtedAbsorbedForce), 2.0, 2.0);

	//-----------------DRAW NEXT BALL STATE IN-----------------
	canvas_y += 25;
	canvas.SetPosition(Vector2{ 0, canvas_y });
	if (ball_state == 0) {
		canvas.DrawString("Next ball state in: " + std::to_string(2500 - (int)ball_charge) + " energy", 2.0, 2.0);
		canvas_y += 25;
	} else if (ball_state == 1) {
		canvas.DrawString("Next ball state in: " + std::to_string(11000 - (int)ball_charge) + " energy", 2.0, 2.0);
		canvas_y += 25;
	}
	//-----------------DRAW IF WE CAN DAMAGE TILES-----------------
	canvas.SetPosition(Vector2{ 0, canvas_y });
	if (((currentTime - ballDidDamageTime).count() / 1000000000.0f) > 0.2f) {
		canvas.SetColor(0, 208, 0, 255);
		canvas.DrawString("Can damage tiles", 2.0, 2.0);
	} else {
		canvas.SetColor(208, 0, 0, 255);
		canvas.DrawString("Can't damage tiles", 2.0, 2.0);
	}
	canvas_y += 25;

	//-----------------DRAW LAST BALL HIT TIME-----------------
	ballLastHitTime = ((currentTime - ballHitTime).count() / 1000000000.0f);
	canvas.SetColor(255, 255, 255, 255);
	canvas.SetPosition(Vector2{ 0, canvas_y });
	canvas.DrawString("Last ball hit: " + std::to_string(roundf(ballLastHitTime * 100) / 100).substr(0, std::to_string(roundf(ballLastHitTime * 100) / 100).size() - 4) + "/s", 2.0, 2.0);

	if (ball.GetLocation().X != 0.0 && ball.GetLocation().Y != 0.0 && ball.GetLocation().Z >= 0) {
		ballLocation = {ball.GetLocation().X, ball.GetLocation().Y, ball.GetLocation().Z};
	}
	//-----------------FIND BEST SHOT-----------------
	std::vector<int> best_shot_tiles = FindBestShot();
	for (int best_shot_tile : best_shot_tiles) {
		if (!best_shot_tile) { return; }
		const DropShotTile& h = all_tiles[best_shot_tile];
		float distanceFromPlayer = DistanceTo(car.GetLocation(), Vector{ h.center_position.X, h.center_position.Y, 0 });
		// Any more lines can cause FPS drop, so if you have beefy pc and care for some crisp hexagons, have at it.
		int numberOfLines = 100000 / (int)distanceFromPlayer;
		if (numberOfLines >= 50) {
			numberOfLines = 50;
		}
		if (h.state != 2) { // When this is false, its an opened tile
			std::vector<Vector> connectors = GetHexagonConnectors(h);
			std::vector<std::vector<Vector>> tile_corners = GetHexagonCornors(Vector{ h.center_position.X , h.center_position.Y, 0});
			std::vector<std::vector<Vector>> tile_fill_coordinates= GetFilledHexagonCoordinates(h, numberOfLines);
			canvas.SetColor(0, 208, 0, 45);
			for (int i = 0; i < tile_fill_coordinates.size(); i++) {
				RT::Line(tile_fill_coordinates[i][0], tile_fill_coordinates[i][1], 2.0f).DrawWithinFrustum(canvas, frust);
			}
			canvas.SetColor(0, 208, 0, 130);
			for (int i = 0; i < tile_corners.size(); i++) {
				if (DoesTileExist(connectors[i])) { // Very important do not remove, this is to ensure the program doesn't look for tiles that are outside of the arena. When an edge tile gets 'opened' it looks for tiles outside there.
					if (!(std::find(best_shot_tiles.begin(), best_shot_tiles.end(), FindTileFromPostion(connectors[i]).id) != best_shot_tiles.end())) {
						RT::Line(tile_corners[i][0], tile_corners[i][1], 3.0f).DrawWithinFrustum(canvas, frust);
					}
				}
			}
		}
	}
	//-----------------FIND OPEN NETS-----------------
	std::vector<int> open_nets = FindOpenNets();
	for (int open_net : open_nets) {
		const DropShotTile& h = all_tiles[open_net];
		float distanceFromPlayer = DistanceTo(car.GetLocation(), Vector{ h.center_position.X, h.center_position.Y, 0 });
		// Any more lines can cause FPS drop, so if you have beefy pc and care for some crisp hexagons, have at it.
		int numberOfLines = 100000 / (int)distanceFromPlayer;
		if (numberOfLines >= 50) {
			numberOfLines = 50;
		}
		std::vector<Vector> connectors = GetHexagonConnectors(h);
		std::vector<std::vector<Vector>> tile_corners = GetHexagonCornors(Vector{ h.center_position.X , h.center_position.Y, 0 });
		std::vector<std::vector<Vector>> tile_fill_coordinates = GetFilledHexagonCoordinates(h, numberOfLines);
		for (int i = 0; i < tile_corners.size(); i++) {
			if (DoesTileExist(connectors[i])) { // Very important do not remove, this is to ensure the program doesn't look for tiles that are outside of the arena. When an edge tile gets 'opened' it looks for tiles outside there.
				canvas.SetColor(team.GetPrimaryColor().R * 255, team.GetPrimaryColor().G * 255, team.GetPrimaryColor().B * 255, 20);
				for (int i = 0; i < tile_fill_coordinates.size(); i++) {
					RT::Line(tile_fill_coordinates[i][0], tile_fill_coordinates[i][1], 2.0f).DrawWithinFrustum(canvas, frust);
				}
				canvas.SetColor(team.GetPrimaryColor().R * 255, team.GetPrimaryColor().G * 255, team.GetPrimaryColor().B * 255, 150);
				if (!(std::find(open_nets.begin(), open_nets.end(), FindTileFromPostion(connectors[i]).id) != open_nets.end())) {
					RT::Line(tile_corners[i][0], tile_corners[i][1], 3.0f).DrawWithinFrustum(canvas, frust);
				}
			}
		}
	}
	/* This is code to draw a hexagon with triangles... its pointless but Im keeping it around..who knows if transparent triangles will come.
	if (frust.IsInFrustum(hexagon[0][0])) {
		RT::Triangle::Triangle(hexagon[0][0], hexagon[0][1], hexagon[2][0]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[2][0], hexagon[2][1], hexagon[5][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[5][1], hexagon[4][1], hexagon[2][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[3][0], hexagon[4][0], hexagon[4][1]).Draw(canvas);
	}*/
}
