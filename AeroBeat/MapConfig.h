#pragma once

class MapConfig
{
private:
	std::unordered_map<std::wstring, std::wstring> _elements;

public:
	MapConfig() { }
	virtual ~MapConfig() { }

	static bool load(std::wstring path, MapConfig& config);
	bool save(std::wstring path);

	decltype(_elements)& elements() { return _elements; }
};
