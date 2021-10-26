#include "tiger/absyn/absyn.h"
#include "tiger/escape/escape.h"
#include "tiger/frame/x64frame.h"
#include "tiger/output/logger.h"
#include "tiger/output/output.h"
#include "tiger/parse/parser.h"
#include "tiger/translate/translate.h"

int main(int argc, char **argv) {
  std::string_view fname;
  std::unique_ptr<absyn::AbsynTree> absyn_tree;
  std::unique_ptr<frame::RegManager> reg_manager(new frame::X64RegManager());
  std::list<std::unique_ptr<frame::Frag>> frags;

  if (argc < 2) {
    fprintf(stderr, "usage: tiger-compiler file.tig\n");
    exit(1);
  }

  fname = std::string_view(argv[1]);

  {
    std::unique_ptr<err::ErrorMsg> errormsg;

    {
      // Lab 3: parsing
      TigerLog("-------====Parse=====-----\n");
      Parser parser(fname, std::cerr);
      parser.parse();
      absyn_tree = parser.TransferAbsynTree();
      errormsg = parser.TransferErrormsg();
    }

    {
      // Lab 6: escape analysis
      TigerLog("-------====Escape analysis=====-----\n");
      esc::EscFinder esc_finder(std::move(absyn_tree));
      esc_finder.FindEscape();
      absyn_tree = esc_finder.TransferAbsynTree();
    }

    {
      // Lab 5: translate IR tree
      TigerLog("-------====Translate=====-----\n");
      tr::ProgTr prog_tr(std::move(absyn_tree), std::move(errormsg),
                         std::move(reg_manager));
      prog_tr.Translate();
      frags = prog_tr.TransferFrags();
      errormsg = prog_tr.TransferErrormsg();
      reg_manager = prog_tr.TransferRegManager();
    }

    if (errormsg->AnyErrors())
      return 1; // Don't continue if error occurrs
  }

  {
    // Output assembly
    output::AssemGen assem_gen(fname, std::move(frags), std::move(reg_manager));
    assem_gen.GenAssem(false);
  }

  return 0;
}

