#include "stdafx.h"
#include "util.h"
#include "BmsData.h"

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

struct HeaderStatement
{
	std::wstring name;
	std::wstring data;
};
struct ChannelStatement
{
	int bar;
	int channel;
	std::wstring data;
};
typedef boost::variant<HeaderStatement, ChannelStatement> Statement;

FUSION_ADAPT_STRUCT_AUTO(HeaderStatement, (name)(data))
FUSION_ADAPT_STRUCT_AUTO(ChannelStatement, (bar)(channel)(data))

template<typename Iterator>
struct BmsStatementSkipper : qi::grammar<Iterator>
{
	BmsStatementSkipper() : base_type(definition)
	{
		definition = qi::standard_wide::space | L'\x3000';
	}

	qi::rule<Iterator> definition;
};

template<typename Iterator>
struct StatementParser : qi::grammar<Iterator, Statement(), BmsStatementSkipper<Iterator>>
{
	StatementParser() : base_type(definition)
	{
		using qi::lexeme;

		definition %= L'#' >> (channel | header);
		header %= lexeme[*qi::standard_wide::alnum] >> -data;
		channel %= number3base10 >> number2base36 >> L':' >> data;
		data %= lexeme[*qi::standard_wide::char_];
	}

	qi::rule<Iterator, Statement(), BmsStatementSkipper<Iterator>> definition;
	qi::rule<Iterator, HeaderStatement(), BmsStatementSkipper<Iterator>> header;
	qi::rule<Iterator, ChannelStatement(), BmsStatementSkipper<Iterator>> channel;
	qi::rule<Iterator, std::wstring(), BmsStatementSkipper<Iterator>> data;
	qi::uint_parser<int, 36, 2, 2> number2base36;
	qi::uint_parser<int, 10, 3, 3> number3base10;
};

template<typename Iterator, int Radix = 36>
struct ChannelDataParser : qi::grammar<Iterator, std::vector<int>(), BmsStatementSkipper<Iterator>>
{
	ChannelDataParser() : base_type(definition)
	{
		definition %= *number2base36 >> -qi::omit[qi::alnum];
	}

	qi::rule<Iterator, std::vector<int>(), BmsStatementSkipper<Iterator>> definition;
	qi::uint_parser<int, Radix, 2, 2> number2base36;
};

struct RawData
{
	int channel;
	int bar;
	boost::rational<long long> position;
	int data;
};

