#pragma once

template <int ID>
struct ReflectID2TypeMap {};

template <typename T>
struct ReflectType2IDMap {};

#define NEDD_STRUCTURE_DEFINE
#define NEED_GENERATE_CODE
#define __REGISTER__(CLASS, ID) \
template <>\
struct ReflectID2TypeMap<ID> {\
  using type = CLASS;\
};\
template <>\
struct ReflectType2IDMap<CLASS> {\
  static constexpr int id = ID;\
}
#include "register.h"
#undef __REGISTER__
#undef NEED_GENERATE_CODE
#undef NEDD_STRUCTURE_DEFINE