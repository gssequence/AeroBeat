#include "stdafx.h"
#include "SkinManager.h"

void SkinManager::initialize(std::vector<std::wstring>& paths, std::function<void(std::wstring)> logger)
{
	MapConfig::load(L"ABFiles/Settings/Skin.json", _config);
	for (auto& a : paths)
	{
		fs::path path(a);
		if (path.extension() == L".abskin")
		{
			Skin s;
			if (Skin::parse(path, s))
			{
				auto wpath = path.generic_wstring();
				_skins[wpath] = std::move(s);
				logger(L"Skin found: " + wpath);
			}
		}
	}
}

Skin& SkinManager::selectedSkin(Skin::Type type)
{
	Skin* first = nullptr;
	for (auto& a : _skins)
	{
		auto& skin = a.second;
		if (skin.type() == type)
		{
			if (first == nullptr) first = &skin;
			if (a.first == _config.elements()[std::to_wstring((int)type)])
				return skin;
		}
	}
	return *first;
}
