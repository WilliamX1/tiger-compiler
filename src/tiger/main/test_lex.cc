#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>

#include "tiger/lex/scanner.h"

// Define here to pass compilation, but no use here
frame::RegManager *reg_manager;
frame::Frags frags;

int main(int argc, char **argv) {
  std::map<int, std::string_view> tokname = {{Parser::ID, "ID"},
                                             {Parser::STRING, "STRING"},
                                             {Parser::INT, "INT"},
                                             {Parser::COMMA, "COMMA"},
                                             {Parser::COLON, "COLON"},
                                             {Parser::SEMICOLON, "SEMICOLON"},
                                             {Parser::LPAREN, "LPAREN"},
                                             {Parser::RPAREN, "RPAREN"},
                                             {Parser::LBRACK, "LBRACK"},
                                             {Parser::RBRACK, "RBRACK"},
                                             {Parser::LBRACE, "LBRACE"},
                                             {Parser::RBRACE, "RBRACE"},
                                             {Parser::DOT, "DOT"},
                                             {Parser::PLUS, "PLUS"},
                                             {Parser::MINUS, "MINUS"},
                                             {Parser::TIMES, "TIMES"},
                                             {Parser::DIVIDE, "DIVIDE"},
                                             {Parser::EQ, "EQ"},
                                             {Parser::NEQ, "NEQ"},
                                             {Parser::LT, "LT"},
                                             {Parser::LE, "LE"},
                                             {Parser::GT, "GT"},
                                             {Parser::GE, "GE"},
                                             {Parser::AND, "AND"},
                                             {Parser::OR, "OR"},
                                             {Parser::ASSIGN, "ASSIGN"},
                                             {Parser::ARRAY, "ARRAY"},
                                             {Parser::IF, "IF"},
                                             {Parser::THEN, "THEN"},
                                             {Parser::ELSE, "ELSE"},
                                             {Parser::WHILE, "WHILE"},
                                             {Parser::FOR, "FOR"},
                                             {Parser::TO, "TO"},
                                             {Parser::DO, "DO"},
                                             {Parser::LET, "LET"},
                                             {Parser::IN, "IN"},
                                             {Parser::END, "END"},
                                             {Parser::OF, "OF"},
                                             {Parser::BREAK, "BREAK"},
                                             {Parser::NIL, "NIL"},
                                             {Parser::FUNCTION, "FUNCTION"},
                                             {Parser::VAR, "VAR"},
                                             {Parser::TYPE, "TYPE"}};

  if (argc != 2) {
    fprintf(stderr, "usage: a.out filename\n");
    exit(1);
  }

  Scanner scanner(argv[1]);

  while (int tok = scanner.lex()) {
    switch (tok) {
    case Parser::ID:
    case Parser::STRING:
      printf("%10s %4d %s\n", tokname[tok].data(), scanner.GetTokPos(),
             !scanner.matched().empty() ? scanner.matched().data() : "(null)");
      break;
    case Parser::INT:
      printf("%10s %4d %d\n", tokname[tok].data(), scanner.GetTokPos(),
             std::stoi(scanner.matched()));
      break;
    default:
      printf("%10s %4d\n", tokname[tok].data(), scanner.GetTokPos());
    }
  }
  return 0;
}
