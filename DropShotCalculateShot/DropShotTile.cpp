#include "pch.h"
#include "DropShotTile.h"

DropShotTile::DropShotTile(int32_t id, const Vector2F& centerPosition)
	: Id(id), State(static_cast<int32_t>(TileStates::Normal)), CenterPosition(centerPosition) {
}

DropShotTile::DropShotTile(int32_t id, int32_t state, const Vector2F& centerPosition)
	: Id(id), State(state), CenterPosition(centerPosition) {
}

DropShotTile::~DropShotTile() { }

bool DropShotTile::IsNormal() const {
	return (static_cast<TileStates>(State) == TileStates::Normal);
}

bool DropShotTile::IsDamaged() const {
	return (static_cast<TileStates>(State) == TileStates::Damaged);
}

// > 2 because of reasons... (idk ask Jare -ItsBranK)
bool DropShotTile::IsOpen() const {
	return (State > static_cast<int32_t>(TileStates::Open) || static_cast<TileStates>(State) == TileStates::Open);
}

void DropShotTile::IncreaseState() {
	if (IsNormal()) {
		State = static_cast<int32_t>(TileStates::Damaged);
	} else if (IsDamaged()) {
		State = static_cast<int32_t>(TileStates::Open);
	}
}