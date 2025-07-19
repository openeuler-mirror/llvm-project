#ifndef WRAPPER_H
#define WRAPPER_H
template <class T> class Wrapper {
  public:
    Wrapper() = default;
    Wrapper(const T &t) : x(t) {}
    void print();
  private:
    T x;
};
template class Wrapper<int>;
#endif
