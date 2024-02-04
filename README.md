# 背景
面向对象编程中的“多态”技术对于代码抽象至关重要，它是面向对象思想所衍生出的一系列设计模式（SOLID）的技术基础。  
SOLID原则对于代码已有功能的维护和未来可能功能的扩展提供了扎实的根基，对于不断迭代发展的大型软件工程来讲，如果没有在设计开发初期就按照SOLID原则打下代码根基，那么后续的开发、维护与扩展将会演变成一场每个技术人都不愿意面对的最恐怖的噩梦。  
尽管面向对象技术在大型复杂软件工程中取得了巨大的成功，然而遗憾的是，它仍然具有两点天生的制约：
1. 面向对象技术所依赖的动态多态依赖虚表指针实现，即，它要求对象是“**引用语义**”的，而非“**值语义**”的，这在非GC语言中（特别是在以**值语义**作为根本抽象哲学的C++语言中）对生命周期管理提出了挑战（但好在我们有shared_ptr这个wrapper来将一个**引用语义**对象采用**值语义**的方式进行传递并妥善管理其生命周期）。
2. 面向对象技术的动态多态由于依赖指针实现，因此它只能工作于**单进程**内部，然而如今跨存储介质（例如内存<->磁盘）或者跨进程通信几乎是大型复杂软件工程的必然组成部分，如何在**多进程**以及**跨介质**条件下仍然保持其多态特性是大型复杂软件功能所要面临的新抽象挑战。（如果你的团队从来没有特别关注过这个问题，但项目仍然运转良好——恭喜你有一群肯卖命填坑的工程师，但我敢说总有一天这是拿命也填不满的坑）

# 最佳实践
本demo试图描述一种在C++语言中实现**跨进程**或者**跨介质**多态抽象的通用思想。  
这种思想并非单一技术的作用结果，而是在C++语言中的多种技术的综合运用。  
首先我将向你展示这种技术所取得的抽象效果：
```c++
int main() {
  vector<InterfaceWrapper> array_before, array_after;
  // ImplA与ImplB是不同的对象，得益于运行时多态技术，它们可以被放置于同一容器中管理
  // 得益于share_ptr所建立的抽象，在享受引用语义的多态效果的同时，还可以享受值语义的对象生命周期管理
  array_before.emplace_back(make_shared<ImplA>("Alice"));
  array_before.emplace_back(make_shared<ImplB>(12));
  // 得益于Wrapper类的抽象，即便array跨介质/进程后，仍然可以保持多态抽象
  {
    cout << "before serialize:" << endl << array_before << endl;
    std::ofstream output_file("1.txt");
    output_file << array_before;
  }
  {
    std::ifstream input_file("1.txt");
    input_file >> array_after;
    cout << "after deserialize:" << endl << array_after << endl;
  }
  return 0;
}
```
```bash
❯ g++ wrapper.cpp impl/impl_a.cpp impl/impl_b.cpp main.cpp -std=c++11
❯ ./a.out
before serialize:
my name is : Alice
name ptr address : 0xaaaae91aeed8
my age is : 12

after deserialize:
my name is : Alice
name ptr address : 0xaaaae91b15c8
my age is : 12
```
接下来我将为你讲解，这背后究竟发生了什么。

# 原理解析
在开始之前，让我们首先观察代码的目录结构及文件组织形式：
```
.
|-- impl #这是未来的扩展类，可能加入其他的实现：impl_c/d/e/f/g...
|   |-- impl_a.cpp
|   |-- impl_a.h
|   |-- impl_b.cpp
|   `-- impl_b.h
|-- interface.h #这是抽象基类的接口定义，它将用于实现对象的运行时类型擦除与统一
|-- main.cpp #这是程序入口以及用户使用这些类的示例
|-- reflection.h #这是用于编译期类型反射的中间文件
|-- register.h #这是类型注册的入口
|-- wrapper.cpp #这是跨介质多态魔法的实现
`-- wrapper.h #这是跨介质多态魔法的声明
```

## 第一步，观察数据的抽象基类接口
```c++
struct Interface {
  virtual ~Interface() {};
  virtual void print() const = 0;
  virtual void serialize(std::ostream &out) const = 0;
  virtual void deserialize(std::istream &in) = 0;
  virtual int get_id() const = 0;
};
```
首先，我们假设抽象接口Interface的作用就是根据不同的子类对象打印出不同的信息，即要求子类必须实现print()方法。  
其次，为了能够让这个对象可以序列化和反序列化，以便跨介质/进程，还要求其子类必须要实现serialize()/deserialize()方法，不难理解这是必须的。  
最后，在反序列化时，为了得知子类的类型，还要求必须提供子类类型的ID，即必须实现get_id()方法，这是为了在跨介质的另一侧实现**反射**以创建子类对象。

## 第二步，观察子类对象ImplA
```c++
// impl_a.h
struct ImplA : Interface {
  ImplA() : name_() {}
  ImplA(std::string name) : name_(std::move(name)) {}
  virtual void print() const override;
  virtual void serialize(std::ostream &out) const override;
  virtual void deserialize(std::istream &in) override;
  virtual int get_id() const override;
  std::string name_;
};

// impl_a.cpp
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
```
ImplA对象存储了一个字符串，并在打印方法和序列化方法中处理了这个字符串，这很容易理解。  
最为关键的一行代码为：`return ReflectType2IDMap<std::decay<decltype(*this)>::type>::id;`  
这里返回了本类型对应的ID值，而该值是从`reflection.h`中定义的模版类中获得的。  

