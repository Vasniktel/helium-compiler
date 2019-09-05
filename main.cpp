#include <iostream>
#include <string>
#include <compiler.hpp>

using namespace std;
using ::helium::Compiler;

int main() {
  vector<uint8_t> dummy;
  for (string s; getline(cin, s);) {
    s += "\n";
    if (auto error = Compiler::FromSource(s, dummy)) {
      cout << error.value();
    }
    cout << endl;
  }
  return 0;
}