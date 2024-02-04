#include "impl_b.h"
#include "../reflection.h"

void ImplB::print() const {
  std::cout << "my age is : " << age_ << std::endl;
}

void ImplB::serialize(std::ostream &out) const {
  out << age_ << std::endl;
}

void ImplB::deserialize(std::istream &in) {
  in >> age_;
}

int ImplB::get_id() const {
  return ReflectType2IDMap<std::decay<decltype(*this)>::type>::id;
}