#ifndef TIGER_ESCAPE_ESCAPE_H_
#define TIGER_ESCAPE_ESCAPE_H_

#include <memory>

#include "tiger/symbol/symbol.h"

// Forward Declarations
namespace absyn {
class AbsynTree;
} // namespace absyn

namespace esc {

class EscapeEntry {
public:
  int depth_;
  bool *escape_;

  EscapeEntry(int depth, bool *escape) : depth_(depth), escape_(escape) {}
};

using EscEnv = sym::Table<esc::EscapeEntry>;
using EscEnvPtr = sym::Table<esc::EscapeEntry> *;

class EscFinder {
public:
  EscFinder() = delete;
  explicit EscFinder(std::unique_ptr<absyn::AbsynTree> absyn_tree)
      : absyn_tree_(std::move(absyn_tree)), env_(std::make_unique<EscEnv>()) {}

  /**
   * Escape analysis
   */
  void FindEscape();

  /**
   * Transfer the ownership of absyn tree to outer scope
   * @return unique pointer to the absyn tree
   */
  std::unique_ptr<absyn::AbsynTree> TransferAbsynTree() {
    return std::move(absyn_tree_);
  }

private:
  std::unique_ptr<absyn::AbsynTree> absyn_tree_;
  std::unique_ptr<esc::EscEnv> env_;
};
} // namespace esc

#endif