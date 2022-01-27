#include "pch.h"
#include "VectorUtilities.h"

/**
 * .Distance from p1 to p2 with Vector2F
 *
 * \param p1
 * \param p2
 * \return double distance
 */
double DistanceTo(const Vector2F& p1, const Vector2F& p2) {
	const double x_diff = static_cast<double>(p1.X) - static_cast<double>(p2.X);
	const double y_diff = static_cast<double>(p1.Y) - static_cast<double>(p2.Y);
	return std::sqrt(x_diff * x_diff + y_diff * y_diff);
}

/**
 * .Distance from p1 to p2 with Vector
 *
 * \param p1
 * \param p2
 * \return double distance
 */
double DistanceTo(const Vector& p1, const Vector& p2) {
	const double x_diff = static_cast<double>(p1.X) - static_cast<double>(p2.X);
	const double y_diff = static_cast<double>(p1.Y) - static_cast<double>(p2.Y);
	const double z_diff = static_cast<double>(p1.Z) - static_cast<double>(p2.Z);
	return std::sqrt(x_diff * x_diff + y_diff * y_diff + z_diff * z_diff);
}
/**
 * .Rotate a line (coordinate) a certain angle with a given radius.
 *
 * \param center
 * \param radius
 * \param angle
 * \return
 */
Vector RotateLine(Vector2F center, float radius, float angle) {
	float newX = center.X + radius * cos(angle);
	float newY = center.Y + radius * sin(angle);
	return Vector{ newX, newY, 0 };
}

/**
 * .Is any vector inside a hexgonal shape
 *
 * \param point
 * \param hexagon_corners
 * \return bool
 */
bool IsPointInsidePolygon(const Vector& point, const std::vector<std::vector<Vector>> hexagon_corners) {
	float ball_x = point.X;
	float ball_y = point.Y;
	bool inside = false;

	for (int i = 0; i < 6; i++) {
		float x1 = hexagon_corners[i][0].X;
		float y1 = hexagon_corners[i][0].Y;
		float x2 = hexagon_corners[i][1].X;
		float y2 = hexagon_corners[i][1].Y;
		bool intersect = ((y1 >= ball_y) != (y2 >= ball_y)) && (ball_x <= (x2 - x1) * (ball_y - y1) / (y2 - y1) + x1);
		if (intersect) {
			inside = !inside;
		}
	}
	return inside;
}
