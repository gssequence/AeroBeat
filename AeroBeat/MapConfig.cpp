#include "stdafx.h"
#include "MapConfig.h"

bool MapConfig::load(std::wstring path, MapConfig& config)
{
	std::wifstream ifs(path, std::ios::in);
	if (ifs.fail()) return false;
	boost::property_tree::wptree tree;
	boost::property_tree::json_parser::read_json(ifs, tree);
	for (auto a : tree)
	{
		auto l = a.first;
		auto r = a.second.get_value_optional<std::wstring>();
		if (r)
			config._elements[l] = r.value();
	}
	return true;
}

bool MapConfig::save(std::wstring path)
{
	boost::property_tree::wptree tree;
	for (auto a : _elements)
	{
		tree.put(boost::property_tree::wpath(a.first, L'\0'), a.second);
	}
	std::wofstream ofs(path, std::ios::out);
	if (ofs.fail()) return false;
	boost::property_tree::json_parser::write_json(ofs, tree);
	return true;
}
