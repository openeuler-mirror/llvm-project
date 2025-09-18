// RUN: %clang_cc1 -std=c++14 -Werror -Wno-error=c++14-extensions -fsyntax-only %s -fGNU-compatibility
// RUN: %clang_cc1 -fsyntax-only -verify %s -std=c++11 -Wc++11-compat -fGNU-compatibility

template <class T>
void f (T* p)
{
  p->~auto(); // expected-warning {{'~auto' only available with '-std=c++14 or -std=gnu++14'}}
}

int d;
struct A { ~A() { ++d; } };

int g (int x)
{
  f(new int(x));
  f(new A);
  if (d != 1) {
    return 1;
  }
  (new int)->~auto(); // expected-warning {{'~auto' only available with '-std=c++14 or -std=gnu++14'}}
  return 0;
}
