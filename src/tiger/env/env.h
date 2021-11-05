#ifndef TIGER_ENV_ENV_H_
#define TIGER_ENV_ENV_H_

#include "tiger/semant/types.h"
#include "tiger/symbol/symbol.h"

namespace env {
class EnvEntry {
public:
  bool readonly_;

  enum Kind {VAR, FUN};

  Kind kind_;

  explicit EnvEntry(Kind kind, bool readonly = true) : kind_(kind), readonly_(readonly) {}
  virtual ~EnvEntry() = default;
};

class VarEntry : public EnvEntry {
public:
  type::Ty *ty_;

  // For lab4(semantic analysis) only
  explicit VarEntry(type::Ty *ty, bool readonly = false)
      : EnvEntry(VAR, readonly), ty_(ty) {};
};

class FunEntry : public EnvEntry {
public:
  type::TyList *formals_;
  type::Ty *result_;

  // For lab4(semantic analysis) only
  FunEntry(type::TyList *formals, type::Ty *result)
      : EnvEntry(FUN, true), formals_(formals), result_(result) {}

};

using VEnv = sym::Table<env::EnvEntry>;
using TEnv = sym::Table<type::Ty>;
using VEnvPtr = sym::Table<env::EnvEntry> *;
using TEnvPtr = sym::Table<type::Ty> *;
} // namespace env

#endif // TIGER_ENV_ENV_H_
