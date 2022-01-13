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
	int state; // the tiles state from 1-3
	Vector2F center_position;
};
std::vector<Hexagon> all_hexagons;

bool is_on_blue_team;
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
std::vector<Vector> team_tiles;
std::vector<int> tiles_to_avoid; //depends on selected team
int32_t ball_state = 0;
//std::shared_ptr<ImageWrapper> hexagon_image;
bool inside(Vector point, std::vector<Vector> hexagon_corners[6]) {
	// ray-casting algorithm based on
	// https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html/pnpoly.html

	float x = point.X;
	float y = point.Y;

	bool inside = false;
	for (int i = 0; i < 6; i++) {
		float xi = hexagon_corners[i][0].X;
		float yi = hexagon_corners[i][0].Y;
		float xj = hexagon_corners[i][1].X;
		float yj = hexagon_corners[i][1].Y;
		bool intersect = ((yi >= y) != (yj >= y)) && (x <= (xj - xi)* (y - yi) / (yj - yi) + xi);
		if (intersect) inside = !inside;
	}

	return inside;
};
double distance(const Vector2F& p1, const Vector2F& p2)
{
	const double x_diff = static_cast<double>(p1.X) - static_cast<double>(p2.X);
	const double y_diff = static_cast<double>(p1.Y) - static_cast<double>(p2.Y);
	return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}
