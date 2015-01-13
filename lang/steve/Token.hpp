
#ifndef METAL_CORE_TOKEN_HPP
#define METAL_CORE_TOKEN_HPP

#include <steve/Meta.hpp>
#include <steve/Memory.hpp>
#include <steve/String.hpp>
#include <steve/Integer.hpp>
#include <steve/Location.hpp>

namespace steve {

// -------------------------------------------------------------------------- //
// Token kind

// The token kind is an encoded binary representation. The high-order
// byte of the token encodes its source language and a bit describing how
// its spelling can be obtained. In paricular, the layout looks like this:
//
//    bXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
//
// Where b indicates whether the token spelling is embedded in the
// representation or is represented externally, in a token table.
//
// When set, the spelling of the token is taken from the low 3 bytes. When
// not set, the remaining 3 bytes are an integer value that indexes the
// the token kind. That is, we have this pattern:
//
//    10000000 CCCCCCCC CCCCCCCC CCCCCCCC
//
// where C denotes the caracter bits. That is, the token can store the
// spelling of tokens of at least 3 characters in length. 
//
// When not set, the token is a unique numeric identifier of 3 bytes. The
// remaining bits in the high-order byte are flags indicating various
// properties of the token.
//
//    0ttttttt NNNNNNNN NNNNNNNN NNNNNNNN
//
// Where N denotes values that can be used for identifiers and t denotes
// the type of the token (there are at most 8 types of tokens). The values
// stored in the t-bits are defined by the Token_type value. When the type
// is 0, the token does not represent a concrete value. In this case, the
// token denotes a keyword.
using Token_kind = std::uint32_t;

// The token type denotes how a compiler should elaborate the token.
// That is, how it's internal value should be acquired from the
// token's spelling.
//
// Currently, the primary use of these identifiers is to signify that
// a token is actually a multi-valued set.
//
// TODO: We probably need a far more granular type system for
// elaborating tokens. It may also be reasonable to allow languages
// to impose their own elaboration scheme for tokens.
using Token_type = std::uint32_t;

namespace token {
// Returns true if spelled explicitly.
constexpr bool
is_spelled(Token_kind k) { return k & 0x80000000; }

// Returns true if the token is numbered.
constexpr bool
is_numbered(Token_kind k) { return not is_spelled(k); }

// Returns the token type. Behavior is undefined when then token is
// not numbered.
constexpr Token_type
get_type(Token_kind k) { return k >> 24; }

// Return true if the token is typed. This is only true for numbered
// tokens.
constexpr bool
is_typed(Token_kind k) { return is_numbered(k) and get_type(k) != 0; }

// Returns true if the token denotes a keyword.
constexpr bool
is_keyword(Token_kind k) { return is_numbered(k) and get_type(k) == 0; }

constexpr Token_kind
make_spelled (const char* p, const char* q, std::size_t n) {
  return p == q ? 0 : (*p << (n * 8)) | make_spelled(p + 1, q, n + 1);
}

constexpr Token_kind
make_spelled(const char* str, std::size_t n) {
  return make_spelled(str, str + n, 0);
}

// Make a token whose spelling is represented by the given string. Behavior
// is undefined when the length of the string is greater than 3.
constexpr Token_kind
make_spelled(const char* str) {
  return 0x80000000 | make_spelled(str, std::strlen(str));
}

// Make a numbered token. 
constexpr Token_kind
make_numbered(std::uint32_t n) { return n; }

// Make a typed token. Behavior is undefined if t == 0.
constexpr Token_kind
make_typed(Token_type t, std::uint32_t n) { return (t << 24) | n; }

} // namespace token


// -------------------------------------------------------------------------- //
// Token construction
//
// The following funcitons are used to define tokens.

// Create a common token with the given compact spelling.
constexpr Token_kind
make_token(const char* str) { return token::make_spelled(str); }

// Create a token having the given id.
constexpr Token_kind
make_token(std::uint32_t n) { return token::make_numbered(n); }

constexpr Token_kind 
make_token(Token_type t, std::uint32_t n) { return token::make_typed(t, n); }

// -------------------------------------------------------------------------- //
// Token classification
//
// Note that the steve language reserves several identifiers, mapping
// them into tokens. For example, the names true and false are not
// defined to be keywords; they are reserved identifiers that map
// to a boolean_literal_token. See init_tokens in Token.cpp to see
// how these are handled.

constexpr Token_type token_id_type      = 1;   // identifier
constexpr Token_type token_bool_type    = 2;   // bools
constexpr Token_type token_int_type     = 3;   // integers
constexpr Token_type token_real_type    = 4;   // reals
constexpr Token_type token_char_type    = 5;   // characters
constexpr Token_type token_str_type     = 6;   // strings
constexpr Token_type token_ipv4_type    = 7;   // ipv4 addresses
constexpr Token_type token_ipv6_type    = 8;   // ipv6 addresses
constexpr Token_type token_comment_type = 127; // comments

// Utility tokens
constexpr Token_kind error_tok               = make_token(0u);
// Operators and punctuators
constexpr Token_kind lparen_tok              = make_token("(");
constexpr Token_kind rparen_tok              = make_token(")");
constexpr Token_kind lbrace_tok              = make_token("{");
constexpr Token_kind rbrace_tok              = make_token("}");
constexpr Token_kind lbracket_tok            = make_token("[");
constexpr Token_kind rbracket_tok            = make_token("]");
constexpr Token_kind semicolon_tok           = make_token(";");
constexpr Token_kind question_tok            = make_token("?");
constexpr Token_kind colon_tok               = make_token(":");
constexpr Token_kind comma_tok               = make_token(",");
constexpr Token_kind dot_tok                 = make_token(".");
constexpr Token_kind dot_dot_tok             = make_token("..");
constexpr Token_kind arrow_tok               = make_token("->");
constexpr Token_kind plus_tok                = make_token("+");
constexpr Token_kind minus_tok               = make_token("-");
constexpr Token_kind star_tok                = make_token("*");
constexpr Token_kind slash_tok               = make_token("/");
constexpr Token_kind percent_tok             = make_token("%");
constexpr Token_kind ampersand_tok           = make_token("&");
constexpr Token_kind pipe_tok                = make_token("|");
constexpr Token_kind caret_tok               = make_token("^");
constexpr Token_kind tilde_tok               = make_token("~");
constexpr Token_kind equal_tok               = make_token("=");
constexpr Token_kind equal_equal_tok         = make_token("==");
constexpr Token_kind bang_equal_tok          = make_token("!=");
constexpr Token_kind langle_tok              = make_token("<");
constexpr Token_kind langle_equal_tok        = make_token("<=");
constexpr Token_kind langle_langle_tok       = make_token("<<");
constexpr Token_kind rangle_tok              = make_token(">");
constexpr Token_kind rangle_equal_tok        = make_token(">=");
constexpr Token_kind rangle_rangle_tok       = make_token(">>");
// Keywords
constexpr Token_kind alias_tok               = make_token(101);
constexpr Token_kind and_tok                 = make_token(102);
constexpr Token_kind array_tok               = make_token(103);
constexpr Token_kind bool_tok                = make_token(104);
constexpr Token_kind char_tok                = make_token(105);
constexpr Token_kind def_tok                 = make_token(106);
constexpr Token_kind else_tok                = make_token(107);
constexpr Token_kind enum_tok                = make_token(108);
constexpr Token_kind export_tok              = make_token(109);
constexpr Token_kind if_tok                  = make_token(110);
constexpr Token_kind import_tok              = make_token(111);
constexpr Token_kind int_tok                 = make_token(112);
constexpr Token_kind nat_tok                 = make_token(113);
constexpr Token_kind not_tok                 = make_token(114);
constexpr Token_kind or_tok                  = make_token(115);
constexpr Token_kind record_tok              = make_token(116);
constexpr Token_kind typename_tok            = make_token(117);
constexpr Token_kind unit_tok                = make_token(118);
constexpr Token_kind using_tok               = make_token(119);
constexpr Token_kind variant_tok             = make_token(120);
constexpr Token_kind where_tok               = make_token(121);
// Identifiers and literals
constexpr Token_kind identifier_tok          = make_token(token_id_type, 200);
constexpr Token_kind boolean_literal_tok     = make_token(token_bool_type, 201);
constexpr Token_kind binary_literal_tok      = make_token(token_int_type, 202);
constexpr Token_kind octal_literal_tok       = make_token(token_int_type, 203);
constexpr Token_kind decimal_literal_tok     = make_token(token_int_type, 204);
constexpr Token_kind hexadecimal_literal_tok = make_token(token_int_type, 205);
constexpr Token_kind character_literal_tok   = make_token(token_char_type, 206);
// Netowrking literals
constexpr Token_kind ipv4_tok                = make_token(token_ipv4_type, 300);
constexpr Token_kind ipv6_tok                = make_token(token_ipv6_type, 301);
// Comments
constexpr Token_kind comment_tok             = make_token(token_comment_type, 500);


// -------------------------------------------------------------------------- //
// Token structure

// A token represents a symbol at a particular location in a
// program's source text.
struct Token {
  Token(Token_kind, String);
  Token(const Location&, Token_kind, String);

