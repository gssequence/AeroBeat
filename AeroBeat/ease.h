#pragma once
#include <cmath>

#define EASE_FROM_STRING(str, func) if ((str) == L ## #func) return (func);

namespace ease
{
	typedef double(*EaseFunc)(double);

	constexpr double PI = 3.1415926535897932384626433832795;
	constexpr double PI_2 = PI / 2;
	constexpr double PI_4 = PI / 4;
	constexpr double PI3_2 = PI + PI_2;

	static double stop(double t)
	{
		return 0;
	}

	static double linear(double t)
	{
		return t;
	}

	static double inOut(double t, EaseFunc in, EaseFunc out)
	{
		return t < 0.5 ? in(t * 2) / 2 : out((t - 0.5) * 2) / 2 + 0.5;
	}

	static double easeInSine(double t)
	{
		return std::sin(t * PI_2 - PI_2) + 1.0;
	}

	static double easeOutSine(double t)
	{
		return std::sin(t * PI_2);
	}

	static double easeInOutSine(double t)
	{
		return inOut(t, easeInSine, easeOutSine);
	}

	static double easeInQuad(double t)
	{
		return t * t;
	}

	static double easeOutQuad(double t)
	{
		return -easeInQuad(t - 1) + 1;
	}

	static double easeInOutQuad(double t)
	{
		return inOut(t, easeInQuad, easeOutQuad);
	}

	static double easeInCubic(double t)
	{
		return t * t * t;
	}

	static double easeOutCubic(double t)
	{
		return easeInCubic(t - 1) + 1;
	}

	static double easeInOutCubic(double t)
	{
		return inOut(t, easeInCubic, easeOutCubic);
	}

	static double easeInQuart(double t)
	{
		return t * t * t * t;
	}

	static double easeOutQuart(double t)
	{
		return -easeInQuart(t - 1) + 1;
	}

	static double easeInOutQuart(double t)
	{
		return inOut(t, easeInQuart, easeOutQuart);
	}

	static double easeInQuint(double t)
	{
		return t * t * t * t * t;
	}

	static double easeOutQuint(double t)
	{
		return easeInQuint(t - 1) + 1;
	}

	static double easeInOutQuint(double t)
	{
		return inOut(t, easeInQuint, easeOutQuint);
	}

	static EaseFunc from_string(std::wstring str)
	{
		EASE_FROM_STRING(str, stop)
		EASE_FROM_STRING(str, linear)
		EASE_FROM_STRING(str, easeInSine)
		EASE_FROM_STRING(str, easeOutSine)
		EASE_FROM_STRING(str, easeInOutSine)
		EASE_FROM_STRING(str, easeInQuad)
		EASE_FROM_STRING(str, easeOutQuad)
		EASE_FROM_STRING(str, easeInOutQuad)
		EASE_FROM_STRING(str, easeInCubic)
		EASE_FROM_STRING(str, easeOutCubic)
		EASE_FROM_STRING(str, easeInOutCubic)
		EASE_FROM_STRING(str, easeInQuart)
		EASE_FROM_STRING(str, easeOutQuart)
		EASE_FROM_STRING(str, easeInOutQuart)
		EASE_FROM_STRING(str, easeInQuint)
		EASE_FROM_STRING(str, easeOutQuint)
		EASE_FROM_STRING(str, easeInOutQuint)
		return linear;
	}
}
