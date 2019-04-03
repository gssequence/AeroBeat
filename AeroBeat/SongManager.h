#pragma once

#include "Config.h"
#include "SQLite.h"

class SongManager
{
public:
	struct SongMetadata
	{
		std::wstring hash;
		std::wstring title;
		std::wstring subtitle;
		std::wstring artist;
		std::wstring subartist;
		std::wstring genre;
		std::wstring folder;
		std::wstring filename;
		std::wstring stageFile;
		std::wstring banner;
		std::wstring backbmp;
		double minBpm;
		double maxBpm;
		int player;
		int judgeRank;
		int playLevel;
		int difficulty;
		int totalnotes;
		bool has_ln;
		bool has_random;

		int clearlamp;
		int maxcombo;
		int misscount;
		int playcount;
		double rate;
	};

	class Node
	{
	public:
		enum NodeType
		{
			Song,
			SongFolder,
			CustomFolder,
			Folder,
		};

	private:
		SongManager& _manager;
		NodeType _type;
		SongMetadata _song;
		boost::optional<std::vector<std::shared_ptr<Node>>> _children;
		std::wstring _category, _title, _description, _command;
		boost::optional<int> _clearLamp;

		Node(SongManager& manager, NodeType type, std::wstring category = L"", std::wstring title = L"", std::wstring description = L"", std::wstring command = L"");
		Node(SongManager& manager, SongMetadata metadata);

	public:
		virtual ~Node() { }

		std::wstring title();
		int clearLamp();
		std::vector<std::shared_ptr<Node>> children();
		void add_children(std::shared_ptr<Node> n);
		std::wstring file_path(const std::wstring& path);

		static std::shared_ptr<Node> make(SongManager& manager, NodeType type, std::wstring category = L"", std::wstring title = L"", std::wstring description = L"", std::wstring command = L"");
		static std::shared_ptr<Node> make(SongManager& manager, SongMetadata metadata);

		auto type() { return _type; }
		const auto& song() { return _song; }
	};

private:
	std::shared_ptr<SQLite> _pdb;

	int clearLamp(std::wstring condition, std::function<void(std::shared_ptr<SQLite::Statement>)> binder = [](auto& _) { });
	std::vector<SongMetadata> get(std::wstring condition, std::function<void(std::shared_ptr<SQLite::Statement>)> binder = [](auto& _) {});

public:
	SongManager();
	virtual ~SongManager() { }

	void initialize(Config& config, std::function<void(std::wstring)> logger);
	std::shared_ptr<Node> root(Config& config);
};