std::vector<int> get_neighbors(Vector2F center_position) {
	std::vector<int> v;
	if (ball_state == 1) {
		int r = 1000;
		for (int i = 0; i < all_hexagons.size(); i++)
		{
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	else if (ball_state == 2) {
		int r = 1650;
		for (int i = 0; i < all_hexagons.size(); i++)
		{
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	return v;
}
std::vector<int> get_neighbors(Vector2F center_position, int ball_state) {
	std::vector<int> v;
	if (ball_state == 1) {
		int r = 1000;
		for (int i = 0; i < all_hexagons.size(); i++)
		{
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	else if (ball_state == 2) {
		int r = 1650;
		for (int i = 0; i < all_hexagons.size(); i++)
		{
			Hexagon h = all_hexagons[i];
			if (distance(center_position, h.center_position) < r) {
				v.push_back(h.id);
			}
		}
	}
	return v;
}
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
			}
			else {
				std::vector<int> neighbor = get_neighbors(h->center_position);
				for (int i = 0; i < neighbor.size(); i++)
				{
					Hexagon* hexagon = &all_hexagons[neighbor[i]];
					if (h->state <= 2)
						hexagon->state++;
				}
			}
		}
	}
}
std::vector<int>find_best_shot() {
	float ratio = 0.0f;
	float normal_counter = 0;
	float damaged_counter = 0;
	float opened_counter = 0;
	std::vector<int> tiles_with_most_damaged_neighbors;
	for (int i = 0; i < all_hexagons.size(); i++)
	{
		std::vector<int> n;
		Hexagon h = all_hexagons[i];
		if (ball_state == 0)
			n = get_neighbors(h.center_position, 1); // We need to act as the ball is charged with the 1
		else
			n = get_neighbors(h.center_position);
		normal_counter = 0;
		damaged_counter = 0;
		opened_counter = 0;
		for (int j = 0; j < n.size(); j++)
		{
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
			}
			else {
				if (ratio >= 0.34f && ratio < 1.4f && h.state != 1 && h.state != 2)
					tiles_with_most_damaged_neighbors.push_back(h.id);
			}
		}
	}
	return tiles_with_most_damaged_neighbors;
}
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
		h.center_position = { 0, 0 };
		all_hexagons.push_back(h);
	}	
	for (int i = 0; i < 70; i++) {
		all_hexagons[i].id = i;
		all_hexagons[i].state = 0;
		all_hexagons[i].center_position = { team_tiles[i].X, team_tiles[i].Y };
	}
}
void DropShotCalculateShot::onLoad()
{
	_globalCvarManager = cvarManager;
	cvarManager->log("Plugin loaded!");

	gameWrapper->RegisterDrawable(bind(&DropShotCalculateShot::Render, this, std::placeholders::_1));
	reset_variables();
	// This gives me the balls current state
	// Function TAGame.Ball_Breakout_TA.SetDamageIndex

	/*hexagon_image = std::make_shared<ImageWrapper>(gameWrapper->GetDataFolder() / "DropShotCalculateShot" / "image.png", true, true);
	hexagon_image->LoadForCanvas();*/



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
			LOG("EVENT APPLIED DAMAGE! Ball state when damage done: {}", ball_state);
			find_updated_tile(caller.GetLocation());
		}); 
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

				if (teamNumber == 0)
					is_on_blue_team = true;
				else
					is_on_blue_team = false;
				reset_variables();
			});

	//struct GetDamageIndexForForceParams {
	//	float damageIndex; //??
	//};
	//gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.GetDamageIndexForForce",
	//	[this](BallWrapper caller, void* params, std::string eventname) {
	//		GetDamageIndexForForceParams* param = (GetDamageIndexForForceParams*)params;
	//		LOG("GetDamageIndexForForce: {}", param->damageIndex);
	//		if (param->damageIndex > 2500)
	//			ball_state = 1;
	//		else if (param->damageIndex > 11000)
	//			ball_state = 2;
	//		else
	//			ball_state = 0;
	//	});
	//Function TAGame.Ball_Breakout_TA.EventDamageIndexChanged
	//struct EventDamageIndexChangedParams {
	//	uintptr_t Ball; // special breakout ball, castable to BallWrapper
	//	int32_t InDamageIndex; // up to you to figure out the encoding for this. Probably 0-2 or 1-3
	//};
	//gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.EventDamageIndexChanged",
	//	[this](BallWrapper caller, void* params, std::string eventname) {
	//		EventDamageIndexChangedParams* param = (EventDamageIndexChangedParams*)params;
	//		LOG("EVENT DAMAGE INDEX CHANGED! InDamageIndex: {}", param->InDamageIndex);
	//	});

	/*struct RigidBodyCollisionDataParams
	{
		uintptr_t Actor;
		uintptr_t Component;
		uintptr_t PhysMat;
		Vector Velocity;
		Vector OtherVelocity;
		Vector Location;
		Vector Normal;
		Vector NormalForce;
		Vector FrictionForce;
		Vector NormalVelocity;
		Vector FrictionVelocity;
		int32_t NumCollisions;
		int32_t NumContacts;
	};
	gameWrapper->HookEventWithCallerPost<BallWrapper>("Function TAGame.Ball_Breakout_TA.OnRigidBodyCollision",
		[this](BallWrapper caller, void* params, std::string eventname) {
			RigidBodyCollisionDataParams* param = (RigidBodyCollisionDataParams*)params;
			LOG("RigidBodyCollisionDataParams: {}", param->NumContacts);
		});*/
	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	cvarManager->log("Hello notifier!");
	//	}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	cvarManager->log("the cvar with name: " + cvarName + " changed");
	//	cvarManager->log("the new value is:" + newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&DropShotCalculated::YourPluginMethod, this, _1, _2));

	 //enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&DropShotCalculated::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
		cvarManager->log("Your hook got called and the ball went POOF Variables should reset");
			reset_variables();
		});
	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", [this](std::string eventName) {
		cvarManager->log("Timer started Variables should reset");
		reset_variables();
		});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&DropShotCalculated::YourPluginMethod, this);
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
	Vector2F ballLocation = { ball.GetLocation().X, ball.GetLocation().Y };
	std::vector<int> best_shot_tiles = find_best_shot();
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
	

	if (frust.IsInFrustum(hexagon[0][0])) {
		RT::Triangle::Triangle(hexagon[0][0], hexagon[0][1], hexagon[2][0]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[2][0], hexagon[2][1], hexagon[5][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[5][1], hexagon[4][1], hexagon[2][1]).Draw(canvas);
		RT::Triangle::Triangle(hexagon[3][0], hexagon[4][0], hexagon[4][1]).Draw(canvas);
	}*/
}