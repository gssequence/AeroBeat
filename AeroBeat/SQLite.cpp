#include "stdafx.h"
#include "SQLite.h"

SQLite::SQLite(std::wstring path)
{
	if (sqlite3_open16(path.c_str(), &pdb) != SQLITE_OK)
		return;

	_fail = false;
}

SQLite::~SQLite()
{
	if (!_fail)
		sqlite3_close(pdb);
}

std::shared_ptr<SQLite> SQLite::open(std::wstring path)
{
	return std::shared_ptr<SQLite>(new SQLite(path));
}

#pragma region Statement

SQLite::Statement::Statement(std::shared_ptr<SQLite> db, std::wstring sql)
{
	pdb = db;
	_fail = sqlite3_prepare16(pdb->pdb, sql.c_str(), -1, &ps, nullptr) != SQLITE_OK || !ps;
}

SQLite::Statement::~Statement()
{
	sqlite3_finalize(ps);
}

std::shared_ptr<SQLite::Statement> SQLite::Statement::make(std::shared_ptr<SQLite> pdb, std::wstring sql)
{
	return std::shared_ptr<Statement>(new Statement(pdb, sql));
}

int SQLite::Statement::bind()
{
	return sqlite3_bind_null(ps, n++);
}

int SQLite::Statement::bind(int i)
{
	return sqlite3_bind_int(ps, n++, i);
}

int SQLite::Statement::bind(long long l)
{
	return sqlite3_bind_int64(ps, n++, l);
}

int SQLite::Statement::bind(double d)
{
	return sqlite3_bind_double(ps, n++, d);
}

int SQLite::Statement::bind(std::wstring str)
{
	return sqlite3_bind_text16(ps, n++, str.c_str(), str.size() * sizeof(wchar_t), SQLITE_TRANSIENT);
}

int SQLite::Statement::bind(const void* ptr, size_t size)
{
	return sqlite3_bind_blob(ps, n++, ptr, size, SQLITE_TRANSIENT);
}

void SQLite::Statement::column(int i, int& dst)
{
	dst = sqlite3_column_int(ps, i);
}

void SQLite::Statement::column(int i, long long& dst)
{
	dst = sqlite3_column_int64(ps, i);
}

void SQLite::Statement::column(int i, double& dst)
{
	dst = sqlite3_column_double(ps, i);
}

void SQLite::Statement::column(int i, std::wstring& dst)
{
	dst = std::move(std::wstring((const wchar_t*)sqlite3_column_text16(ps, i)));
}

void SQLite::Statement::column(int i, const void*& dst)
{
	dst = sqlite3_column_blob(ps, i);
}

void SQLite::Statement::column(int i, bool& dst)
{
	dst = sqlite3_column_int(ps, i) != 0;
}

int SQLite::Statement::bytes(int i)
{
	return sqlite3_column_bytes(ps, i);
}

int SQLite::Statement::step()
{
	return sqlite3_step(ps);
}

int SQLite::Statement::reset()
{
	n = 1;
	return sqlite3_reset(ps);
}

#pragma endregion
