#pragma once

class SQLite
{
public:
	class Statement
	{
	private:
		bool _fail = true;
		std::shared_ptr<SQLite> pdb;
		sqlite3_stmt* ps = nullptr;
		int n = 1;

		Statement(std::shared_ptr<SQLite> db, std::wstring sql);

	public:
		virtual ~Statement();

		static std::shared_ptr<Statement> make(std::shared_ptr<SQLite> pdb, std::wstring sql);

		int bind();
		int bind(int i);
		int bind(long long l);
		int bind(double d);
		int bind(std::wstring str);
		int bind(const void* ptr, size_t size);
		void column(int i, int& dst);
		void column(int i, long long& dst);
		void column(int i, double& dst);
		void column(int i, std::wstring& dst);
		void column(int i, const void*& dst);
		void column(int i, bool& dst);
		int bytes(int i);
		int step();
		int reset();

		auto fail() { return _fail; }
	};

private:
	bool _fail = true;
	sqlite3* pdb = nullptr;
	SQLite(std::wstring path);

public:
	friend Statement;

	virtual ~SQLite();

	static std::shared_ptr<SQLite> open(std::wstring path);

	auto fail() { return _fail; }
};
