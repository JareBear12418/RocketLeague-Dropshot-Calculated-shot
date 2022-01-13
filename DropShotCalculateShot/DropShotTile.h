#pragma once

struct DropShotTile {
	int id; // the tile id
	int state; // the tiles state from 0-2, 0 being normal, 1 being damaged, 2 being opened
	Vector2F center_position; // The center position of the tile.
};


std::vector<Vector> GetOrangeTiles();
std::vector<Vector> GetBlueTiles();