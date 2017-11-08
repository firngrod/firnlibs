#include "sqlite.hpp"
#include <algorithm>

namespace FirnLibs {


SQLite::SQLite()
{
  db = nullptr;
  lastDBError = 0;
  statementCount = 0;
}


SQLite::SQLite(const std::string &dbFile)
{
  db = nullptr;
  lastDBError = SQLITE_OK;
  statementCount = 0;
  LoadDB(dbFile);
}


SQLite::~SQLite()
{
  Cleanup();
}


void SQLite::Cleanup()
{
  {
    auto tok = statementMap.Get();
    while(tok->size() > 0)
    {
      Unprepare(tok->front());
    }
  }
  if(db != nullptr)
    sqlite3_close(db);
  dbFile = "";
  lastDBError = SQLITE_OK;
}


SQLite::Error SQLite::LoadDB(const std::string &dbFile)
{
  if(db != nullptr)
    return Error::HasDatabase;
  
  lastDBError = sqlite3_open(dbFile.c_str(), &db);

  if(lastDBError != SQLITE_OK)
    return Error::APIError;

  this->dbFile = dbFile;
  return Error::None;
}


int UnpreparedExecuteCallback(void * stdFuncCallback, int argc, char **argv, char **argv2)
{
  auto *fncPtr = (std::function<int (int argc, char **argv, char **argv2)> *) stdFuncCallback;
  (*fncPtr)(argc, argv, argv2);
  delete fncPtr;
}


SQLite::Error SQLite::UnpreparedExecute(const std::string &statement, const std::function<int (int argc, char **argv, char **argv2)> &callback)
{
  if(db == nullptr)
    return Error::NotConnected;

  char *errorMsg = nullptr;
  void * callbackPtr = (void *) new std::function<int (int argc, char **argv, char **argv2)>(callback);
  lastDBError = sqlite3_exec(db, statement.c_str(), UnpreparedExecuteCallback, callbackPtr, &errorMsg);
  if(lastDBError != SQLITE_OK)
  {
    lastDBErrorStr = errorMsg;
    sqlite3_free(errorMsg);
    return Error::APIError;
  }

  return Error::None;
}


sqlite3_stmt *SQLite::Prepare(const std::string &statementStr)
{
  sqlite3_stmt * prepped = nullptr;
  const char * dummy;
  lastDBError = sqlite3_prepare_v2(db, statementStr.c_str(), statementStr.size(), &prepped, &dummy);
  if(prepped != nullptr)
  {
    auto tok = statementMap.Get();
    tok->push_back(prepped);
  }
  return prepped;
}


void SQLite::Unprepare(sqlite3_stmt *statement)
{
  auto tok = statementMap.Get();
  auto itr = std::find(tok->begin(), tok->end(), statement);
  if(itr == tok->end())
    return;
  sqlite3_finalize(*itr);
  tok->erase(itr);
}


void PreparedExecuteCleanup(void * tmpie)
{
  delete [] (unsigned char *)tmpie;
}


SQLite::Error SQLite::PreparedExecute(sqlite3_stmt *statement, std::vector<Prepvar> &vars, 
                                      const std::function<void (const std::vector<Prepvar> &vals, const std::vector<std::string> &columnNames)> &callback)
{
  // Sanity check.
  if(vars.size() != sqlite3_bind_parameter_count(statement))
  {
    return Error::ParameterCountMismatch;
  }

  // Get the result column names.
  std::vector<std::string> colNames;
  int colCount = sqlite3_column_count(statement);
  for(int i = 0; i < colCount; i++)
  {
    colNames.push_back(sqlite3_column_name(statement, i));
  }

  // Bind the variables.
  for(int i = 1; i <= vars.size(); i++)
  {
    if(vars[i].type == Prepvar::Type::Blob)
    {
      sqlite3_bind_blob(statement, i, (const void *)vars[i].data, vars[i].size, PreparedExecuteCleanup);
      vars[i].type = Prepvar::Type::Null;
    }
    if(vars[i].type == Prepvar::Type::Double)
    {
      sqlite3_bind_double(statement, i, *(const double *)vars[i].data);
      delete (double *)vars[i].data;
    }
    if(vars[i].type == Prepvar::Type::Int)
    {
      sqlite3_bind_int(statement, i, *(const int *)vars[i].data);
      delete (int *)vars[i].data;
    }
    if(vars[i].type == Prepvar::Type::Int64)
    {
      sqlite3_bind_int64(statement, i, *(const int64_t *)vars[i].data);
      delete (int64_t *)vars[i].data;
    }
    if(vars[i].type == Prepvar::Type::Null)
    {
      sqlite3_bind_null(statement, i);
    }
    if(vars[i].type == Prepvar::Type::Text)
    {
      sqlite3_bind_blob(statement, i, (const char *)vars[i].data, -1, PreparedExecuteCleanup);
      vars[i].type = Prepvar::Type::Null;
    }
  }

  int status;
  do
  {
    status = sqlite3_step(statement);
    if(status == SQLITE_ROW)
    {
      
    }
      
  } while(status == SQLITE_ROW);

  return Error::None;
}


}
