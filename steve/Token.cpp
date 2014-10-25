
#include <iostream>
#include <unordered_map>

#include <steve/Token.hpp>
#include <steve/Debug.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Token spelling

namespace {

// A mapping from token kinds to token names. This is used in
// the parser and lexer (and for debugging) to print the class
// of values expected tokens.
std::unordered_map<Token_kind, String> tokens_;

// A mapping from keyword strings to tokens. This is used by the
// lexer to find token kinds based on the spelling of strings
std::unordered_map<String, Token_kind> keywords_;

inline String
get_name(Token_kind k) {
  char buf[4];
  buf[0] = *((char*)&k);
  buf[1] = *((char*)&k + 1);
  buf[2] = *((char*)&k + 2);
  buf[3] = 0;
  return buf;
}

// Bind the spelling of the token to the given name.
void
save_token(Token_kind k, const char* s) {
  steve_assert(tokens_.count(k) == 0, 
              format("token kind '{0}' already registered", s));
  tokens_.insert({k, s});
}

// Insert the token as a keyword.
void
save_keyword(Token_kind k, const char* s) { 
  steve_assert(keywords_.count(s) == 0, 
               format("keyword '{0}' already registered", s));
  keywords_.insert({s, k}); 
}

void
init_token(Token_kind k, const char* s) {
  save_token(k, s);
  if (token::is_keyword(k))
    save_keyword(k, s);
}

} // namespace

void
init_tokens() {
  // System tokens
  init_token(error_tok, "<error>");
  // Keywords
  init_token(alias_tok, "alias");
  init_token(and_tok, "and");
  init_token(bool_tok, "bool");
  init_token(bitfield_tok, "bitfield");
  init_token(char_tok, "char");
  init_token(def_tok, "def");
  init_token(else_tok, "else");
  init_token(enum_tok, "enum");
  init_token(export_tok, "export");
  init_token(if_tok, "if");
  init_token(import_tok, "import");
  init_token(int_tok, "int");
  init_token(nat_tok, "nat");
  init_token(not_tok, "not");
  init_token(or_tok, "or");
  init_token(record_tok, "record");
  init_token(typename_tok, "typename");
  init_token(unit_tok, "unit");
  init_token(variant_tok, "variant");
  init_token(where_tok, "where");
  // Identifiers and literals
  init_token(identifier_tok, "identifier");
  init_token(boolean_literal_tok, "boolean-literal");
  init_token(binary_literal_tok, "binary-literal");
  init_token(octal_literal_tok, "octal-literal");
  init_token(decimal_literal_tok, "decimal-literal");
  init_token(hexadecimal_literal_tok, "hexadecimal-literal");
  init_token(character_literal_tok, "character-literal");
  // Networking literals
  init_token(ipv4_tok, "ipv4-address");
  init_token(ipv6_tok, "ipv6-address");
  // Reserved names
  save_keyword(boolean_literal_tok, "true");
  save_keyword(boolean_literal_tok, "false");
}

// Given a token kind, return the spelling associated with that 
// identifier. Note that numbered tokens must be registered in the
// token spelling table using the spelling() function. This is typically
// done during language initialization.
String
token_name(Token_kind k) {
  // Check for direct spelling first.
  if (token::is_spelled(k))
    return get_name(k);
  
  // Check the spelling table.
  auto iter = tokens_.find(k);
  if (iter != tokens_.end())
    return iter->second;
  else
    return "<unknown token>";
}

// Returns the token kind associated with the given keyword spelling
// or error_tok if no such keyword is availble.
Token_kind
keyword(String s) {
  auto iter = keywords_.find(s);
  if (iter != keywords_.end())
    return iter->second;
  return error_tok;
}

// Elaborate the token as an identifer.
String
as_identifier(const Token& k) {
  steve_assert(token::get_type(k.kind) == token_id_type,
               format("token '{0}' is not an identifier", k));
  return k.text;
}

// Elaborate the token as a boolean value.
bool
as_boolean(const Token& k) {
  steve_assert(token::get_type(k.kind) == token_bool_type,
               format("token '{0}' is not a boolean value", k));
  static String true_ = "true";
  static String false_ = "false";
  if (k.text == true_)
    return true;
  if (k.text == false_)
    return false;
  steve_unreachable("ill-formed boolean-literal");
}

// Elaborate the token as an integer value.
Integer
as_integer(const Token& k) {
  steve_assert(token::get_type(k.kind) == token_int_type,
               format("token '{0}' is not a boolean value", k));
  switch (k.kind) {
  case binary_literal_tok: return {k.text, 2};
  case octal_literal_tok: return {k.text, 8};
  case decimal_literal_tok: return {k.text, 10};
  case hexadecimal_literal_tok: return {k.text, 16};
  default: break;
  }
  steve_unreachable("unhandled integer-literal");
}

} // namespace steve
