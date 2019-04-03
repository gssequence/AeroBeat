#pragma once
#include "stdafx.h"

#define CP_SHIFT_JIS 932

#define TRANSFORM_STRUCT_MEMBER(ign, name, member) (decltype(name::member), member)
#define FUSION_ADAPT_STRUCT_AUTO(name, members) BOOST_FUSION_ADAPT_STRUCT(name, BOOST_PP_SEQ_FOR_EACH(TRANSFORM_STRUCT_MEMBER, name, members))

namespace qi = boost::spirit::qi;

namespace util
{
	static bool starts_with(const std::wstring& str, const std::wstring substr)
	{
		return str.substr(0, substr.length()) == substr;
	}

	static bool contains(const std::wstring& str, const std::wstring& substr)
	{
		return str.find(substr) != std::wstring::npos;
	}

	static std::wstring trim(const std::wstring& string, const wchar_t* trimCharacterList = L" \t\v\r\n\x3000")
	{
		std::wstring result;
		std::wstring::size_type left = string.find_first_not_of(trimCharacterList);

		if (left != std::wstring::npos)
		{
			std::wstring::size_type right = string.find_last_not_of(trimCharacterList);
			result = string.substr(left, right - left + 1);
		}

		return std::move(result);
	}

	static std::wstring tolower(const std::wstring& str)
	{
		std::wstring ret;
		std::transform(str.begin(), str.end(), std::back_inserter(ret), std::towlower);
		return std::move(ret);
	}

	static std::wstring toupper(const std::wstring& str)
	{
		std::wstring ret;
		std::transform(str.begin(), str.end(), std::back_inserter(ret), std::towupper);
		return std::move(ret);
	}

