#ifndef GENET_NODE_POINTER_WRAPPER_H
#define GENET_NODE_POINTER_WRAPPER_H

#include <stdint.h>

namespace genet_node {

template <class T>
class Pointer {
public:
  static Pointer null();
  static Pointer ref(T *ptr);
  static Pointer ref(const T *ptr);
  static Pointer owned(T *ptr);
  static Pointer owned(const T *ptr);
  Pointer(const Pointer<T> &ptr) = default;
  bool operator==(const Pointer<T> &ptr) const;
  bool operator!=(const Pointer<T> &ptr) const;
  operator bool() const;
  T *get();
  const T *getConst();
  T *getOwned();
  const T *getConstOwned();

private:
  Pointer(intptr_t data);
  T *ptr() const;
  bool isMutable() const;
  bool isOwned() const;

private:
  intptr_t data;
};

template <class T>
Pointer<T> Pointer<T>::null() {
  return Pointer<T>(0);
}

template <class T>
Pointer<T> Pointer<T>::ref(T *ptr) {
  return Pointer<T>(reinterpret_cast<intptr_t>(ptr) | 0b01);
}

template <class T>
Pointer<T> Pointer<T>::ref(const T *ptr) {
  return Pointer<T>(reinterpret_cast<intptr_t>(ptr) | 0b00);
}

template <class T>
Pointer<T> Pointer<T>::owned(T *ptr) {
  return Pointer<T>(reinterpret_cast<intptr_t>(ptr) | 0b11);
}

template <class T>
Pointer<T> Pointer<T>::owned(const T *ptr) {
  return Pointer<T>(reinterpret_cast<intptr_t>(ptr) | 0b10);
}

template <class T>
Pointer<T>::Pointer(intptr_t data) : data(data) {}

template <class T>
bool Pointer<T>::operator==(const Pointer<T> &ptr) const {
  return data == ptr.data;
}

template <class T>
bool Pointer<T>::operator!=(const Pointer<T> &ptr) const {
  return data != ptr.data;
}

template <class T>
Pointer<T>::operator bool() const {
  return data != 0;
}

template <class T>
T *Pointer<T>::get() {
  if (isMutable()) {
    return ptr();
  }
  return nullptr;
}

template <class T>
const T *Pointer<T>::getConst() {
  return ptr();
}

template <class T>
T *Pointer<T>::getOwned() {
  if (isMutable() && isOwned()) {
    return ptr();
  }
  return nullptr;
}

template <class T>
const T *Pointer<T>::getConstOwned() {
  if (isOwned()) {
    return ptr();
  }
  return nullptr;
}

template <class T>
T *Pointer<T>::ptr() const {
  return reinterpret_cast<T *>(data & (~0b11));
}

template <class T>
bool Pointer<T>::isMutable() const {
  return data & 0b01;
}

template <class T>
bool Pointer<T>::isOwned() const {
  return data & 0b10;
}

} // namespace genet_node

#endif
