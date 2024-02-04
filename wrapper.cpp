#include "wrapper.h"
#include <utility>
#define NEDD_STRUCTURE_DEFINE
#include "register.h"
#undef NEDD_STRUCTURE_DEFINE

void InterfaceWrapper::serialize(std::ostream &out) const {
  out << p_obj_->get_id() << std::endl;
  p_obj_->serialize(out);
}

void InterfaceWrapper::deserialize(std::istream &in) {
  int id = 0;
  in >> id;
  switch (id) {
    #define NEED_GENERATE_CODE
    #define __REGISTER__(TYPE, ID) \
    case ID:\
      p_obj_ = std::make_shared<TYPE>();\
      break;
    #include "register.h"
    #undef __REGISTER__
    #undef NEED_GENERATE_CODE
    default:
      abort();
      break;
  }
  p_obj_->deserialize(in);
}