#pragma once
#include <sqlite3.h>
#include <string>
#include <functional>
#include <list>
#include <vector>
#include "../threading/guardedvar.hpp"

namespace FirnLibs
{
  class SQLite
  {
  public:
    enum Error
    {
      None,
      NotConnected,
      HasDatabase,
      APIError,
      ParameterCountMismatch,
    };


    SQLite();
    SQLite(const std::string &dbFile);
    ~SQLite();

    void Cleanup();

    int GetAPIError() { return lastDBError; }

    Error LoadDB(const std::string &dbFile);

    Error UnpreparedExecute(const std::string &statement, const std::function<int (int argc, char **argv, char **argv2)> &callback);



    class Prepvar
    {
      // Variable holders for prepared statement variables.
      // Variables are copied when the holder is constructed.
      // Variables will be destroyed either when the Prepvar is destroyed or when the sqlite code is done with it.
      // Variables cannot be copied, and they are single use only.
    public:
      Prepvar(const std::vector<unsigned char> &data);
      Prepvar(const double &data);
      Prepvar(const int &data);
      Prepvar(const int64_t &data);
      Prepvar();
      Prepvar(const std::string &data);

      Prepvar &operator=(const Prepvar &) = delete;
    protected:
      Prepvar(const Prepvar &other);
      ~Prepvar();
      void * data;
      int size;
      enum Type
      {
        Blob,
        Double,
        Int,
        Int64,
        Null,
        Text,
      };
      Type type;
      friend SQLite;
    };
    // Prepare a statement and return the identifier for the prepared statement.  -1 means error.
    sqlite3_stmt *Prepare(const std::string &statementStr);
    void Unprepare(sqlite3_stmt *statement);
    Error PreparedExecute(sqlite3_stmt *statement, std::vector<Prepvar> &vars, 
                                  const std::function<void (const std::vector<Prepvar> &vals, const std::vector<std::string> &columnNames)> &callback);


    
  protected:
    int statementCount;
    FirnLibs::Threading::GuardedVar<std::list<sqlite3_stmt *> > statementMap;

  protected:
    std::string dbFile;
    sqlite3 *db;
    int lastDBError;
    std::string lastDBErrorStr;


    

    
    // Don't split the connection.
  public:
    SQLite(const SQLite &) = delete;
    void operator=(const SQLite &) = delete;
  };
}
