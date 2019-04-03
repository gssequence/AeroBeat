#include "stdafx.h"
#include "util.h"
#include "BmsData.h"
#include "SongManager.h"

SongManager::SongManager()
{
	std::error_code ec;
	fs::create_directories(L"ABFiles/Databases", ec);
	_pdb = SQLite::open(L"ABFiles/Databases/Main.db");

	if (_pdb->fail()) return;

	auto stmt = SQLite::Statement::make(_pdb,
		L"CREATE TABLE IF NOT EXISTS songs(hash TEXT PRIMARY KEY, title TEXT, subtitle TEXT, artist TEXT, subartist TEXT, genre TEXT, folder TEXT, path TEXT, stagefile TEXT, "
		L"banner TEXT, backbmp TEXT, minbpm REAL, maxbpm REAL, player INTEGER, judgerank INTEGER, playlevel INTEGER, difficulty INTEGER, totalnotes INTEGER, longnote INTEGER, random INTEGER)");
	stmt->step();

	stmt = SQLite::Statement::make(_pdb, L"CREATE TABLE IF NOT EXISTS scores(hash TEXT PRIMARY KEY, clearlamp INTEGER, maxcombo INTEGER, misscount INTEGER, playcount INTEGER, rate REAL)");
	stmt->step();
}

