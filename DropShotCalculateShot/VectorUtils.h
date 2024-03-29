﻿#pragma once

#include "pch.h"

namespace VectorUtils {
	double DistanceTo(const Vector2F& p1, const Vector2F& p2);
	double DistanceTo(const Vector& p1, const Vector& p2);
	Vector RotateLine(Vector2F center, float radius, float angle);
	bool IsPointInsidePolygon(const Vector& point, const std::vector<std::pair<Vector, Vector>>& hexagonCorners);
}