#include "impl/impl_a.h"
#include "impl/impl_b.h"
#include "wrapper.h"
#include <fstream>
#include <memory>
#include <vector>

using namespace std;

namespace std {
  template <typename T>
  ostream &operator<<(ostream &out, const std::vector<T> &array) {
    if (&out == &cout) {// 打印至标准输出时，采用print方式，而非序列化
      for (const auto &object : array)
        object->print();
    } else {
      out << (int)array.size() << endl;
      for (const auto &object : array)
        object.serialize(out);
    }
    return out;
  }
  template <typename T>
  istream &operator>>(istream &in, std::vector<T> &array) {
    array.clear();
    int size = 0;
    in >> size;
    array.resize(size);
    for (auto &object : array) {
      object.deserialize(in);
    }
    return in;
  }
}

// output :
// before serialize:
// my name is : Alick
// name ptr address : 0xaaaae91aeed8
// my age is : 12

// after deserialize:
// my name is : Alick
// name ptr address : 0xaaaae91b15c8
// my age is : 12
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