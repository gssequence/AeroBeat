#pragma once

class Timer
{
private:
	bool _custom = false;
	bool _isEnabled = false;
	double _before = 0;
	std::chrono::time_point<std::chrono::steady_clock> _startTime;
	double _custom_time = 0;

public:
	Timer(bool custom = false) : _custom(custom) { }
	virtual ~Timer() { }

	double operator()();

	void start();
	void restart();
	void stop();
	void set_custom_time(double value);

	bool isEnabled() { return _isEnabled; }
	auto before() { return _before; }
};
