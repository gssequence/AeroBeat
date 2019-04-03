#pragma once

#include "MapConfig.h"
#include "Skin.h"

class SkinManager
{
private:
	MapConfig _config;
	std::unordered_map<std::wstring, Skin> _skins;

public:
	SkinManager() { }
	virtual ~SkinManager() { }

	void initialize(std::vector<std::wstring>& paths, std::function<void(std::wstring)> logger);
	Skin& selectedSkin(Skin::Type type);
};
