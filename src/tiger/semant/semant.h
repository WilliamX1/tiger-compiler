#ifndef TIGER_SEMANT_SEMANT_H_
#define TIGER_SEMANT_SEMANT_H_

#include <list>
#include <memory>

#include "tiger/absyn/absyn.h"
#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/semant/types.h"

namespace sem {

//lab4 only
class ProgSem {
public:
  ProgSem(std::unique_ptr<absyn::AbsynTree> absyn_tree,
         std::unique_ptr<err::ErrorMsg> errormsg)
      : absyn_tree_(std::move(absyn_tree)), errormsg_(std::move(errormsg)),
        tenv_(std::make_unique<env::TEnv>()),
        venv_(std::make_unique<env::VEnv>()) {}

  /**
   * Semantic analysis
   */
  void SemAnalyze();

  /**
   * Transfer the ownership of errormsg to outer scope
   * @return unique pointer to errormsg
   */
  std::unique_ptr<err::ErrorMsg> TransferErrormsg() {
    return std::move(errormsg_);
  }

  /**
   * Transfer the ownership of absyn tree to outer scope
   * @return unique pointer to the absyn tree
   */
  std::unique_ptr<absyn::AbsynTree> TransferAbsynTree() {
    return std::move(absyn_tree_);
  }

private:
  std::unique_ptr<absyn::AbsynTree> absyn_tree_;
  std::unique_ptr<err::ErrorMsg> errormsg_;
  
  std::unique_ptr<env::TEnv> tenv_;
  std::unique_ptr<env::VEnv> venv_;

  // Fill base symbol for var env and type env
  void FillBaseVEnv();
  void FillBaseTEnv();
};

} // namespace sem

#endif
