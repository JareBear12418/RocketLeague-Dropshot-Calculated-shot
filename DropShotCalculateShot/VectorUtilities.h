#pragma once

/**
* Your standard distance calculation
* @param p1 - first positon
* @param p2 - second position
* @return distance as a double
*/
double DistanceTo(const Vector2F& p1, const Vector2F& p2);

bool IsPointInsidePolygon(const Vector& point, const std::vector<Vector> hexagon_corners[6]);;