void SongManager::initialize(Config& config, std::function<void(std::wstring)> logger)
{
	if (_pdb->fail() || !config.scanBmsAtLaunch()) return;

	auto stmt = SQLite::Statement::make(_pdb, L"INSERT INTO songs VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	auto exist_stmt = SQLite::Statement::make(_pdb, L"SELECT hash FROM songs WHERE hash = ?");
	for (fs::path a : config.bmsDirectories())
	{
		logger(L"Scanning: " + a.generic_wstring());
		for (auto it = fs::recursive_directory_iterator(a), end = fs::recursive_directory_iterator(); it != end; ++it)
		{
			fs::path path = *it;
			auto ext = path.extension().generic_wstring();
			std::transform(ext.begin(), ext.end(), ext.begin(), std::towlower);
			if (ext == L".bms" || ext == L".bme" || ext == L".bml" || ext == L".pms")
			{
				BmsData data;
				std::wstring text, hash;
				if (util::decode(path.generic_wstring(), text, &hash))
				{
					auto folder = path.parent_path().generic_wstring(), filename = path.filename().generic_wstring();
					exist_stmt->reset();
					exist_stmt->bind(hash);
					if (exist_stmt->step() == SQLITE_ROW)
						continue;

					if (BmsData::parse(text, data))
					{
						stmt->reset();
						stmt->bind(hash);
						stmt->bind(data.title());
						stmt->bind(data.subtitle());
						stmt->bind(data.artist());
						stmt->bind(data.subartist());
						stmt->bind(data.genre());
						stmt->bind(folder);
						stmt->bind(filename);
						stmt->bind(data.stageFile());
						stmt->bind(data.banner());
						stmt->bind(data.backbmp());
						stmt->bind(data.minBpm());
						stmt->bind(data.maxBpm());
						stmt->bind(data.player());
						stmt->bind(data.judgeRank());
						stmt->bind(data.playLevel());
						stmt->bind(data.difficulty());
						stmt->bind(data.totalnotes());
						stmt->bind(data.has_ln() ? 1 : 0);
						stmt->bind(data.has_random() ? 1 : 0);
						if (stmt->step() == SQLITE_DONE)
							logger(L"BMS loaded: " + path.generic_wstring());
					}
				}
			}
		}
	}
}

std::shared_ptr<SongManager::Node> SongManager::root(Config& config)
{
	auto ret = Node::make(*this, Node::NodeType::Folder);
	for (auto& a : config.bmsDirectories())
	{
		auto path = fs::path(a);
		ret->add_children(Node::make(*this, Node::NodeType::SongFolder, L"BMS", path.filename(), L"", path.generic_wstring()));
	}
	// TODO: カスタムフォルダを再帰的に追加
	return std::move(ret);
}

std::vector<SongManager::SongMetadata> SongManager::get(std::wstring condition, std::function<void(std::shared_ptr<SQLite::Statement>)> binder)
{
	std::vector<SongMetadata> ret;

	if (condition.empty())
		condition = L"1";
	auto stmt = SQLite::Statement::make(_pdb, L"SELECT * FROM songs NATURAL LEFT JOIN scores WHERE " + condition);
	binder(stmt);
	while (stmt->step() == SQLITE_ROW)
	{
		SongMetadata m;
		stmt->column(0, m.hash);
		stmt->column(1, m.title);
		stmt->column(2, m.subtitle);
		stmt->column(3, m.artist);
		stmt->column(4, m.subartist);
		stmt->column(5, m.genre);
		stmt->column(6, m.folder);
		stmt->column(7, m.filename);
		stmt->column(8, m.stageFile);
		stmt->column(9, m.banner);
		stmt->column(10, m.backbmp);
		stmt->column(11, m.minBpm);
		stmt->column(12, m.maxBpm);
		stmt->column(13, m.player);
		stmt->column(14, m.judgeRank);
		stmt->column(15, m.playLevel);
		stmt->column(16, m.difficulty);
		stmt->column(17, m.totalnotes);
		stmt->column(18, m.has_ln);
		stmt->column(19, m.has_random);

		stmt->column(20, m.clearlamp);
		stmt->column(21, m.maxcombo);
		stmt->column(22, m.misscount);
		stmt->column(23, m.playcount);
		stmt->column(24, m.rate);
		ret.push_back(m);
	}
	
	return std::move(ret);
}

int SongManager::clearLamp(std::wstring condition, std::function<void(std::shared_ptr<SQLite::Statement>)> binder)
{
	if (condition.empty())
		condition = L"1";
	auto stmt = SQLite::Statement::make(_pdb, L"SELECT MIN(NVL(clearlamp, 0)) FROM songs NATURAL LEFT JOIN scores WHERE " + condition);
	binder(stmt);
	if (stmt->step() == SQLITE_ROW)
	{
		int ret;
		stmt->column(0, ret);
		return ret;
	}
	return 0;
}

#pragma region Node

SongManager::Node::Node(SongManager& manager, SongManager::Node::NodeType type, std::wstring category, std::wstring title, std::wstring description, std::wstring command) : _manager(manager)
{
	_type = type;
	_category = category;
	_title = title;
	_description = description;
	_command = command;
	if (type == NodeType::Folder)
		_children = decltype(_children)::value_type();
}

SongManager::Node::Node(SongManager& manager, SongManager::SongMetadata metadata) : _manager(manager)
{
	_type = NodeType::Song;
	_song = metadata;
}

std::shared_ptr<SongManager::Node> SongManager::Node::make(SongManager& manager, SongManager::Node::NodeType type, std::wstring category, std::wstring title, std::wstring description, std::wstring command)
{
	return std::shared_ptr<Node>(new Node(manager, type, category, title, description, command));
}

std::shared_ptr<SongManager::Node> SongManager::Node::make(SongManager& manager, SongManager::SongMetadata metadata)
{
	return std::shared_ptr<Node>(new Node(manager, metadata));
}

std::wstring SongManager::Node::title()
{
	if (type() == NodeType::Song)
	{
		if (_song.subtitle.empty()) return _song.title;
		return _song.title + L" " + _song.subtitle;
	}
	return _title;
}

int SongManager::Node::clearLamp()
{
	if (type() == NodeType::Song) return _song.clearlamp;
	if (!_clearLamp)
	{
		if (type() == NodeType::SongFolder)
			_clearLamp = _manager.clearLamp(L"folder LIKE ? || '%'", [this](auto& a) { a->bind(_command); });
		else if (type() == NodeType::CustomFolder)
			_clearLamp = _manager.clearLamp(_command);
		else if (_children)
		{
			auto l = INT_MAX;
			for (auto& a : *_children)
				l = min(a->clearLamp(), l);
			_clearLamp = l;
		}
		else
			_clearLamp = 0;
	}
	return *_clearLamp;
}

std::vector<std::shared_ptr<SongManager::Node>> SongManager::Node::children()
{
	if (type() == NodeType::Song)
		throw std::domain_error("Node error");

	if (!_children)
	{
		if (type() == NodeType::SongFolder)
		{
			auto v = _manager.get(L"folder LIKE ? || '%'", [this](auto& a) { a->bind(_command); });
			std::vector<std::shared_ptr<Node>> n;
			std::transform(v.begin(), v.end(), std::back_inserter(n), [this](auto& a)
			{
				return Node::make(_manager, a);
			});
			_children = std::move(n);
		}
		else if (type() == NodeType::CustomFolder)
		{
			auto v = _manager.get(_command);
			std::vector<std::shared_ptr<Node>> n;
			std::transform(v.begin(), v.end(), std::back_inserter(n), [this](auto& a)
			{
				return Node::make(_manager, a);
			});
			_children = std::move(n);
		}
	}
	return *_children;
}

void SongManager::Node::add_children(std::shared_ptr<SongManager::Node> n)
{
	if (type() != NodeType::Folder)
		throw std::domain_error("Node error");

	_children->push_back(n);
}

std::wstring SongManager::Node::file_path(const std::wstring& path)
{
	if (type() != NodeType::Song)
		throw std::domain_error("Node error");

	return std::move((fs::path(_song.folder) / path).generic_wstring());
}

#pragma endregion
