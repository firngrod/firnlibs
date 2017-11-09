#include "sqlite.hpp"
#include <algorithm>
#include <iostream>

namespace FirnLibs {


SQLite::SQLite()
{
  db = nullptr;
  lastDBError = 0;
}


SQLite::SQLite(const std::string &dbFile)
{
  db = nullptr;
  lastDBError = SQLITE_OK;
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


std::string SQLite::DBFileName() const
{
  return dbFile;
}


int UnpreparedExecuteCallback(void * stdFuncCallback, int argc, char **argv, char **argv2)
{
  auto *fncPtr = (std::function<int (int argc, char **argv, char **argv2)> *) stdFuncCallback;
  if(*fncPtr != nullptr)
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


SQLite::Error SQLite::PreparedExecute(sqlite3_stmt *statement, const std::vector<Prepvar> &vars, 
                                      const std::function<void (const std::vector<Prepvar> &vals, const std::vector<std::string> &columnNames)> &callback)
{
  // Sanity check.
  if(vars.size() != sqlite3_bind_parameter_count(statement))
  {
    return Error::ParameterCountMismatch;
  }

  if(db == nullptr)
  {
    return Error::NotConnected;
  }

  if(statement == nullptr)
  {
    return NullStatement;
  }

  // Get the result column names.
  std::vector<std::string> colNames;
  int colCount = sqlite3_column_count(statement);
  for(int i = 0; i < colCount; i++)
  {
    colNames.push_back(sqlite3_column_name(statement, i));
    std::cout << "Column name: " << colNames.back() << std::endl;
  }

  // Bind the variables.
  for(int i = 0, s = 1; i < vars.size(); i++, s++)
  {
    if(vars[i].type == Prepvar::Type::Blob)
    {
      sqlite3_bind_blob(statement, s, (const void *)vars[i].data, vars[i].size, SQLITE_TRANSIENT);
    }
    if(vars[i].type == Prepvar::Type::Double)
    {
      sqlite3_bind_double(statement, s, *(const double *)vars[i].data);
    }
    if(vars[i].type == Prepvar::Type::Int)
    {
      sqlite3_bind_int64(statement, s, *(const int64_t *)vars[i].data);
    }
    if(vars[i].type == Prepvar::Type::Null)
    {
      sqlite3_bind_null(statement, s);
    }
    if(vars[i].type == Prepvar::Type::Text)
    {
      sqlite3_bind_text(statement, s, (const char *)vars[i].data, -1, SQLITE_TRANSIENT);
    }
  }

  int status;
  std::vector<Prepvar> rowContent;
  do
  {
    status = sqlite3_step(statement);
    std::cout << "Stepped.  Status: " << status << std::endl;
    if(status == SQLITE_ROW)
    {
      // Get the row data.
      int iii = 0;
      for(int i = 0; i < colCount ; i++)
      {
        if(iii >= 0)
          iii++;
        int colType = sqlite3_column_type(statement, i);
        if(colType == SQLITE_BLOB)
        {
          const void *data = sqlite3_column_blob(statement, i);
          int size = sqlite3_column_bytes(statement, i);
          rowContent.push_back(Prepvar(data, size));
        }
        else if(colType == SQLITE_FLOAT)
        {
          rowContent.push_back(Prepvar(sqlite3_column_double(statement, i)));
        }
        else if(colType == SQLITE_INTEGER)
        {
          rowContent.push_back(Prepvar((int64_t)sqlite3_column_int64(statement, i)));
        }
        else if(colType == SQLITE_TEXT)
        {
          rowContent.push_back(Prepvar((const char *)sqlite3_column_text(statement, i)));
        }
        else
        {
          rowContent.push_back(Prepvar());
        }
      }
      // Do the callback on the row data.
      if(callback != nullptr)
        callback(rowContent, colNames);
      // Prepare for the next round
      rowContent.clear();
    }
  } while(status == SQLITE_ROW);

  // Alright, we are done.  Reset the statement to get the error code if any.
  lastDBError = sqlite3_reset(statement);

  if(lastDBError != SQLITE_OK)
  {
    return Error::APIError;
  }

  return Error::None;
}


}
