#ifndef _TYPEGAME_TG_MATH_H_
#define _TYPEGAME_TG_MATH_H_

#include <QtGlobal>
#include <cmath>

namespace tg
{
inline qreal clamp(qreal v, qreal lo, qreal hi)
{
	if (v < lo)
		return lo;
	if (v > hi)
		return hi;
	return v;
}

inline qreal length(qreal x, qreal y)
{
	return std::sqrt(x * x + y * y);
}
}

#endif
