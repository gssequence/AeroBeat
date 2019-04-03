#include "stdafx.h"
#include "Skin.h"

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace xpressive = boost::xpressive;

#pragma region Preprocessor

struct PreprocessorStringLiteral
{
	std::wstring str;
};
struct PreprocessorSymbolLiteral
{
	std::wstring sym;
};
struct PreprocessorPairLiteral
{
	PreprocessorSymbolLiteral sym;
	PreprocessorStringLiteral str;
};
typedef boost::variant<PreprocessorPairLiteral, PreprocessorSymbolLiteral, PreprocessorStringLiteral> PreprocessorArgument;
struct PreprocessorDefinition
{
	std::wstring op;
	std::vector<PreprocessorArgument> args;
};

FUSION_ADAPT_STRUCT_AUTO(PreprocessorStringLiteral, (str))
FUSION_ADAPT_STRUCT_AUTO(PreprocessorSymbolLiteral, (sym))
FUSION_ADAPT_STRUCT_AUTO(PreprocessorPairLiteral, (sym)(str))
FUSION_ADAPT_STRUCT_AUTO(PreprocessorDefinition, (op)(args))

template<typename Iterator>
struct SkinSkipper : qi::grammar<Iterator>
{
	SkinSkipper() : base_type(definition)
	{
		definition = qi::standard_wide::space | L"//" >> *(qi::standard_wide::char_ - qi::eol) >> qi::eol;
	}

	qi::rule<Iterator> definition;
};

template<typename Iterator>
struct PreprocessParser : qi::grammar<Iterator, PreprocessorDefinition(), SkinSkipper<Iterator>>
{
	PreprocessParser() : base_type(definition)
	{
		definition = qi::lexeme[L'#' >> operatorLiteral[phoenix::at_c<0>(qi::_val) = qi::_1]] >> arguments[phoenix::at_c<1>(qi::_val) = qi::_1];
		arguments %= (pairLiteral | symbolLiteral | stringLiteral) % L',' | qi::eps;
		operatorLiteral %= +qi::standard_wide::alpha;
		stringLiteral %= qi::lexeme[L'"' >> +(qi::standard_wide::char_ - L'"') >> L'"'];
		symbolLiteral %= L'$' >> qi::lexeme[(qi::standard_wide::alpha | qi::standard_wide::char_(L'_')) >> *(qi::standard_wide::alnum | qi::standard_wide::char_(L'_'))];
		pairLiteral %= symbolLiteral >> L':' >> stringLiteral;
	}

	qi::rule<Iterator, PreprocessorDefinition(), SkinSkipper<Iterator>> definition;
	qi::rule<Iterator, std::vector<PreprocessorArgument>, SkinSkipper<Iterator>> arguments;
	qi::rule<Iterator, std::wstring()> operatorLiteral;
	qi::rule<Iterator, PreprocessorStringLiteral(), SkinSkipper<Iterator>> stringLiteral;
	qi::rule<Iterator, PreprocessorSymbolLiteral(), SkinSkipper<Iterator>> symbolLiteral;
	qi::rule<Iterator, PreprocessorPairLiteral(), SkinSkipper<Iterator>> pairLiteral;
};

#pragma endregion

#pragma region Parser

