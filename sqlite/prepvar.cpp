#include "sqlite.hpp"
#include <cstring>

namespace FirnLibs{

SQLite::Prepvar::Prepvar(const std::vector<unsigned char> &data)
{
  this->data = new unsigned char[data.size()];
  std::memcpy(this->data, &data[0], data.size());
  this->size = data.size();
  type = Type::Blob;
}


SQLite::Prepvar::Prepvar(const double &data)
{
  this->data = (void *) new auto(data);
  type = Type::Double;
}


SQLite::Prepvar::Prepvar(const int &data)
{
  this->data = (void *) new auto(data);
  type = Type::Int;
}


SQLite::Prepvar::Prepvar(const int64_t &data)
{
  this->data = (void *) new auto(data);
  type = Type::Int64;
}


SQLite::Prepvar::Prepvar()
{
  type = Type::Null;
}


SQLite::Prepvar::Prepvar(const std::string &data)
{
  this->data = new unsigned char[data.size() + 1];
  std::strcpy((char *)this->data, &data[0]);
  type = Type::Text;
}


SQLite::Prepvar::~Prepvar()
{
  if(type == Type::Null)
    return;
  else if(type == Type::Blob)
    delete [] (unsigned char *)data;
  else if(type == Type::Double)
    delete (double *)data;
  else if(type == Type::Int)
    delete (int *)data;
  else if(type == Type::Int64)
    delete (int64_t *)data;
  else if(type == Type::Text)
    delete [](unsigned char *)data;
}


}
