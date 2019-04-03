#pragma once

#include "Timer.h"

template <typename T>
class ValueAnimator
{
private:
	T start, end;
	Timer timer;
	double duration;
	ease::EaseFunc func;

public:
	ValueAnimator(T initial, double duration, ease::EaseFunc func = ease::linear) : start(initial), end(initial), duration(duration), func(func) { }
	virtual ~ValueAnimator() { }

	void set(T value)
	{
		start = operator()();
		end = value;
		timer.restart();
	}

	T operator()()
	{
		if (timer.isEnabled())
		{
			double t = timer();
			if (t > duration)
				timer.stop();
			else
				return static_cast<T>(start + (end - start) * func(t / duration));
		}
		return end;
	}
};
