#pragma once
#include "../interface.h"
#include <string>
#include <iostream>

struct ImplA : Interface {
  ImplA() : name_() {}
  ImplA(std::string name) : name_(std::move(name)) {}
  virtual void print() const override;
  virtual void serialize(std::ostream &out) const override;
  virtual void deserialize(std::istream &in) override;
  virtual int get_id() const override;
  std::string name_;
};