bool BmsData::parse(std::wstring str, BmsData& data, std::function<unsigned int(unsigned int)> next)
{
	std::stack<int> st_random, st_if;
	auto proceed = [&]
	{
		return st_random == st_if;
	};
	auto pop_eq = [&]
	{
		while (st_random.size() > st_if.size())
			st_random.pop();
	};

	std::wstring title_temp;
	std::wstring subtitle_temp;
	std::vector<RawData> raw;

	// 解析
	std::wstringstream stream(str);
	std::wstring line;
	while (std::getline(stream, line))
	{
		line = util::trim(line);
		auto it = line.begin(), end = line.end();
		StatementParser<decltype(it)> parser;
		BmsStatementSkipper<decltype(it)> skipper;
		Statement statement;
		if (qi::phrase_parse(it, end, parser, skipper, statement) && it == end)
		{
			if (statement.which() == 0)
			{
				auto& s = boost::get<HeaderStatement>(statement);
				std::transform(s.name.begin(), s.name.end(), s.name.begin(), std::towupper);
				s.data = util::trim(s.data);
				
				if (s.name == L"RANDOM")
				{
					int i;
					if (util::try_stod_round(s.data, i) && i > 0)
						st_random.push(next(--i));
					data._has_random = true;
					continue;
				}
				else if (s.name == L"ENDRANDOM")
				{
					if (!st_random.empty())
						st_random.pop();
					continue;
				}
				else if (s.name == L"IF")
				{
					int i;
					if (util::try_stod_round(s.data, i))
						st_if.push(--i);
					continue;
				}
				else if (s.name == L"ENDIF")
				{
					if (!st_if.empty())
						st_if.pop();
					continue;
				}

				pop_eq();
				if (proceed())
				{
					auto namelen = s.name.size();
					auto indexname = s.name.substr(0, namelen - 2);
					auto index = std::stoi(s.name.substr(s.name.size() - 2), nullptr, 36);
					if (s.name == L"PLAYER")
						util::try_stod_round(s.data, data._player);
					else if (s.name == L"RANK")
						util::try_stod_round(s.data, data._judgeRank);
					else if (s.name == L"TOTAL")
						util::try_stod(s.data, data._total);
					else if (s.name == L"STAGEFILE")
						data._stageFile = s.data;
					else if (s.name == L"BANNER")
						data._banner = s.data;
					else if (s.name == L"BACKBMP")
						data._backbmp = s.data;
					else if (s.name == L"PLAYLEVEL")
						util::try_stod_round(s.data, data._playLevel);
					else if (s.name == L"DIFFICULTY")
						util::try_stod_round(s.data, data._difficulty);
					else if (s.name == L"TITLE")
						title_temp = s.data;
					else if (s.name == L"SUBTITLE")
						subtitle_temp = s.data;
					else if (s.name == L"ARTIST")
						data._artist = s.data;
					else if (s.name == L"SUBARTIST")
						data._subartist = s.data;
					else if (s.name == L"GENRE")
						data._genre = s.data;
					else if (s.name == L"BPM")
						util::try_stod(s.data, data._initialBpm);
					else if (namelen == 5 && indexname == L"BPM" || namelen == 7 && indexname == L"EXBPM")
					{
						double d;
						if (util::try_stod(s.data, d))
							data._bpms[index] = d;
					}
					else if (namelen == 6 && indexname == L"STOP")
					{
						int i;
						if (util::try_stod_floor(s.data, i))
							data._stops[index] = i;
					}
					else if (s.name == L"LNOBJ")
					{
						auto i = s.data.begin(), e = s.data.end();
						int ln;
						if (qi::parse(i, e, qi::uint_parser<int, 36, 2, 2>(), ln) && i == e)
							data._lnobj = ln;
					}
					else if (namelen == 5 && indexname == L"WAV")
						data._wavs[index] = s.data;
					else if (namelen == 5 && indexname == L"BMP")
						data._bmps[index] = s.data;
				}
			}
			else if (statement.which() == 1)
			{
				auto& s = boost::get<ChannelStatement>(statement);

				pop_eq();
				if (proceed())
				{
					if (s.channel == AB_BMS_CHANNEL_BAR_LENGTH)
					{
						double d;
						if (util::try_stod(s.data, d))
							data._barLengths[s.bar] = d;
					}
					else
					{
						std::vector<int> d;
						auto i = s.data.begin(), e = s.data.end();
						BmsStatementSkipper<decltype(i)> sk;
						bool b = false;
						if (s.channel == AB_BMS_CHANNEL_BPM_CHANGE)
						{
							ChannelDataParser<decltype(i), 16> p;
							b = qi::phrase_parse(i, e, p, sk, d) && i == e;
						}
						else
						{
							ChannelDataParser<decltype(i)> p;
							b = qi::phrase_parse(i, e, p, sk, d) && i == e;
						}
						if (b)
						{
							int c = d.size();
							for (auto& a : d)
							{
								if (a == 0) continue;
								raw.push_back({ s.channel, s.bar, boost::rational<long long>(&a - &d[0], c), a });
							}
						}
					}
				}
			}
		}
	}

	// 自動SUBTITLE
	if (subtitle_temp.empty() && title_temp.size() >= 2)
	{
		auto rit = title_temp.rbegin(), end = title_temp.rend();
		auto last = *rit++;
		wchar_t first = L'\0';
		if (last == L'-') first = L'-';
		else if (last == L'~') first = L'~';
		else if (last == L'～') first = L'～';
		else if (last == L')') first = L'(';
		else if (last == L']') first = L'[';
		else if (last == L']') first = L'[';
		else if (last == L'>') first = L'<';
		else if (last == L'"') first = L'"';
		if (first)
		{
			for (; rit != end - 1; ++rit)
			{
				if (*rit == first)
				{
					subtitle_temp = std::wstring(rit.base() - 1, title_temp.end());
					title_temp = util::trim(std::wstring(title_temp.begin(), rit.base() - 1));
					break;
				}
			}
		}
	}
	data._title = title_temp;
	data._subtitle = subtitle_temp;

	// 最小・最大BPM設定
	data._minBpm = data._maxBpm = data._initialBpm;

	// 拍イベント追加
	for (int i = 0; i <= 999; i++)
	{
		double barLength = std::abs(data._barLengths[i]);
		if (barLength == 0) barLength = 1;
		double d = barLength * 4;
		int n = (int)std::floor(d);
		auto r = util::torational(d);
		if (n == 0)
			raw.push_back({ AB_BMS_CHANNEL_EX_BEAT, i, 0, 0 });
		else
		{
			for (int j = 0; j < n; j++)
				raw.push_back({ AB_BMS_CHANNEL_EX_BEAT, i, j / r, 0 });
		}
	}

	// ソートして逆順
	std::stable_sort(raw.begin(), raw.end(), [](auto& l, auto& r)
	{
		if (l.bar != r.bar) return l.bar < r.bar;
		if (l.position != r.position) return l.position < r.position;
		if (l.channel != r.channel) return l.channel < r.channel;
		return false;
	});
	std::reverse(raw.begin(), raw.end());

	// 重複データを消去
	raw.erase(std::unique(raw.begin(), raw.end(), [](auto& l, auto& r)
	{
		return l.bar == r.bar
			&& l.position == r.position
			&& (l.channel == r.channel
				|| l.channel == AB_BMS_CHANNEL_BPM_CHANGE && r.channel == AB_BMS_CHANNEL_EXBPM_CHANGE
				|| l.channel == AB_BMS_CHANNEL_EXBPM_CHANGE && r.channel == AB_BMS_CHANNEL_BPM_CHANGE)
			&& l.channel != AB_BMS_CHANNEL_BGM;
	}), raw.end());

	// 再度ソート(チャンネルを気にする)
	std::stable_sort(raw.begin(), raw.end(), [](auto& l, auto& r)
	{
		if (l.bar != r.bar) return l.bar < r.bar;
		if (l.position != r.position) return l.position < r.position;
		if (r.channel == AB_BMS_CHANNEL_STOP) return true;
		if (l.channel == AB_BMS_CHANNEL_STOP) return false;
		if (r.channel == AB_BMS_CHANNEL_BPM_CHANGE || r.channel == AB_BMS_CHANNEL_EXBPM_CHANGE) return true;
		if (l.channel == AB_BMS_CHANNEL_BPM_CHANGE || l.channel == AB_BMS_CHANNEL_EXBPM_CHANGE) return false;
		return false;
	});

	// 実時間計算
	double t = 0;
	double bpm = data._initialBpm;
	auto it = raw.begin(), end = raw.end();
	for (int i = 0; it != end; i++)
	{
		// 小節長
		double barLength = std::abs(data._barLengths[i]);
		if (barLength == 0) barLength = 1;

		// 小節線を追加
		data._events.push_back({ t, AB_BMS_CHANNEL_EX_BAR_LINE, 0, 0 });

		// 前回処理したオブジェクト位置
		boost::rational<long long> beforePosition = 0;

		// i小節目にあるオブジェクトを処理
		for (; it != end && it->bar == i; ++it)
		{
			auto& obj = *it;

			// 時間tとオブジェクト位置更新
			if (beforePosition != obj.position)
			{
				double diff = boost::rational_cast<double>(obj.position - beforePosition);
				t += barLength * (4 / (bpm / 60)) * diff;
				beforePosition = obj.position;
			}

			if (obj.channel == AB_BMS_CHANNEL_BPM_CHANGE || obj.channel == AB_BMS_CHANNEL_EXBPM_CHANGE)
			{
				// BPM変更時
				double tempo = obj.channel == AB_BMS_CHANNEL_BPM_CHANGE ? obj.data : data._bpms[obj.data];
				data._minBpm = min(data._minBpm, tempo);
				data._maxBpm = max(data._maxBpm, tempo);
				if (tempo <= 0)
				{
					// どうしよう…
					continue;
				}
				bpm = tempo;
				data._events.push_back({ t, AB_BMS_CHANNEL_BPM_CHANGE, 0, tempo });
			}
			else if (obj.channel == AB_BMS_CHANNEL_STOP)
			{
				// STOP時
				int tick = data._stops[obj.data];
				double duration = (4 / (bpm / 60) * (tick / 196.0));
				data._events.push_back({ t, obj.channel, 0, duration });
				t += duration;
			}
			else
				data._events.push_back({ t, obj.channel, obj.data });
		}
		t += boost::rational_cast<double>(1 - beforePosition) * barLength * (4 / (bpm / 60));
	}

	// LN処理
	auto rend = data._events.rend();
	for (auto it = data._events.rbegin(); it != rend; ++it)
	{
		// LN終点を見つける
		auto& lnend = *it;
		if (AB_BMS_CHANNEL_NOTE(lnend.channel) && lnend.data == data._lnobj || AB_BMS_CHANNEL_LN(lnend.channel))
		{
			for (auto itr = it + 1; itr != rend; ++itr)
			{
				// LN始点を見つける
				auto& lnbegin = *itr;
				if (lnend.channel == lnbegin.channel)
				{
					if (lnbegin.data == data._lnobj) continue;

					// 終端の時間を代入
					lnbegin.extra = lnend.time;

					// LNの終端のチャンネルをLN終端専用チャンネルへ移動
					lnend.channel = AB_BMS_CHANNEL_EX_LN_END;

					// 始点をLN専用チャンネルからキー配置チャンネルへ移動
					if (AB_BMS_CHANNEL_LN(lnbegin.channel))
						lnbegin.channel += AB_BMS_CHANNEL_NOTE_BEGIN - AB_BMS_CHANNEL_LN_BEGIN;

					// フラグを立てる
					data._has_ln = true;
				}
			}
		}
	}

	// #LNOBJ LN終端オブジェクトを削除
	data._events.erase(std::remove_if(data._events.begin(), data._events.end(), [&](auto& a)
	{
		return (AB_BMS_CHANNEL_NOTE(a.channel)) && a.data == data._lnobj;
	}), data._events.end());

	// TOTAL NOTESを計算
	for (auto& a : data._events)
	{
		if (AB_BMS_CHANNEL_NOTE(a.channel))
			data._totalnotes++;
	}

	// 自動難易度設定
	if (data._difficulty < 1 || 5 < data._difficulty)
		data._difficulty = 0;

	// サブタイトルで判定
	if (data._difficulty == 0 && data._subtitle.size() >= 3)
	{
		std::wstring str(data._subtitle.begin() + 1, data._subtitle.end() - 1);
		std::transform(str.begin(), str.end(), str.begin(), std::towlower);
		if (util::contains(str, L"insane") || util::contains(str, L"black"))
			data._difficulty = 5;
		else if (util::contains(str, L"another") || str == L"a")
			data._difficulty = 4;
		else if (util::contains(str, L"hyper") || util::contains(str, L"7keys") || str == L"h")
			data._difficulty = 3;
		else if (util::contains(str, L"normal") || util::contains(str, L"light") || str == L"n")
			data._difficulty = 2;
		else if (util::contains(str, L"beginner"))
			data._difficulty = 1;
	}

	// ジャンルで判定
	if (data._difficulty == 0 && data._genre.size() > 0)
	{
		auto str = data._genre;
		std::transform(str.begin(), str.end(), str.begin(), std::towlower);
		if (util::contains(str, L"insane") || util::contains(str, L"black"))
			data._difficulty = 5;
		else if (util::contains(str, L"another"))
			data._difficulty = 4;
		else if (util::contains(str, L"hyper") || util::contains(str, L"7keys"))
			data._difficulty = 3;
		else if (util::contains(str, L"normal") || util::contains(str, L"light"))
			data._difficulty = 2;
		else if (util::contains(str, L"beginner"))
			data._difficulty = 1;
	}

	// 密度で判定(適当)
	if (data._difficulty == 0 && data._events.size() > 0)
	{
		// 最後のノーツ
		auto it = data._events.rbegin();
		for (; it != data._events.rend(); ++it)
		{
			if (AB_BMS_CHANNEL_NOTE(it->channel))
				break;
		}
		// 2分あたりのノート数
		auto density = data._totalnotes / it->time * 60 * 2;
		if (density < 300)
			data._difficulty = 1;
		else if (density < 500)
			data._difficulty = 2;
		else if (density < 1200)
			data._difficulty = 3;
		else if (density < 1800)
			data._difficulty = 4;
		else
			data._difficulty = 5;
	}

	return true;
}
