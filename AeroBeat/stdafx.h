#pragma once

#pragma comment(lib, "portaudio_x86.lib")
#ifdef _DEBUG
#pragma comment(lib, "sqlite3_d.lib")
#else
#pragma comment(lib, "sqlite3.lib")
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

#include <boost/fusion/include/std_pair.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/rational.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <portaudio.h>

#include <sqlite3.h>

namespace fs = std::experimental::filesystem;
namespace dx = DxLib;