  Location   loc;  // The location of the token
  Token_kind kind; // The kind of symbol represented
  String     text; // A textual represntation of the symbol
};

using Tokens = std::vector<Token>;
using Token_iterator = Tokens::const_iterator;

// -------------------------------------------------------------------------- //
// Operations

Token_kind kind(const Token&);
Token_kind kind(const Token*);

String token_name(Token_kind);

Token_kind keyword(String);

String as_identifier(const Token&);
String as_identifier(const Token*);
bool as_boolean(const Token&);
Integer as_integer(const Token&);

// -------------------------------------------------------------------------- //
// Concepts

// Returns true when T is derived from (or is) Token
template<typename T>
  constexpr bool is_token() { return std::is_base_of<Token, T>::value; }

template<typename T, typename U>
  using Requires_token = Requires<is_token<T>(), U>;


// -------------------------------------------------------------------------- //
// Printing

template<typename T>
  struct debug_token { const T& tok; };

template<typename T>
  struct quote_token { const T& tok; };

template<typename T>
  Requires<is_token<T>(), debug_token<T>> debug(const T&);

template<typename T>
  Requires<is_token<T>(), debug_token<T>> debug(const T*);

template<typename T>
  Requires<is_token<T>(), quote_token<T>> quote(const T&);

template<typename T>
  Requires<is_token<T>(), quote_token<T>> quote(const T*);

// Output operators
template<typename C, typename T>
  std::basic_ostream<C, T>& 
  operator<<(std::basic_ostream<C, T>&, const Token&);

template<typename C, typename T, typename K>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const quote_token<K>& k);

template<typename C, typename T, typename K>
  std::basic_ostream<C, T>&
  operator<<(std::basic_ostream<C, T>& os, const debug_token<K>& k);

} // namespace steve

#include <steve/Token.ipp>

#endif
