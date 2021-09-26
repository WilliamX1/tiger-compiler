#ifndef TIGER_UTIL_TABLE_H_
#define TIGER_UTIL_TABLE_H_

#include <cassert>
#include <functional>

namespace tab {
template <typename KeyType, typename ValueType> class Table {
public:
  Table() : top_(nullptr), table_() {}
  void Enter(KeyType *key, ValueType *value);
  ValueType *Look(KeyType *key);
  void Set(KeyType *key, ValueType *value);
  KeyType *Pop();
  void Dump(std::function<void(KeyType *, ValueType *)> show);

protected:
  static const unsigned long TABSIZE = 127;
  struct Binder {
  public:
    KeyType *key;
    ValueType *value;
    Binder *next;
    KeyType *prevtop;

    Binder(KeyType *key, ValueType *value, Binder *next, KeyType *prevtop)
        : key(key), value(value), next(next), prevtop(prevtop) {}
  };

  Binder *table_[TABSIZE];
  KeyType *top_;
};

template <typename KeyType, typename ValueType>
void Table<KeyType, ValueType>::Enter(KeyType *key, ValueType *value) {
  unsigned long index = reinterpret_cast<unsigned long>(key) % TABSIZE;
  table_[index] = new Binder(key, value, table_[index], top_);
  top_ = key;
}

template <typename KeyType, typename ValueType>
ValueType *Table<KeyType, ValueType>::Look(KeyType *key) {
  assert(key);
  unsigned long index = reinterpret_cast<unsigned long>(key) % TABSIZE;
  Binder *b;
  for (b = table_[index]; b; b = b->next)
    if (b->key == key)
      return b->value;
  return nullptr;
}

template <typename KeyType, typename ValueType>
void Table<KeyType, ValueType>::Set(KeyType *key, ValueType *value) {
  assert(key);
  unsigned long index = reinterpret_cast<unsigned long>(key) % TABSIZE;
  Binder *b;
  for (b = table_[index]; b; b = b->next)
    if (b->key == key) {
      b->value = value;
      return;
    }
}

template <typename KeyType, typename ValueType>
KeyType *Table<KeyType, ValueType>::Pop() {
  KeyType *k = top_;
  assert(k);
  unsigned long index = reinterpret_cast<unsigned long>(k) % TABSIZE;
  Binder *b = table_[index];
  assert(b);
  table_[index] = b->next;
  top_ = b->prevtop;
  return b->key;
}

template <typename KeyType, typename ValueType>
void Table<KeyType, ValueType>::Dump(
    std::function<void(KeyType *, ValueType *)> show) {
  KeyType *k = top_;
  unsigned long index = reinterpret_cast<unsigned long>(k) % TABSIZE;
  Binder *b = table_[index];
  if (b == nullptr)
    return;
  table_[index] = b->next;
  top_ = b->prevtop;
  show(b->key, b->value);
  Dump(show);
  assert(top_ == b->prevtop && table_[index] == b->next);
  top_ = k;
  table_[index] = b;
}

} // namespace tab

#endif // TIGER_UTIL_TABLE_H_
