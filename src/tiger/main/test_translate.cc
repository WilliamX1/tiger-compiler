#include "tiger/absyn/absyn.h"
#include "tiger/escape/escape.h"
#include "tiger/frame/x64frame.h"
#include "tiger/parse/parser.h"
#include "tiger/translate/translate.h"

frame::RegManager *reg_manager;
frame::Frags *frags;

int main(int argc, char **argv) {
  std::string_view fname;
  std::unique_ptr<absyn::AbsynTree> absyn_tree;
  reg_manager = new frame::X64RegManager();
  frags = new frame::Frags();
  
  if (argc < 2) {
    fprintf(stderr, "usage: tiger-compiler file.tig\n");
    exit(1);
  }

  fname = std::string_view(argv[1]);

  {
    std::unique_ptr<err::ErrorMsg> errormsg;

    {
      // Lab 3: parsing
    //   TigerLog("-------====Parse=====-----\n");
      Parser parser(fname, std::cerr);
      parser.parse();
      absyn_tree = parser.TransferAbsynTree();
      errormsg = parser.TransferErrormsg();
    }

    {
      // Lab 5: escape analysis
      // TigerLog("-------====Escape analysis=====-----\n");
      esc::EscFinder esc_finder(std::move(absyn_tree));
      esc_finder.FindEscape();
      absyn_tree = esc_finder.TransferAbsynTree();
      absyn_tree->Print(stderr);
    }
    
    {
      // Lab 5: translate IR tree
    //   TigerLog("-------====Translate=====-----\n");
      tr::ProgTr prog_tr(std::move(absyn_tree), std::move(errormsg));
      prog_tr.Translate();
      errormsg = prog_tr.TransferErrormsg();
    }

    if (errormsg->AnyErrors())
      return 1; // Don't continue if error occurrs
  }

  return 0;
}

