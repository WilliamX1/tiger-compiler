#include <cstdio>
#include <fstream>

#include "tiger/absyn/absyn.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/parse/parser.h"
#include "tiger/semant/semant.h"

int main(int argc, char **argv) {
  std::unique_ptr<absyn::AbsynTree> absyn_tree;
  std::unique_ptr<err::ErrorMsg> errormsg;

  if (argc < 2) {
    fprintf(stderr, "usage: a.out filename\n");
    exit(1);
  }

  {
    Parser parser(argv[1], std::cerr);
    parser.parse();
    errormsg = parser.TransferErrormsg();
    absyn_tree = parser.TransferAbsynTree();
  }

  {
    sem::ProgSem program_sem_analyzer(std::move(absyn_tree), std::move(errormsg));
    program_sem_analyzer.SemAnalyze();
  }
  return 0;
}
