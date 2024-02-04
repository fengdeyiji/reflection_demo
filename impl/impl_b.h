#pragma once
#include "../interface.h"
#include <string>
#include <iostream>

struct ImplB : Interface {
  ImplB() : age_(0) {}
  ImplB(int age) : age_(age) {}
  virtual void print() const override;
  virtual void serialize(std::ostream &out) const override;
  virtual void deserialize(std::istream &in) override;
  virtual int get_id() const override;
  int age_;
};