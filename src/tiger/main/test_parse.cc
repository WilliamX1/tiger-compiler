#include <cstdio>
#include <fstream>

#include "tiger/absyn/absyn.h"
#include "tiger/parse/parser.h"

// define here to parse compilation
frame::RegManager *reg_manager;
frame::Frags frags;

int main(int argc, char **argv) {
  std::unique_ptr<absyn::AbsynTree> absyn_tree;

  if (argc < 2) {
    fprintf(stderr, "usage: a.out filename\n");
    exit(1);
  }

  Parser parser(argv[1], std::cerr);
  parser.parse();
  absyn_tree = parser.TransferAbsynTree();
  absyn_tree->Print(stderr);
  fprintf(stderr, "\n");
  return 0;
}
