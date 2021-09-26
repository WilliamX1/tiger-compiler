#include "tiger/symbol/symbol.h"

namespace {

constexpr unsigned int HASH_TABSIZE = 109;
sym::Symbol *hashtable[HASH_TABSIZE];

unsigned int Hash(std::string_view str) {
  unsigned int h = 0;
  for (const char *s = str.data(); *s; s++)
    h = h * 65599 + *s;
  return h;
}

} // namespace

namespace sym {

Symbol *Symbol::UniqueSymbol(std::string_view name) {
  unsigned int index = Hash(name) % HASH_TABSIZE;
  Symbol *syms = hashtable[index], *sym;
  for (sym = syms; sym; sym = sym->next_)
    if (sym->name_ == name)
      return sym;
  sym = new Symbol(static_cast<std::string>(name), syms);
  hashtable[index] = sym;
  return sym;
}

} // namespace sym