	static std::vector<std::wstring> tolines(std::wstring& str)
	{
		std::vector<std::wstring> lines;
		std::wstringstream steram(str);
		std::wstring line;
		while (std::getline(steram, line))
		{
			if (line[line.size() - 1] == '\n') line.erase(line.size() - 1);
			if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);
			lines.push_back(line);
		}
		return std::move(lines);
	}

	static boost::rational<long long> torational(double d)
	{
		boost::rational<long long> ret = 0;

		long long n = 1;
		while (true)
		{
			double ip, fp = std::modf(d, &ip);
			if (ip == 0 && fp == 0) break;
			ret += boost::rational<long long>((long long)std::round(ip), n);
			d = fp * 10;
			n *= 10;
			if (n < 0) break; // overflow
		}

		return ret;
	}

	template <typename T>
	static bool unwrap(const boost::optional<T>& op, T& dst)
	{
		if (op)
		{
			dst = *op;
			return true;
		}
		return false;
	}

	template <typename T, typename U>
	static boost::optional<U> get(const std::unordered_map<T, U>& map, const T& key)
	{
		auto it = map.find(key);
		if (it != map.cend())
			return it->second;
		else
			return boost::none;
	}
	template <typename T, typename U>
	static U get(const std::unordered_map<T, U>& map, const T& key, U def)
	{
		auto it = map.find(key);
		if (it != map.cend())
			return it->second;
		else
			return def;
	}

	template <typename T>
	static boost::optional<T> get_any(const boost::any& any)
	{
		auto ptr = boost::any_cast<T>(&any);
		if (ptr != nullptr)
			return *ptr;
		else
			return boost::none;
	}
	template <typename T>
	static T get_any(const boost::any& any, T def)
	{
		auto ptr = boost::any_cast<T>(&any);
		if (ptr != nullptr)
			return *ptr;
		else
			return def;
	}
	template <typename T, typename U>
	static boost::optional<U> get_any(const std::unordered_map<T, boost::any>& map, const T& key)
	{
		auto it = map.find(key);
		if (it != map.cend())
		{
			auto& any = it->second;
			auto ptr = boost::any_cast<U>(&any);
			if (ptr != nullptr)
				return *ptr;
			else
				return boost::none;
		}
		else
			return boost::none;
	}
	template <typename T, typename U>
	static U get_any(const std::unordered_map<T, boost::any>& map, const T& key, U def)
	{
		auto it = map.find(key);
		if (it != map.cend())
		{
			auto& any = it->second;
			auto ptr = boost::any_cast<U>(&any);
			if (ptr != nullptr)
				return *ptr;
			else
				return def;
		}
		else
			return def;
	}

	static int round(double d)
	{
		return (int)std::round(d);
	}

	static bool try_stod(std::wstring& str, double& dst)
	{
		auto it = str.begin(), end = str.end();
		double result;
		if (qi::phrase_parse(it, end, qi::double_, qi::standard_wide::space, result) && it == end)
		{
			dst = result;
			return true;
		}
		return false;
	}

	template<typename T>
	static bool try_stod_select(std::wstring& str, T& dst, std::function<T(double)> selector)
	{
		double d;
		if (try_stod(str, d))
		{
			dst = selector(d);
			return true;
		}
		return false;
	}

	static bool try_stod_round(std::wstring& str, int& dst)
	{
		return try_stod_select<int>(str, dst, [](auto a) { return util::round(a); });
	}

	static bool try_stod_floor(std::wstring& str, int& dst)
	{
		return try_stod_select<int>(str, dst, [](auto a) { return (int)std::floor(a); });
	}

	template <typename T>
	static constexpr T clip(T value, T min, T max)
	{
		return value > max ? max : value < min ? min : value;
	}

	template <typename T>
	static std::vector<int> digits(T number)
	{
		std::vector<int> ret;
		number = std::abs(number);
		if (number == 0) return { 0 };
		while (number > 0)
		{
			ret.push_back(number % 10);
			number /= 10;
		}
		return std::move(ret);
	}

	template <typename T>
	static int digit_number(T number)
	{
		int ret = 0;
		number = std::abs(number);
		if (number == 0) return 1;
		while (number > 0)
		{
			ret++;
			number /= 10;
		}
		return ret;
	}

	static void clear_mouse_input()
	{
		int d;
		dx::GetMouseInputLog(&d, &d, &d);
	}

	static int get_mouse_input()
	{
		int d, b;
		int r = dx::GetMouseInputLog(&b, &d, &d);
		if (r == -1) return 0;
		return b;
	}

	static std::wstring compute_sha256(const char* data, size_t length)
	{
		HCRYPTPROV hProv = NULL;
		HCRYPTHASH hHash = NULL;
		std::array<BYTE, 32> sha;
		DWORD sha_size = sha.size();
		auto release_exit = [&](auto a)
		{
			if (hHash) CryptDestroyHash(hHash);
			if (hProv) CryptReleaseContext(hProv, 0);
			return a;
		};

		if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
			return release_exit(L"");
		if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
			return release_exit(L"");
		if (!CryptHashData(hHash, (const BYTE*)data, length, 0))
			return release_exit(L"");
		if (!CryptGetHashParam(hHash, HP_HASHVAL, sha.data(), &sha_size, 0))
			return release_exit(L"");

		std::wstringstream out;
		for (auto a : sha)
			out << std::setfill(L'0') << std::setw(2) << std::right << std::hex << a;
		return release_exit(out.str());
	}

	static void decode(const char* buf, const size_t length, std::wstring& str)
	{
		if (length == 0) // Zero Length
		{
			str = L"";
			return;
		}
		if (length >= 3 && (unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) // UTF-8
		{
			auto wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, nullptr, 0);
			if (wlen != 0)
			{
				std::shared_ptr<wchar_t> wbuf(new wchar_t[wlen], std::default_delete<wchar_t[]>());
				if (MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf.get(), wlen))
				{
					str = std::wstring(wbuf.get() + 1);
					return;
				}
			}
		}
		if (length >= 2 && (unsigned char)buf[0] == 0xFF && (unsigned char)buf[1] == 0xFE) // UTF-16 LE
		{
			auto l = length / 2;
			std::shared_ptr<wchar_t> wbuf(new wchar_t[l], std::default_delete<wchar_t[]>());
			for (unsigned int i = 0; i < l; ++i)
			{
				*(wbuf.get() + i) = (((unsigned char)buf[i * 2 + 1]) << 8) | (unsigned char)buf[i * 2];
			}
			str = std::wstring(wbuf.get() + 1);
			return;
		}
		if (length >= 2 && (unsigned char)buf[0] == 0xFE && (unsigned char)buf[1] == 0xFF) // UTF-16 BE
		{
			auto l = length / 2;
			std::shared_ptr<wchar_t> wbuf(new wchar_t[l], std::default_delete<wchar_t[]>());
			for (unsigned int i = 0; i < l; ++i)
			{
				*(wbuf.get() + i) = (((unsigned char)buf[i * 2]) << 8) | (unsigned char)buf[i * 2 + 1];
			}
			str = std::wstring(wbuf.get() + 1);
			return;
		}

		auto wlen = MultiByteToWideChar(CP_SHIFT_JIS, 0, buf, -1, nullptr, 0);
		std::shared_ptr<wchar_t> wbuf(new wchar_t[wlen], std::default_delete<wchar_t[]>());
		if (MultiByteToWideChar(CP_SHIFT_JIS, 0, buf, -1, wbuf.get(), wlen))
		{
			str = std::wstring(wbuf.get());
			return;
		}
		else
		{
			str = L"";
			return;
		}
	}
	static bool decode(const std::wstring& path, std::wstring& str, std::wstring* hash = nullptr)
	{
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		if (!ifs) return false;

		ifs.seekg(0, std::ifstream::end);
		auto end = ifs.tellg();
		ifs.clear();
		ifs.seekg(0, std::ifstream::beg);
		auto beg = ifs.tellg();
		unsigned int size = (unsigned int)(end - beg + 2);

		std::shared_ptr<char> buf(new char[size], std::default_delete<char[]>());
		ifs.read(buf.get(), size);
		buf.get()[size - 1] = 0; // null terminate
		buf.get()[size - 2] = 0; // null terminate
		decode(buf.get(), size, str);
		if (hash)
			*hash = compute_sha256(buf.get(), size - 2);
		return true;
	}
}
