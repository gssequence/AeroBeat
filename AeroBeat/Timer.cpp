#include "stdafx.h"
#include "Timer.h"

double Timer::operator()()
{
	using namespace std::chrono;
	if (_isEnabled)
	{
		if (_custom)
			return (_before = _custom_time);
		else
			return (_before = duration_cast<microseconds>(steady_clock::now() - _startTime).count() / 1000000.);
	}
	else
		return (_before = 0);
}

void Timer::start()
{
	if (!_isEnabled)
	{
		_startTime = std::chrono::steady_clock::now();
		_isEnabled = true;
	}
}

void Timer::restart()
{
	stop();
	_before = 0;
	start();
}

void Timer::stop()
{
	_isEnabled = false;
}

void Timer::set_custom_time(double value)
{
	_custom_time = value;
}
