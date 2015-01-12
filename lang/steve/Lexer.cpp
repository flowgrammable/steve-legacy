
#include <steve/Lexer.hpp>
#include <steve/Comment.hpp>

#include <cctype>
#include <iostream>
#include <map>


namespace steve {
namespace {

// -------------------------------------------------------------------------- //
// Comments

// TODO: Only save comments on the input file, and only for certain
// applications. We don't always need to do this.
//
// Save the comment.
void 
save_comment(Lexer& lex, String str) {
  comments().save(lex.loc, str);
}

// If the previous token was a comment, then we need to break
// its adjacency with the previous comment.
//
// TODO: Only do this when actually managing comments.
void
break_comment_block(Lexer& lex) {
  if (lex.toks.size() > 1) {
    Token& last = lex.toks[lex.toks.size() - 1];
    if (last.kind == comment_tok)
      comments().reset();
  }
}

// -------------------------------------------------------------------------- //
// Lexer control

// Advance the lexer by n characters assuming that a) the new position
// is not past the limit, and b) a newline sequence is not included
// in that string.
inline void
advance(Lexer& lex, int n = 1) {
  lex.first += n;
  lex.loc.col += n;
}

// Save a token having the given location, symbol, and text.
inline void
save(Lexer& lex, Token_kind k, String str) {
  lex.toks.emplace_back(lex.loc, k, str);
  break_comment_block(lex);
}

// -------------------------------------------------------------------------- //
// Characters

// Returns true if c is in [a-zA-Z_].
inline bool
is_id_head(char c) { return std::isalpha(c) || c == '_'; }

// Returns true if c is in [a-zA-Z0-9_].
inline bool
is_id_rest(char c) {return std::isalpha(c) || std::isdigit(c) || c == '_'; }

// Returns true if c is in [0-9].
inline bool
is_digit(char c) { return std::isdigit(c); }

// Returns true if c is in [0-1].
inline bool
is_bin_digit(char c) { return c == '0' || c == '1'; }

// Returns true if c is in [0-9a-fA-F]
inline bool
is_hex_digit(char c) { return std::isxdigit(c); }


// -------------------------------------------------------------------------- //
// Lexing rules

// Returns true if the next character matches c.
inline bool
next_char_is(Lexer& lex, char c) {
  if (lex.last - lex.first > 1)
    return *(lex.first + 1) == c;
  else
    return false;
}

// Consume horizontal whitespace starting at the current character.
//
// TODO: Consume all consecutive horizontal whitespace?
void
lex_space(Lexer& lex) { advance(lex); }

// Consume a newline starting at the current character.
//
// TODO: Recognize CR as a newline character and handle combinations of
// CR/LF and LF/CR as a single newline.
void
lex_newline(Lexer& lex) {
  ++lex.first;
  ++lex.loc.line;
  lex.loc.col = 1;
}

// Consume a comment, starting with "//" and up to (but not including)
// the new line.
void
lex_comment(Lexer& lex) {
  auto iter = lex.first + 2;
  lex.first += 2;
  while (lex.first != lex.last and *lex.first != '\n')
    ++lex.first;
  String text(iter, lex.first);
  save_comment(lex, text);  
}

// Consume an n-character lexeme, creating a token.
void
lex_ngraph(Lexer& lex, Token_kind sym, int n) {
  String str(lex.first, lex.first + n);
  save(lex, sym, str);
  advance(lex, n);
}

// Consume a single-character symbol, creating a corresponding token.
void
lex_unigraph(Lexer& lex, Token_kind k) { return lex_ngraph(lex, k, 1); }

// Consume a 2-character symnbol, creating a token.
void
lex_digraph(Lexer& lex, Token_kind k) { return lex_ngraph(lex, k, 2); }

void
lex_error(Lexer& lex) {
  std::cerr << lex.loc << ": error: unrecognized character '" 
            << *lex.first << "'\n";
  advance(lex);
}

// Consume an identifier or keyword.
void
lex_id(Lexer& lex) {
  Lexer::Iterator iter = lex.first + 1;
  while (iter != lex.last and is_id_rest(*iter))
    ++iter;
  String str(lex.first, iter);
  if (Token_kind k = keyword(str))
    save(lex, k, str);
  else
    save(lex, identifier_tok, str);
  advance(lex, iter - lex.first);
}

// Lex an integer-formatted number. Start scanning at n characters
// past current character. This allows skipping prefixes when we
// know what they are.
//
// TODO: Allow for lex and binary floats?
template<Token_kind K, typename Fn>
  void 
  lex_integer(Lexer& lex, Fn test, int n) {
    Lexer::Iterator start = lex.first + n;
    Lexer::Iterator iter = start;
    while (iter != lex.last and test(*iter))
      ++iter;

    // Save the string, but without any prefix characters. 
    String str(start, iter);
    save(lex, K, str);

    // Advance past the number.
    advance(lex, iter - lex.first);
  }

// Consume a numeric value.
//
// FIXME: Allow lexing of octal values. Also add the ability to lex
// floating point numbers. See lex_integer for todo information on
// non-decimal floats.
//
// Note that we have range expressions, separated by '..'. Make sure
// that we safely bak up to a good entry point if we find those.
void
lex_num(Lexer& lex) {
  if (*lex.first == '0') {
    if (next_char_is(lex, 'b' || next_char_is(lex, 'B'))) {
      lex_integer<binary_literal_tok>(lex, &is_bin_digit, 2);
      return;
    }
    else if (next_char_is(lex, 'x') || next_char_is(lex, 'X')) {
      lex_integer<hexadecimal_literal_tok>(lex, is_hex_digit, 2);
      return;
    }
    // Fall through, lexing this this as a decimal value.
  }

  // Lex the value as an integer.
  lex_integer<decimal_literal_tok>(lex, is_digit, 0);
}

// Lex a term in an IPv4 address. Each term is an integer value.
// The lexer state is updated iff there is a match.
bool
lex_ipv4_decimal(Lexer& lex) {
  auto iter = lex.first;
  while (iter != lex.last and is_digit(*iter))
    ++iter;
  if (iter == lex.first)
    return false;
  lex.first = iter;
  return true;
}

// Lex a term in an ipv4 address that is follwed by a '.'.
bool
lex_ipv4_dotted_decimal(Lexer& lex) {
  if (lex_ipv4_decimal(lex))
    if (lex.first != lex.last && *lex.first == '.') {
      ++lex.first;
      return true;
    }
  return false;
}

// Lex an IPv4 token, returning true if there is a match
// and false otherwise.
bool
lex_ipv4(Lexer& lex) {
  auto iter = lex.first;
  if (lex_ipv4_dotted_decimal(lex))
    if (lex_ipv4_dotted_decimal(lex))
      if (lex_ipv4_dotted_decimal(lex))
        if (lex_ipv4_decimal(lex)) {
          String str(iter, lex.first);
          save(lex, ipv4_tok, str);
          return true;
        }

  // Reset the lexer.
  lex.first = iter;
  return false;
}

void 
lex(Lexer& lex) {
  switch (*lex.first) {
  // Horizontal whitespace
  case ' ':
  case '\t': lex_space(lex); break;

  // Vertical whitespace
  case '\n': lex_newline(lex); break;

  case '/':
    if (next_char_is(lex, '/'))
      lex_comment(lex);
    else
      lex_unigraph(lex, slash_tok);
    break;

  case '(': lex_unigraph(lex, lparen_tok); break;
  case ')': lex_unigraph(lex, rparen_tok); break;
  case '{': lex_unigraph(lex, lbrace_tok); break;
  case '}': lex_unigraph(lex, rbrace_tok); break;
  case '[': lex_unigraph(lex, lbracket_tok); break;
  case ']': lex_unigraph(lex, rbracket_tok); break;
  case ';': lex_unigraph(lex, semicolon_tok); break;
  case ':': lex_unigraph(lex, colon_tok); break;
  case ',': lex_unigraph(lex, comma_tok); break;
  
  case '.': 
    if (next_char_is(lex, '.'))
      lex_digraph(lex, dot_dot_tok);
    else
      lex_unigraph(lex, dot_tok); 
    break;
  
  case '=': 
    if (next_char_is(lex, '='))
      lex_digraph(lex, equal_equal_tok);
    else
      lex_unigraph(lex, equal_tok); 
    break;

  case '+': lex_unigraph(lex, plus_tok); break;
  
  case '-':
    if (next_char_is(lex, '>'))
      return lex_digraph(lex, arrow_tok);
    else
      lex_unigraph(lex, minus_tok); break;
    break;
  
  case '*': lex_unigraph(lex, star_tok); break;
  case '%': lex_unigraph(lex, percent_tok); break;

  case '&': lex_unigraph(lex, ampersand_tok); break;
  case '|': lex_unigraph(lex, pipe_tok); break;
  case '^': lex_unigraph(lex, caret_tok); break;
  case '~': lex_unigraph(lex, tilde_tok); break;

  case '<':
    if (next_char_is(lex, '<'))
      lex_digraph(lex, langle_langle_tok);
    else if (next_char_is(lex, '='))
      lex_digraph(lex, langle_equal_tok);
    else
      lex_unigraph(lex, langle_tok);
    break;

  case '>':
    if (next_char_is(lex, '>'))
      lex_digraph(lex, rangle_rangle_tok);
    else if (next_char_is(lex, '='))
      lex_digraph(lex, rangle_equal_tok);
    else
      lex_unigraph(lex, rangle_tok);
    break;

  default:
    // Maybe this is an identifier, keyowrd, number, or address.
    if (is_id_head(*lex.first)) {
      // TODO: This could be an ipv6 address.
      lex_id(lex);      
    }
    else if (is_digit(*lex.first)) {
      // Try a complete ipv4 address first.
      if (lex_ipv4(lex))
        break;
      else
        lex_num(lex);
    }
    else
      lex_error(lex);
    break;
  }
}

} // namespace

Tokens
Lexer::operator()(File* file, Iterator f, Iterator l) {
  use_diagnostics(diags);

  first = f;
  last = l;
  loc = Location(file);

  while (first != last)
    lex(*this);

  return toks;
}

} // namespace steve
