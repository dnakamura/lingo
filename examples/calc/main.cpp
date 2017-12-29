// Copyright (c) 2015 Andrew Sutton
// All rights reserved

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "directive.hpp"
#include "step.hpp"

#include "lingo/error.hpp"
#include "lingo/memory.hpp"
#include "lingo/io.hpp"

#include <iostream>


using namespace lingo;
using namespace calc;


// Initialize the token set used by the language.
void
init_tokens()
{
  symbols.put_symbol(lparen_tok, "(");
  symbols.put_symbol(rparen_tok, ")");
  symbols.put_symbol(plus_tok, "+");
  symbols.put_symbol(minus_tok, "-");
  symbols.put_symbol(star_tok, "*");
  symbols.put_symbol(slash_tok, "/");
}


std::istream&
prompt(std::string& line)
{
  std::cout << "> ";
  return getline(std::cin, line);
}


int
main()
{
  init_colors();
  init_tokens();

  evaluation_mode(eval_mode);
  std::string line;
  while (prompt(line)) {
    if (line.empty())
      continue;

    // Construct a buffer for the line.
    Buffer buf(line);

    // If the input contains a directive, then process
    // that and continue.
    if (contains_directive(buf)) {
      process_directive(buf);
      continue;
    }

    try {
      Character_stream cs(buf);
      Token_stream ts;
      Lexer lex(cs, ts);
      Parser parse(ts);

      // Transform characters into tokens.
      lex();
      if (error_count()) {
        reset_diagnostics();
        continue;
      }

      // Transform tokens into abstract syntax.
      Expr const* expr = parse();
      if (error_count()) {
        reset_diagnostics();
        continue;
      }

      // This isn't an error. It's essentially an empty
      // sequence of tokens.
      if (!expr)
        continue;

      if (is_step_mode())
        step_eval(expr);
      else
        std::cout << expr << " == " << evaluate(expr) << '\n';
    }
    catch (Parse_error& err) {
      // Clear the diagnostic count and resume
      // taking input and parsing.
      reset_diagnostics();
    }
  }
}