## 第三步，接下来我们观察`reflection.h`
```c++
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
```
`reflection.h`首先定义了两个通用模版`ReflectID2TypeMap`以及`ReflectType2IDMap`。  
它们的作用是作`编译期反射`，可以在编译期将type转换为ID，或者将ID转换为type。  
具体ID<->Type的映射关系是通过模版特化来实现的，而所涉及到的特化类则是宏展开的结果。  
在本文件中定义了`__REGISTER__`宏，其目的就是为了在特定的类型和ID之间建立静态映射关系。  
但很显然，具体的类型和ID并不在本文件中定义，而是来自于`register.h`。

## 第四步，观察`register.h`
```c++
// 如果需要看到数据定义的话
#ifdef NEDD_STRUCTURE_DEFINE
  #include "impl/impl_a.h"
#endif

// 采用“X宏”技术对代码进行二次展开，在不同文件中展开为不同的代码
#define REGISTER(CLASS, ID) __REGISTER__(CLASS, ID)
#ifdef NEED_GENERATE_CODE
  REGISTER(ImplA, 1);
#endif
#undef REGISTER
```
`register.h`是框架的扩展接口文件，注意该文件并没有被声明`#progma once`，这是因为在同一编译单元中被`#include`多次是设计内的。（有些时候是必须的）  
`register.h`由两个宏定义块组成，采用条件编译生效。  
在第一个宏定义块`NEDD_STRUCTURE_DEFINE`中包含了需要跨介质/进程多态的类型的定义文件，这在框架的其他部分可能用到。  
在第二个宏定义块`NEED_GENERATE_CODE`中，则为对应的类型编码了其他的额外信息，在本demo中，编码了该类型所对应的全局静态ID。（当然还可以编码其他额外信息）  
额外的编码信息由`REGISTER`宏输入，但并没有被展开为有效的信息，而是被展开为了另一个`__REGISTER__`宏。  
与我们在第三步观察的结果相呼应，`__REGISTER__`宏被二次展开的结果将由框架的其他部分自定义，`register.h`文件仅提供必要的信息。（尽管我是通过独立思考得到这一整套解决方案的，但后来我发现这项技术在编译器内核代码中早就被大量应用了，并有一个专门的名称，称为“X宏”技术）  

## 第五步，实现跨介质/进程的多态Wrapper类
> David Wheeler：“任何问题在计算机科学中都可以通过另外一个间接层来解决"。
```c++
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
```
`InterfaceWrapper`类包含了一个`shared_ptr<Interface>`对象，它采用值语义+RAII的方式析构实际的Interface对象，同时，它还能享受引用语义带来的多态抽象。  
`InterfaceWrapper`表现的如同一个指针，它的取成员->操作以及解引用*操作将被转发给被包裹的实际Interface对象，就如同它是那个被包裹的引用对象一样。  
这里我们重点观察序列化方法：  
```c++
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
```
`serialize()`方法在序列化子类对象之前（通过虚函数动态绑定到子类实现），首先序列化了子类类型所对应的ID。在上面的分析中，我们知道，这是通过查阅由模版特化生成的编译期MAP来获取到的值。  
而在`deserialize()`方法中，采用了X宏将不同的ID映射为了不同的case分支，这实际上是构建了一个运行时的动态类型反射表，也许你会觉得switch/case有些许丑陋，但你必须考虑到这里输入的ID信息完全取决于运行时输入，事实上这里不可避免要引入运行时反射所必须的开销，而这种switch/case的反射方式几乎没有冗余。(当然也许在语义的表达上还存在其他方式)  

## 小结
现在，`InterfaceWrapper`类已经具备了一切完美的抽象特征：
1. 它是值语义的。（这意味着拷贝或者移动它都不需要考虑额外的步骤，表现的如同一个int一样）
2. 它具备动态多态特性。（同时表现出引用语义的优势）
3. 无需特别考虑生命周期管理问题。（通过引用计数）
4. 它的多态特性即便在跨介质/进程的情况下仍然可以保持。（编译期静态反射以及运行时的动态反射）

现在让我们来总结一下为了实现这个完美的wrapper类所需要的步骤：
1. 在抽象基类中必须声明纯虚函数`int get_id() const`方法以要求子类实现type->id映射。
2. 为了给子类编码对应的ID映射方法，我们必须建立编译期的映射map，即`reflection.h`。
3. 为了能实现运行时的id->type的映射，我们必须构建wrapper类并生成运行时反射case表。
4. 为了简化上述两步的代码书写过程并考虑未来的可能扩展，我们采用了“X宏”技术描述了`register.h`接口文件。
5. 为了使`InterfaceWrapper`对象具备值语义，我们并不直接包裹`Interface`对象，而是包裹`shared_ptr<Interface>`对象。
## 最后，观察框架的扩展行为
现在假设我们需要实现一个新的ImplB类型，而要求该类型也要支持跨介质/进程的多态，那么框架该如何为这个类型提供支持呢？
这里我们假设`impl_b.h/cpp`已经具备完好的定义，那么接下来只需要：
```c++
// register.h
// 如果需要看到数据定义的话
#ifdef NEDD_STRUCTURE_DEFINE
  #include "impl/impl_a.h"
+  #include "impl/impl_b.h"
#endif

// 采用“X宏”技术对代码进行二次展开，在不同文件中展开为不同的代码
#define REGISTER(CLASS, ID) __REGISTER__(CLASS, ID)
#ifdef NEED_GENERATE_CODE
  REGISTER(ImplA, 1);
+  REGISTER(ImplB, 2);
#endif
#undef REGISTER
```
two lines code，that's all you need, bro.

enjoy.