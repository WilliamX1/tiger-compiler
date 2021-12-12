%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

 /* You can add lex definitions here. */
 /* TODO: Put your lab2 code here */
  digits  [0-9]+
%x COMMENT STR IGNORE

%%

 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
"\n" {adjust(); errormsg_->Newline();}

","                 {adjust(); return Parser::COMMA; }
":"                 {adjust(); return Parser::COLON; }
";"                 {adjust(); return Parser::SEMICOLON; }
"("                 {adjust(); return Parser::LPAREN; }
")"                 {adjust(); return Parser::RPAREN; }
"["                 {adjust(); return Parser::LBRACK; }
"]"                 {adjust(); return Parser::RBRACK; }
"{"                 {adjust(); return Parser::LBRACE; }
"}"                 {adjust(); return Parser::RBRACE; }
"."                 {adjust(); return Parser::DOT; }
"+"                 {adjust(); return Parser::PLUS; }
"-"                 {adjust(); return Parser::MINUS; }
"*"                 {adjust(); return Parser::TIMES; }
"/"                 {adjust(); return Parser::DIVIDE; }
"="                 {adjust(); return Parser::EQ; }
"<>"                {adjust(); return Parser::NEQ; }
"<"                 {adjust(); return Parser::LT; } 
"<="                {adjust(); return Parser::LE; }
">"                 {adjust(); return Parser::GT; }
">="                {adjust(); return Parser::GE; }
"&"                 {adjust(); return Parser::AND; }
"|"                 {adjust(); return Parser::OR; }
":="                {adjust(); return Parser::ASSIGN; }

 /* reserved words */
"array" {adjust(); return Parser::ARRAY;}
 /* TODO: Put your lab2 code here */
"if"    {adjust(); return Parser::IF; }
"then"  {adjust(); return Parser::THEN; }
"else"  {adjust(); return Parser::ELSE; }
"while" {adjust(); return Parser::WHILE; }
"for"   {adjust(); return Parser::FOR; }
"to"    {adjust(); return Parser::TO; }
"do"    {adjust(); return Parser::DO; }
"let"   {adjust(); return Parser::LET; }
"in"    {adjust(); return Parser::IN; }
"end"   {adjust(); return Parser::END; }
"of"    {adjust(); return Parser::OF; }
"break" {adjust(); return Parser::BREAK; }
"nil"   {adjust(); return Parser::NIL; }
"function" {adjust(); return Parser::FUNCTION; }
"var"   {adjust(); return Parser::VAR; }
"type"  {adjust(); return Parser::TYPE; }


/* Identifier and Integer */
[a-zA-Z][_a-zA-Z0-9]*    {adjust(); string_buf_ = matched(); return Parser::ID; }
[0-9]+     {adjust(); string_buf_ = matched(); return Parser::INT; }

/* String and Comment */
\" {adjust(); begin(StartCondition__::STR); string_buf_.clear(); }
"/*" {adjust(); comment_level_ = 0; begin(StartCondition__::COMMENT); }


<STR>{
  \"  {adjustStr(); begin(StartCondition__::INITIAL); setMatched(string_buf_); return Parser::STRING; }
  \\\"  {adjustStr(); string_buf_ += '\"'; }
  \\\\  {adjustStr(); string_buf_ += '\\'; }
  \\n   {adjustStr(); string_buf_ += '\n'; }
  \\t   {adjustStr(); string_buf_ += '\t'; }
  \\[0-9]{3}  {adjustStr(); string_buf_ += (char)atoi(matched().c_str() + 1); }
  \\[ \n\t\f]+\\ {adjustStr(); }
  \\\^[A-Z]  {adjustStr(); string_buf_ += matched()[2] - 'A' + 1; }
  . {adjustStr(); string_buf_ += matched(); }
}

<COMMENT>{
  "*/"  {adjustStr(); if (comment_level_) comment_level_--; else begin(StartCondition__::INITIAL); }
  .|\n  {adjustStr(); }
  "/*"  {adjustStr(); comment_level_++; }
}

 /* illegal input */
. {adjust(); errormsg_->Error(errormsg_->tok_pos_, "illegal token");}
 /* end of file */
<<EOF>>  return 0;