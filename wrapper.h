#pragma once
#include "interface.h"
#include <fstream>
#include <memory>

struct InterfaceWrapper {// RAII
  InterfaceWrapper() : p_obj_() {}
  InterfaceWrapper(std::shared_ptr<Interface> p_obj) : p_obj_(p_obj) {}
  void serialize(std::ostream &out) const;
  void deserialize(std::istream &in);
  Interface *operator->() const { return p_obj_.operator->(); }
  Interface &operator*() const { return p_obj_.operator*(); }
  Interface *operator->() { return p_obj_.operator->(); }
  Interface &operator*() { return p_obj_.operator*(); }
  std::shared_ptr<Interface> p_obj_;
};