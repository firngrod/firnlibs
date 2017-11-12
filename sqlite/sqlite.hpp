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
      InvalidType,
      NullStatement,
    };


    SQLite();
    SQLite(const std::string &dbFile);
    ~SQLite();

    std::string DBFileName() const;

    void Cleanup();

    int GetAPIError() const { return lastDBError; }
    std::string GetAPIErrorStr() const { return lastDBErrorStr; }

    Error LoadDB(const std::string &dbFile);

    // Quick and dirty executes.  Do NOT use user input for this.
    Error UnpreparedExecute(const std::string &statement, const std::function<int (int argc, char **argv, char **argv2)> &callback);


    // Prepared executes.

    // First a variable wrapper for all the variable types I have chosen to support.
    class Prepvar
    {
      // Variable holders for prepared statement variables.
      // Variables are copied when the holder is constructed.
    public:
      Prepvar(const std::vector<unsigned char> &data);
      Prepvar(const void *data, const int &size);
      SQLite::Error GetValue(std::vector<unsigned char> &data) const;

      Prepvar(const double &data);
      SQLite::Error GetValue(double &data) const;

      Prepvar(const int64_t &data);
      SQLite::Error GetValue(int64_t &data) const;

      Prepvar(const std::string &data);
      SQLite::Error GetValue(std::string &data) const;

      Prepvar();

      Prepvar &operator=(const Prepvar &);
      Prepvar(const Prepvar &other);

      void Cleanup();
      ~Prepvar();
    protected:
      void FromData(const void *data, const int &size);
      void FromData(const double &data);
      void FromData(const int64_t &data);
      void FromData(const char *data);
      void FromData();

      void CopyFromOther(const Prepvar &other);
      void * data;
      int size;
      enum Type
      {
        Blob,
        Double,
        Int,
        Text,
        Null,
      };
      Type type;
      friend SQLite;
    };
    // Prepare a statement and return the identifier for the prepared statement.  -1 means error.
    sqlite3_stmt *Prepare(const std::string &statementStr);
    void Unprepare(sqlite3_stmt *statement);

    typedef std::vector<Prepvar> PrepVector;
    // Execute a prepared statement.  It will call the callback function for each row returned by the execute.
    typedef std::function<void (const std::vector<Prepvar> &vals, const std::vector<std::string> &columnNames)> PrepCallback;
    Error PreparedExecute(sqlite3_stmt *statement, const std::vector<Prepvar> &vars, const PrepCallback &callback);


    
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
