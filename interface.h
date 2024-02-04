#pragma once
#include <fstream>

struct Interface {
  virtual ~Interface() {};
  virtual void print() const = 0;
  virtual void serialize(std::ostream &out) const = 0;
  virtual void deserialize(std::istream &in) = 0;
  virtual int get_id() const = 0;
};