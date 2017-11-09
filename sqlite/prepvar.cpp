#include "sqlite.hpp"
#include <cstring>

namespace FirnLibs{

SQLite::Prepvar::Prepvar(const std::vector<unsigned char> &data)
{
  FromData((const void *)&data[0], size);
}


SQLite::Prepvar::Prepvar(const void *data, const int &size)
{
  FromData(data, size);
}


void SQLite::Prepvar::FromData(const void *data, const int &size)
{
  this->data = new unsigned char[size];
  std::memcpy(this->data, data, size);
  this->size = size;
  type = Type::Blob;
}


SQLite::Error SQLite::Prepvar::GetValue(std::vector<unsigned char> &data) const
{
  if(type != Type::Blob)
    return SQLite::Error::InvalidType;

  data.clear();
  data.insert(data.begin(), (unsigned char *)this->data, (unsigned char *)this->data + size);
  return SQLite::Error::None;
}


SQLite::Prepvar::Prepvar(const double &data)
{
  FromData(data);
}


void SQLite::Prepvar::FromData(const double &data)
{
  this->data = (void *) new auto(data);
  type = Type::Double;
}


SQLite::Error SQLite::Prepvar::GetValue(double &data) const
{
  if(type != Type::Double)
    return SQLite::Error::InvalidType;

  data = *(double *)this->data;
  return SQLite::Error::None;
}


SQLite::Prepvar::Prepvar(const int64_t &data)
{
  FromData(data);
}


void SQLite::Prepvar::FromData(const int64_t &data)
{
  this->data = (void *) new auto(data);
  type = Type::Int;
}


SQLite::Error SQLite::Prepvar::GetValue(int64_t &data) const
{
  if(type != Type::Int)
    return SQLite::Error::InvalidType;

  data = *(int64_t *)this->data;
  return SQLite::Error::None;
}


SQLite::Prepvar::Prepvar(const std::string &data)
{
  FromData(data.c_str());
}


void SQLite::Prepvar::FromData(const char *data)
{
  this->data = new unsigned char[std::strlen(data) + 1];
  std::strcpy((char *)this->data, &data[0]);
  ((char *)this->data)[strlen(data)] = '\0';
  type = Type::Text;
}


SQLite::Error SQLite::Prepvar::GetValue(std::string &data) const
{
  if(type != Type::Text)
    return SQLite::Error::InvalidType;

  data = (char *)this->data;
  return SQLite::Error::None;
}


SQLite::Prepvar::Prepvar()
{
  FromData();
}


void SQLite::Prepvar::FromData()
{
  type = Type::Null;
}


SQLite::Prepvar::Prepvar(const Prepvar &other)
{
  CopyFromOther(other);
}


void SQLite::Prepvar::CopyFromOther(const Prepvar &other)
{
  Cleanup();
  if(other.type == Type::Blob)
    FromData(other.data, other.size);
  else if(other.type == Type::Double)
    FromData(*(double *)other.data);
  else if(other.type == Type::Int)
    FromData(*(int64_t *)other.data);
  else if(other.type == Type::Text)
    FromData((char *)other.data);
  else if(other.type == Type::Null)
    FromData();
}

void SQLite::Prepvar::Cleanup()
{
  if(type == Type::Null)
    return;
  else if(type == Type::Blob)
    delete [] (unsigned char *)data;
  else if(type == Type::Double)
    delete (double *)data;
  else if(type == Type::Int)
    delete (int64_t *)data;
  else if(type == Type::Text)
    delete [](unsigned char *)data;

  size = 0;
  data = nullptr;
  type = Type::Null;
}


SQLite::Prepvar &SQLite::Prepvar::operator=(const Prepvar &other)
{
  CopyFromOther(other);
}


SQLite::Prepvar::~Prepvar()
{
  Cleanup();
}


}
