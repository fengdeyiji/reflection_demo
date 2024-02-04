// 如果需要看到数据定义的话
#ifdef NEDD_STRUCTURE_DEFINE
  #include "impl/impl_a.h"
  #include "impl/impl_b.h"
#endif

// 采用“X宏”技术对代码进行二次展开，在不同文件中展开为不同的代码
#define REGISTER(CLASS, ID) __REGISTER__(CLASS, ID)
#ifdef NEED_GENERATE_CODE
  REGISTER(ImplA, 1);
  REGISTER(ImplB, 2);
#endif
#undef REGISTER