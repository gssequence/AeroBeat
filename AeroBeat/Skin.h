#pragma once

#include "MapConfig.h"
#include "util.h"

class Skin
{
public:
	enum Type
	{
		None,
		Play7,
		Decide,
		MusicSelect,
		SkinSelect,
		KeyConfig,
		Result
	};

	struct SelectOptionInfo
	{
		std::wstring name;
		std::vector<std::wstring> elements;
	};

	struct FileOptionInfo
	{
		std::wstring name;
		std::wstring regex;
	};

	struct Keyframe
	{
		double time;
		double value;
		std::wstring easing;
	};

	struct Timeline
	{
		std::wstring timer;
		boost::optional<double> loop;
		std::vector<Keyframe> keyframes;
	};

	struct Condition
	{
		bool negate;
		std::wstring identifier;
	};

	struct Element
	{
		std::wstring type;
		std::wstring source;
		std::unordered_map<std::wstring, boost::any> src_params;
		std::vector<Condition> conditions;
		std::unordered_map<std::wstring, boost::any> dst_params;
	};

private:
	std::unordered_map<std::wstring, std::wstring> _variables;
	std::vector<boost::variant<SelectOptionInfo, FileOptionInfo>> _optionInfo;
	MapConfig _options;
	std::wstring _title;
	std::wstring _author;
	Type _type;
	int _resolutionWidth = 1280;
	int _resolutionHeight = 720;
	double _startInput = 0;
	double _loadEnd = 0;
	double _ready = 0;
	double _fadeOut = 0;
	double _close = 0;
	double _sceneTime = 0;
	std::vector<Element> _elements;

	static bool _preprocess(std::wstring path, std::wstring& str, Skin& skin, bool loadOption = true, bool saveOption = true);
	static bool _parse(std::wstring str, Skin& skin);

public:
	Skin() { }
	virtual ~Skin() { }

	static bool parse(std::wstring path, Skin& skin);

	const Type type() { return _type; }
	const int resolutionWidth() { return _resolutionWidth; }
	const int resolutionHeight() { return _resolutionHeight; }
	const double startInput() { return _startInput; }
	const double loadEnd() { return _loadEnd; }
	const double ready() { return _ready; }
	const double fadeOut() { return _fadeOut; }
	const double close() { return _close; }
	const double sceneTime() { return _sceneTime; }
	const std::vector<Element>& elements() { return _elements; }
};

FUSION_ADAPT_STRUCT_AUTO(Skin::Keyframe, (time)(value)(easing))
FUSION_ADAPT_STRUCT_AUTO(Skin::Timeline, (timer)(loop)(keyframes))
FUSION_ADAPT_STRUCT_AUTO(Skin::Condition, (negate)(identifier))
FUSION_ADAPT_STRUCT_AUTO(Skin::Element, (type)(source)(src_params)(conditions)(dst_params))
