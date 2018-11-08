#include "bwPoint.h"

using namespace bWidgets;


bwPoint::bwPoint(float x, float y) :
    x(x), y(y)
{
	
}

bool bwPoint::operator==(const bwPoint& rhs) const
{
	return (x == rhs.x) && (y == rhs.y);
}

namespace bWidgets {

bwPoint operator*(const bwPoint& lhs, float factor)
{
	return {lhs.x * factor, lhs.y * factor};
}
bwPoint operator*(float factor, const bwPoint& rhs)
{
	return {rhs.x * factor, rhs.y * factor};
}
bwPoint operator+(const bwPoint& lhs, const bwPoint& rhs)
{
	return {lhs.x + rhs.x, lhs.y + rhs.y};
}
bwPoint operator-(const bwPoint& lhs, const bwPoint& rhs)
{
	return {lhs.x - rhs.x, lhs.y - rhs.y};
}

} // namespace bWidgets
