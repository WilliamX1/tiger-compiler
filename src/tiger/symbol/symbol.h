#ifndef TIGER_SYMBOL_SYMBOL_H_
#define TIGER_SYMBOL_SYMBOL_H_

#include <string>

#include "tiger/util/table.h"

/**
 * Forward Declarations
 */
namespace env {
class EnvEntry;
} // namespace env
namespace type {
class Ty;
} // namespace type

namespace sym {
class Symbol {
  template <typename ValueType> friend class Table;

public:
  static Symbol *UniqueSymbol(std::string_view);
  [[nodiscard]] std::string Name() const { return name_; }

private:
  Symbol(std::string name, Symbol *next)
      : name_(std::move(name)), next_(next) {}

  std::string name_;
  Symbol *next_;
};

template <typename ValueType>
class Table : public tab::Table<Symbol, ValueType> {
public:
  Table() : tab::Table<Symbol, ValueType>() { in_loop_ = 0; }
  void BeginScope();
  void EndScope();
  void BeginLoop();
  void EndLoop();
  bool inLoop();

private:
  Symbol marksym_ = {"<mark>", nullptr};
  int in_loop_; /* loop level */
};

template <typename ValueType> void Table<ValueType>::BeginScope() {
  this->Enter(&marksym_, nullptr);
}

template <typename ValueType> void Table<ValueType>::EndScope() {
  Symbol *s;
  do
    s = this->Pop();
  while (s != &marksym_);
}

template <typename ValueType> void Table<ValueType>::BeginLoop() {
  this->in_loop_++;
}

template <typename ValueType> void Table<ValueType>::EndLoop() {
  this->in_loop_--;
}

template <typename ValueType> bool Table<ValueType>::inLoop() {
  return in_loop_ > 0;
}

} // namespace sym

#endif // TIGER_SYMBOL_SYMBOL_H_
