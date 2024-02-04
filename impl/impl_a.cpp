#include "impl_a.h"
#include "../reflection.h"

void ImplA::print() const {
  std::cout << "my name is : " << name_ << std::endl;
  std::cout << "name ptr address : " << std::hex << (void *)name_.data() << std::dec << std::endl;
}

void ImplA::serialize(std::ostream &out) const {
  out << name_ << std::endl;
}

void ImplA::deserialize(std::istream &in) {
  in >> name_;
}

int ImplA::get_id() const {
  return ReflectType2IDMap<std::decay<decltype(*this)>::type>::id;
}