template<typename Iterator>
struct SkinParser : qi::grammar<Iterator, std::vector<Skin::Element>(), SkinSkipper<Iterator>>
{
	SkinParser() : base_type(definition)
	{
		using phoenix::at_c;
		using qi::_val;
		using qi::_1;

		definition %= *element;
		element %= image_ss | generic_def;

		image_ss = qi::eps[at_c<0>(_val) = L"image_source"] >> source[at_c<1>(_val) = _1] >> L'=' >> stringLiteral[at_c<2>(_val)[L"path"] = _1];
		generic_def = identifier[at_c<0>(_val) = _1]
			>> -source[at_c<1>(_val) = _1]
			>> -src_params[at_c<2>(_val) = _1]
			>> -conditions[at_c<3>(_val) = _1]
			>> -dst_params[at_c<4>(_val) = _1];

		source %= L'[' >> identifier >> L']';
		src_params %= L'(' >> parameters >> -qi::lit(L',') >> L')';
		conditions %= L'<' >> condition_list >> -qi::lit(L',') >> L'>';
		dst_params %= L'{' >> parameters >> -qi::lit(L',') >> L'}';
		
		identifier %= qi::lexeme[(qi::standard_wide::alpha | qi::standard_wide::char_(L'_')) >> *(qi::standard_wide::alnum | qi::standard_wide::char_(L'_'))];
		identifiers %= identifier % L',' | qi::eps;
		stringLiteral %= qi::lexeme[L'"' >> +(qi::standard_wide::char_ - L'"') >> L'"'];
		parameter %= identifier >> L':' >> (stringLiteral | identifier | timeline | qi::double_);
		parameters %= parameter % L',' | qi::eps;
		timeline %= L'(' >> identifier >> -(L',' >> qi::double_) >> L')' >> L'{' >> keyframes >> -qi::lit(L',') >> L'}';
		keyframe %= qi::double_ >> L':' >> qi::double_ >> -easing;
		easing %= L'(' >> identifier >> L')';
		keyframes %= keyframe % L',' | qi::eps;
		condition = -qi::standard_wide::char_(L'!')[at_c<0>(_val) = true] >> identifier[at_c<1>(_val) = _1];
		condition_list %= condition % L',' | qi::eps;
	}

	qi::rule<Iterator, std::vector<Skin::Element>(), SkinSkipper<Iterator>> definition;
	qi::rule<Iterator, Skin::Element(), SkinSkipper<Iterator>> element;
	qi::rule<Iterator, Skin::Element(), SkinSkipper<Iterator>> image_ss;
	qi::rule<Iterator, Skin::Element(), SkinSkipper<Iterator>> generic_def;
	qi::rule<Iterator, std::wstring(), SkinSkipper<Iterator>> source;
	qi::rule<Iterator, std::unordered_map<std::wstring, boost::any>(), SkinSkipper<Iterator>> src_params;
	qi::rule<Iterator, std::vector<Skin::Condition>(), SkinSkipper<Iterator>> conditions;
	qi::rule<Iterator, std::unordered_map<std::wstring, boost::any>(), SkinSkipper<Iterator>> dst_params;
	qi::rule<Iterator, std::wstring(), SkinSkipper<Iterator>> identifier;
	qi::rule<Iterator, std::vector<std::wstring>(), SkinSkipper<Iterator>> identifiers;
	qi::rule<Iterator, std::wstring(), SkinSkipper<Iterator>> stringLiteral;
	qi::rule<Iterator, std::pair<std::wstring, boost::any>(), SkinSkipper<Iterator>> parameter;
	qi::rule<Iterator, std::unordered_map<std::wstring, boost::any>(), SkinSkipper<Iterator>> parameters;
	qi::rule<Iterator, Skin::Timeline(), SkinSkipper<Iterator>> timeline;
	qi::rule<Iterator, Skin::Keyframe(), SkinSkipper<Iterator>> keyframe;
	qi::rule<Iterator, std::wstring(), SkinSkipper<Iterator>> easing;
	qi::rule<Iterator, std::vector<Skin::Keyframe>(), SkinSkipper<Iterator>> keyframes;
	qi::rule<Iterator, Skin::Condition(), SkinSkipper<Iterator>> condition;
	qi::rule<Iterator, std::vector<Skin::Condition>(), SkinSkipper<Iterator>> condition_list;
};

#pragma endregion

bool Skin::_preprocess(std::wstring path, std::wstring& str, Skin& skin, bool loadOption, bool saveOption)
{
	auto configPath = path + L".json";
	if (loadOption)
		MapConfig::load(configPath, skin._options);
	std::wstring content;
	if (!util::decode(path, content)) return false;
	auto lines = util::tolines(content);

	bool pass = false;
	bool done = false;

	for (unsigned int i = 0; i < lines.size(); i++)
	{
		auto& line = lines[i];
		auto it = line.begin();
		auto end = line.end();
		PreprocessParser<decltype(it)> parser;
		SkinSkipper<decltype(it)> skipper;
		PreprocessorDefinition definition;
		if (qi::phrase_parse(it, end, parser, skipper, definition) && it == end)
		{
			auto op = definition.op;
			std::transform(op.begin(), op.end(), op.begin(), std::towupper);
			auto argc = definition.args.size();
			if (op == L"INCLUDE" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto subpath = ptr->str;
					std::wstring subcontent;
					if (util::decode(subpath, subcontent))
					{
						auto sublines = util::tolines(subcontent);
						lines.resize(lines.size() + sublines.size());
						lines.insert(lines.begin() + i + 1, sublines.begin(), sublines.end());
					}
				}
				else
				{
					auto ptr = boost::get<PreprocessorSymbolLiteral>(&definition.args[0]);
					if (ptr != nullptr)
					{
						auto sym = ptr->sym;
						if (skin._variables.find(sym) != skin._variables.end())
						{
							auto subpath = skin._variables[sym];
							std::wstring subcontent;
							if (util::decode(subpath, subcontent))
							{
								auto sublines = util::tolines(subcontent);
								lines.resize(lines.size() + sublines.size());
								lines.insert(lines.begin() + i + 1, sublines.begin(), sublines.end());
							}
						}
					}
				}
			}
			else if (op == L"TITLE" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
					skin._title = ptr->str;
			}
			else if (op == L"AUTHOR" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
					skin._author = ptr->str;
			}
			else if (op == L"TYPE" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto typestr = ptr->str;
					std::transform(typestr.begin(), typestr.end(), typestr.begin(), std::towlower);
					if (typestr == L"7keys")
						skin._type = Skin::Type::Play7;
					else if (typestr == L"decide")
						skin._type = Skin::Type::Decide;
					else if (typestr == L"musicselect")
						skin._type = Skin::Type::MusicSelect;
					else if (typestr == L"skinselect")
						skin._type = Skin::Type::SkinSelect;
					else if (typestr == L"keyconfig")
						skin._type = Skin::Type::KeyConfig;
					else if (typestr == L"result")
						skin._type = Skin::Type::Result;
				}
			}
			else if (op == L"RESOLUTION" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					int w, h;
					std::transform(str.begin(), str.end(), str.begin(), std::towlower);
					auto it = str.begin();
					auto end = str.end();
					if (str == L"sd")
					{
						skin._resolutionWidth = 640;
						skin._resolutionHeight = 480;
					}
					else if (str == L"hd")
					{
						skin._resolutionWidth = 1280;
						skin._resolutionHeight = 720;
					}
					else if (qi::parse(it, end, qi::int_[phoenix::ref(w) = qi::_1] >> L"x" >> qi::int_[phoenix::ref(h) = qi::_1]) && it == end && w > 0 && h > 0)
					{
						skin._resolutionWidth = w;
						skin._resolutionHeight = h;
					}
				}
			}
			else if (op == L"STARTINPUT" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._startInput = t;
				}
			}
			else if (op == L"LOADEND" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._loadEnd = t;
				}
			}
			else if (op == L"READY" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._ready = t;
				}
			}
			else if (op == L"FADEOUT" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._fadeOut = t;
				}
			}
			else if (op == L"CLOSE" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._close = t;
				}
			}
			else if (op == L"SCENETIME" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto str = ptr->str;
					auto it = str.begin();
					auto end = str.end();
					double t;
					if (qi::parse(it, end, qi::double_[phoenix::ref(t) = qi::_1]) && it == end)
						skin._sceneTime = t;
				}
			}
			else if (op == L"DEFINE" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorPairLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					skin._variables[ptr->sym.sym] = ptr->str.str;
				}
				else
				{
					auto ptr = boost::get<PreprocessorSymbolLiteral>(&definition.args[0]);
					if (ptr != nullptr)
					{
						skin._variables[ptr->sym] = L"";
					}
				}
			}
			else if (op == L"UNDEF" && argc == 1 && !pass)
			{
				auto ptr = boost::get<PreprocessorSymbolLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					skin._variables.erase(ptr->sym);
				}
			}
			else if (op == L"OPTION" && argc >= 2 && !pass)
			{
				auto ptitle = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				if (ptitle == nullptr) continue;
				bool flag = false;
				std::map<std::wstring, std::wstring> elements;
				std::vector<std::wstring> keys;
				for (auto it = definition.args.begin() + 1; it != definition.args.end(); ++it)
				{
					auto ppair = boost::get<PreprocessorPairLiteral>(&*it);
					if (ppair == nullptr)
					{
						flag = true;
						break;
					}
					auto& l = ppair->str.str;
					auto& r = ppair->sym.sym;
					elements[l] = r;
					keys.push_back(l);
				}
				if (flag) continue;

				SelectOptionInfo info;
				info.name = ptitle->str;
				info.elements = keys;
				skin._optionInfo.push_back(info);

				std::wstring selected;
				auto& options = skin._options.elements();
				if (options.find(ptitle->str) != options.end())
					selected = options[ptitle->str];
				else
					selected = *keys.begin();
				if (elements.find(selected) == elements.end())
					selected = *keys.begin();
				skin._variables[elements[selected]] = L"";
				options[ptitle->str] = selected;
			}
			else if (op == L"FILE" && argc == 3)
			{
				auto ptitle = boost::get<PreprocessorStringLiteral>(&definition.args[0]);
				auto pdefine = boost::get<PreprocessorPairLiteral>(&definition.args[1]);
				auto pdefault = boost::get<PreprocessorStringLiteral>(&definition.args[2]);
				if (ptitle != nullptr && pdefine != nullptr && pdefault != nullptr)
				{
					auto title = ptitle->str;
					auto sym = pdefine->sym.sym;
					auto regex = pdefine->str.str;
					auto def = pdefault->str;

					FileOptionInfo info;
					info.name = title;
					info.regex = regex;
					skin._optionInfo.push_back(info);

					std::wstring selected;
					auto& options = skin._options.elements();
					if (options.find(title) != options.end())
						selected = options[title];
					else
						selected = def;
					skin._variables[sym] = selected;
					options[title] = selected;
				}
			}
			else if (op == L"IF" && argc == 1)
			{
				auto ptr = boost::get<PreprocessorSymbolLiteral>(&definition.args[0]);
				if (ptr != nullptr)
				{
					auto symstr = ptr->sym;
					if (skin._variables.find(symstr) != skin._variables.end())
					{
						pass = false;
						done = true;
					}
					else
					{
						pass = true;
						done = false;
					}
				}
			}
			else if (op == L"ELSEIF" && argc == 1)
			{
				if (done)
				{
					pass = true;
				}
				else
				{
					auto ptr = boost::get<PreprocessorSymbolLiteral>(&definition.args[0]);
					if (ptr != nullptr)
					{
						auto symstr = ptr->sym;
						if (skin._variables.find(symstr) != skin._variables.end())
						{
							pass = false;
							done = true;
						}
						else
						{
							pass = true;
							done = false;
						}
					}
				}
			}
			else if (op == L"ELSE" && argc == 0)
			{
				if (done)
				{
					pass = true;
				}
				else
				{
					pass = false;
					done = true;
				}
			}
			else if (op == L"ENDIF" && argc == 0)
			{
				pass = false;
				done = false;
			}
		}
		else if (!pass)
		{
			xpressive::wsregex regex = L'$' >> (xpressive::s1 = ((xpressive::alpha | L'_') >> *(xpressive::alnum | L'_')));
			line = xpressive::regex_replace(line, regex, [&](const xpressive::wsmatch& match) -> std::wstring {
				auto sym = match[1];
				if (skin._variables.find(sym) != skin._variables.end())
					return L'"' + skin._variables[sym] + L'"';
				else
					return match[0];
			});
			str += line + L"\n";
		}
	}
	if (saveOption)
		skin._options.save(configPath);
	return true;
}

bool Skin::_parse(std::wstring str, Skin& skin)
{
	auto it = str.begin();
	auto end = str.end();
	SkinParser<decltype(it)> parser;
	SkinSkipper<decltype(it)> skipper;
	decltype(skin._elements) elements;
	if (qi::phrase_parse(it, end, parser, skipper, elements) && it == end)
	{
		skin._elements = std::move(elements);
		return true;
	}
	return false;
}

bool Skin::parse(std::wstring path, Skin& skin)
{
	std::wstring str;
	if (!_preprocess(path, str, skin)) return false;
	if (!_parse(str, skin)) return false;
	return true;
}
