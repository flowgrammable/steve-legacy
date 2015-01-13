
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <steve/Parser.hpp>
#include <steve/Syntax.hpp>

namespace steve {

namespace {

// -------------------------------------------------------------------------- //
// Parser facilities
//
// TODO: Refactor these into a generic library for parsers.

// Returns there are no more tokens.
inline bool 
end_of_stream(const Parser& p) { return p.current == p.last; }

// Returns a pointer to the current token.
inline const Token*
peek(const Parser& p) { 
  if (end_of_stream(p))
    return nullptr;
  else
    return &*p.current; 
}

// Returns a popinter to the nth token past the current token. If the
// nth token is past the end of the token stream, returns nullptr.
const Token*
lookahead(const Parser& p, std::size_t n) {
  if (p.last - p.current > std::ptrdiff_t(n))
    return &*(p.current + n); 
  else
    return nullptr;
}

// Returns true if the next token has type t.
inline bool
next_token_is(const Parser& p, Token_kind t) {
  if (const Token* k = peek(p))
    return k->kind == t;
  else
    return false;
}

// Returns true if the next token is something other than type t.
inline bool
next_token_is_not(const Parser& p, Token_kind t) {
  return not next_token_is(p, t);
}

// Returns true if the next token is one of those in the given list.
inline bool
next_token_is_in(const Parser& p, std::initializer_list<Token_kind> ks) {
  const Token* k = peek(p);
  if (not k)
    return false;
  for (Token_kind tk : ks)
    if (k->kind == tk)
      return true;
  return false;
}

// Returns true if the last token had type t.
inline bool
last_token_was(const Parser& p, Token_kind t) {
  if (p.current == p.first)
    return false;
  Token_iterator iter = std::prev(p.current);
  return iter->kind == t;
}

// Returns true if the nth token has type t.
inline bool
nth_token_is(const Parser& p, std::size_t n, Token_kind t) { 
  if (const Token* tok = lookahead(p, n))
    return tok->kind == t;
  else
    return false;
}

// Returns the current location in the program source.
Location
location(const Parser& p) { 
  if (const Token* k = peek(p)) {
    return k->loc;
  } else {
    if (p.first != p.last)
      return {eof_location, p.first->loc.file};
    else
      return no_location;
  }
}

// Emit an error at the current input location.
inline Diagnostic_stream
error(Parser& p) { return error(p.diags, location(p)); }

// Emit a note at the current input location.
inline Diagnostic_stream
note(Parser& p) { return error(p.diags, location(p)); }

// Emit an error at the current input location.
inline Diagnostic_stream
sorry(Parser& p) { return sorry(p.diags, location(p)); }

// Returns the current token, and advances the parser.
//
// TODO: Implement brace matching for consumed tokens.
inline const Token*
consume(Parser& p) {
  const Token* tok = &*p.current;
  ++p.current;
  return tok;
}

// Consume tokens up to and including the case where the given
// predicate is satisified.
//
// TODO: Use brace matching so that we don't stop consuming too early.
template<typename P>
  void
  consume_thru(Parser& p, P pred) {
    while (pred(p))
      consume(p);
    consume(p);
  }

// A predicate that returns true 
template<Token_kind K>
  struct is_token {
    bool operator()(Parser& p) const {
      return next_token_is(p, K);
    }
  };

// Consume up to the closing token.
template<Token_kind K>
  inline void
  consume_thru_token(Parser& p) {
    consume_thru(p, is_token<K>());
  }

// Consume up to and including the semicolon.
inline void
consume_thru_semicolon(Parser& p) { 
  consume_thru_token<semicolon_tok>(p); 
}

// Consume up to and including a closing brace.
inline void
consume_thru_brace(Parser& p) { 
  consume_thru_token<rbrace_tok>(p); 
}

// If the current token is of type k, consume and return that token.
inline const Token*
accept(Parser& p, Token_kind k) {
  if (next_token_is(p, k))
      return consume(p);
  else
    return nullptr;
}

// If the current is one of those in ks, consume and return that token.
inline const Token*
accept_one(Parser& p, std::initializer_list<Token_kind> ks) {
  if (next_token_is_in(p, ks))
    return consume(p);
  else
    return nullptr;
}

// Require the current token to match t, consuming it. Generate a
// diagnostic if the current token does not match.
const Token*
expect(Parser& p, Token_kind k) {
  if (const Token* tok = accept(p, k))
    return tok;

  // Print a meaningful error.
  Token_iterator iter = p.current;
  if (end_of_stream(p))
    --iter;
  Diagnostic_stream ds = error(p);
  ds << "expected '" << token_name(k) << "' but found ";
  if (end_of_stream(p))
    ds << "end of file";
  else
    ds << '\'' << iter->text << '\'';

  return nullptr;
}

// -------------------------------------------------------------------------- //
// Parser combinators

// A helper type function for extracting the result type from a
// parsing function. 
template<typename T>
  using Parse_result = decltype(std::declval<T>()(std::declval<Parser&>()));


// -------------------------------------------------------------------------- //
// Expected parse

// Parse the given rule with the expectation that it succeed. If it
// doesn't succeed, emit a diagnostic.
template<typename Rule>
  inline Parse_result<Rule>
  parse_expected(Parser& p, Rule rule, const char* msg) {
    if (auto* t = rule(p))
      return t;
    error(p) << "expected '" << msg << '\'';
    return nullptr;
  }


// -------------------------------------------------------------------------- //
// Unary and binary expressions
// 
// NOTE: The left, right, and unary parser combinators are slightly less
// efficient than hand-coded versions could be because the token kind
// is evaluated twice. Once to match the token, and once to create the
// corresponding node. However, this is only the case when the operator
// matches a set of tokens, mapping to a set of nodes.

// Parse a left-associative binary expression.
//
//    left(sub, op) ::= sub [op sub]*
//
// In this parameterized rule, sub is the sub-grammar matched by the
// parser, and op is a parser that recognizes the set of binary operators
// in the parse.
//
// The msg argument is provided for diagnostics.
template<typename Sub, typename Op>
  Parse_result<Sub>
  parse_left(Parser& p, Sub sub, Op op, const char* msg) {
    if (auto* l = sub(p)) {
      while (const Token* k = op(p)) {
        if (auto* r = parse_expected(p, sub, msg))
          l = new Binary_tree(k, l, r);
        else
          return nullptr;
      }
      return l;
    }
    return nullptr;
  }

// Parse a right-associative binary expression.
// 
//    right(sub, op) ::= sub [op right(sub, op)]
//
// In this parameterized rule, sub is the sub-grammar matched by the
// parser, and op is a parser that recognizes the set of binary operators
// in the parse.
//
// The msg argument is provided for diagnostics.
template<typename Sub, typename Op>
  Parse_result<Sub>
  parse_right(Parser& p, Sub sub, Op op, const char* msg) {
    auto recur = [=](Parser& p) { return parse_right(p, sub, op, msg); };
    if (auto* l = sub(p)) {
      if (const Token* k = op(p)) {
        if (auto* r = parse_expected(p, recur, msg))
          l = new Binary_tree(k, l, r);
        else
          return nullptr;
      }
      return l;
    }
    return nullptr;
  }

// Parse a unary expression.
//
//    unary(sub, op) ::= sub | op unary(sub, op)
//
// In this parameterized rule, sub is the sub-grammar matched by the
// parser, and op is a parser that recognizes the set of binary operators
// in the parse.
//
// The msg argument is provided for diagnostics.
//
// TODO: In many languages, the "recur" rule for a unary expression
// may be a lower-precedence parse. For example, in C++, a the
// operand of a unary-expression is a cast-expression, which includes
// several forms of post-fix expressions (casts and member access) before 
// matching a unary-expression again. It might be nice to parameterize
// over the recursion.
template<typename Sub, typename Op>
  Parse_result<Sub>
  parse_unary(Parser& p, Sub sub, Op op, const char* msg) {
    auto recur = [=](Parser& p) { return parse_unary(p, sub, op, msg); };
    if (const Token* k = op(p)) {
      if (auto* t = parse_expected(p, recur, msg))
        return new Unary_tree(k, t);
    } 
    return sub(p);
  }


// -------------------------------------------------------------------------- //
// Tentative parsing

// Begin a tentative parse. This initialize tp with the current
// state of op (but with an empty set of diagnostics).
inline void
begin_tentative_parse(Parser& op, Parser& tp) {
  tp.first = op.first;
  tp.last = op.last;
  tp.current = op.current;
  use_diagnostics(tp.diags);
}

// Commit to a tentative parse. This updates the parser p with
// the state of tp and restores the diagnostics to the original
// parser. Note that any diagnostics accrued during the tentative
// parse are merged into the diagnostics of the original parser.
inline void
commit_tentative_parse(Parser& op, Parser& tp) {
  op.current = tp.current;
  op.diags.insert(op.diags.end(), tp.diags.begin(), tp.diags.end());
  use_diagnostics(op.diags);
}

// Abort the tentative parse. Reset the diagnostics to those
// of the original parser.
inline void
abort_tentative_parse(Parser& op, Parser& tp) { 
  use_diagnostics(op.diags);
}

// Parse a rule tentatively. If the parse fails (returning nullptr), 
// the no changes are made to the parser.
//
// It is currently the responsibility of the writer of the
// tentatively parsed rule to ensure that no changes to the global
// state are leaked from an aborted tentative parse (i.e., scopes).
template<typename Rule>
  inline Parse_result<Rule>
  parse_tentative(Parser& p, Rule rule) {
    Parser tp;
    begin_tentative_parse(p, tp);
    if (auto* e = rule(tp)) {
      commit_tentative_parse(p, tp);
      return e;
    }
    abort_tentative_parse(p, tp);
    return nullptr;
  }


// -------------------------------------------------------------------------- //
// Enclosures

// Parse an enclosed production.
//
// If the Acc argument is expect, then an the opening token will
// be expected, not conditioned.
template<typename Left, typename Right, typename Sub>
  Parse_result<Sub>
  parse_enclosed(Parser& p, Left left, Right right, Sub sub) {
    if (const Token* l = left(p))
      if (auto* t = sub(p))
        if (const Token* r = right(p))
          return t;
    return nullptr;
  }

// Parse a paren-enclosed production.
template<typename Sub>
  inline Parse_result<Sub>
  parse_paren_enclosed(Parser& p, Sub sub) {
    auto left = [](Parser& p) { return accept(p, lparen_tok); };
    auto right = [](Parser& p) { return expect(p, rparen_tok); };
    return parse_enclosed(p, left, right, sub);
  }

// Parse a brace-enclosed production.
template<typename Sub>
  inline Parse_result<Sub>
  parse_brace_enclosed(Parser& p, Sub sub) {
    auto left = [](Parser& p) { return accept(p, lbrace_tok); };
    auto right = [](Parser& p) { return expect(p, rbrace_tok); };
    return parse_enclosed(p, left, right, sub);
  }

// Parse a bracket-enclosed production.
template<typename Sub>
  inline Parse_result<Sub>
  parse_bracket_enclosed(Parser& p, Sub sub) {
    auto left = [](Parser& p) { return accept(p, lbracket_tok); };
    auto right = [](Parser& p) { return expect(p, rbracket_tok); };
    return parse_enclosed(p, left, right, sub);
  }

// -------------------------------------------------------------------------- //
// Separated expressions

template<typename T>
  using Raw_parse_result = typename std::remove_pointer<Parse_result<T>>::type;

template<typename T>
  using Parse_seq_result = Seq<Raw_parse_result<T>>;

// Parse a list of productions separated by the given parse. Returns
// a sequence of parsed elements.
template<typename Sep, typename Sub>
  Parse_seq_result<Sub>*
  parse_separated(Parser& p, Sep sep, Sub sub) {
    Parse_seq_result<Sub>* result = new Parse_seq_result<Sub>();
    while (auto* t = sub(p)) {
      result->push_back(t);
      if (sep(p))
        continue;
      else
        break;
    }
    return result;
  }

// Parse a comma-separated list of productions.
template<typename Sub>
  inline Parse_seq_result<Sub>*
  parse_comma_separated(Parser& p, Sub sub) {
    auto sep = [](Parser& p) { return accept(p, comma_tok); };
    return parse_separated(p, sep, sub);
  }

// -------------------------------------------------------------------------- //
// Node construction

// Create the node and set its location to the given position.
template<typename T, typename... Args>
  inline T*
  make_node(const Location& loc, Args&&... args) {
    T* n = new T(std::forward<Args>(args)...);
    n->loc = loc;
    return n;
  }

} // namespace


// -------------------------------------------------------------------------- //
// Parsers

Tree* parse_name(Parser&);
Tree* parse_expr(Parser&);
Tree* parse_decl(Parser&);
Tree* pares_stmt(Parser&);


// -------------------------------------------------------------------------- //
// Expression parsers
//
// this also includes parsers for expression-related constructs.

// Parse an argument.
//
//    argument ::= expr
Tree*
parse_argument(Parser& p) { 
  return parse_expr(p); 
}

// Parse an argument-list, a comma-separated list of arguments.
//
//    argument-list ::= comma-separated(argument)
Tree_seq*
parse_argument_list(Parser& p) {
  return parse_comma_separated(p, parse_argument);
}

// Parse type binding. This specifies the type of an object or
// parameter.
//
//    declared-type ::= ':' expr
Tree*
parse_declared_type(Parser& p) {
  if (accept(p, colon_tok))
    return parse_expected(p, parse_expr, "expr");
  return nullptr;
}

Tree*
parse_required_declared_type(Parser& p) {
  if (expect(p, colon_tok))
    return parse_expected(p, parse_expr, "expression");
  return nullptr;
}

// Parse a return type. This specifies the result type of a 
// function declarator.
//
//    return-type ::= '->' expr
Tree*
parse_return_type(Parser& p) {
  if (accept(p, arrow_tok))
    return parse_expected(p, parse_expr, "expression");
  return nullptr;
}

Tree*
parse_required_return_type(Parser& p) {
  if (expect(p, arrow_tok))
    return parse_expected(p, parse_expr, "expression");
  return nullptr;
}

// Parse an initializer clause. This associates a definition with a
// declarator.
//
//    initializer-clause ::= '=' expr
Tree*
parse_initializer_clause(Parser& p) {
  if (accept(p, equal_tok))
    return parse_expr(p);
  return nullptr;
}

Tree*
parse_required_initializer_clause(Parser& p) {
  if (expect(p, equal_tok))
    return parse_expr(p);
  return nullptr;
}

// Parse a where clause.
//
//    where-clause ::= 'where' constraint
Tree*
parse_where_clause(Parser& p) {
  if (accept(p, where_tok)) {
    return parse_expected(p, parse_expr, "expression");
  }
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Names
//
// A name uniquely identifies a declaration within some scope. Note
// That names are distinct from an id-expression, which refers to a
// declaration.

// Parse a basic-id.
//
//    basic-id ::= identifier
Tree*
parse_basic_id(Parser& p) {
  if (const Token* k = accept(p, identifier_tok))
    return new Id_tree(k);
  return nullptr;
}

// Parse a name.
//
//    name ::= basic-id
Tree*
parse_name(Parser& p) {
  if (Tree* n = parse_basic_id(p)) 
    return n;
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Types

// Parse a record field declaration.
//
//    field-decl ::= [named-field-decl | unnamed-field-decl] ';'
//    named-field-decl ::= name declared-type [constriaint-clause]
//    unnamed-field-decl ::= declared-type
//
// Note that the name is optional. If the name is unspecified, there
//
// The point of declaration is after the declarator and before
// the constraint. Note that the field generally appears in the
// constraint expression.
//
// TODO: Allow member initializers?
Tree*
parse_field_decl(Parser& p) {
  const Token* k = peek(p);
  if (not k)
    return nullptr;

  // Parse a named field.
  if (Tree* n = parse_name(p)) {
    if (Tree* t = parse_required_declared_type(p)) {
      Tree* c = parse_where_clause(p);
      if (expect(p, semicolon_tok))
        return new Field_tree(k, n, t, c);
    }
    return nullptr;
  }
  
  // If it wasn't a named field, then it's an unnamed field.
  if (Tree* t = parse_declared_type(p))
    if (expect(p, semicolon_tok))
    return new Field_tree(k, nullptr, t, nullptr);
  return nullptr;
}

// Parse a sequence of fields.
//
//    field-list ::= field-decl*
Tree_seq* 
parse_field_list(Parser& p) {
  Tree_seq* ds = new Tree_seq();
  while (next_token_is_not(p, rbrace_tok)) {
    Tree* d = parse_field_decl(p);
    if (not d)
      return nullptr;
    ds->push_back(d);
  }
  return ds;
}

// Parse a record type.
//
//    record-type ::= 'record' record-body
//    record-body ::= '{' field-list '}';
Tree*
parse_record_type(Parser& p) {
  if (const Token* k = accept(p, record_tok)) {
    if (Tree_seq* ds = parse_brace_enclosed(p, parse_field_list))
      return new Record_tree(k, ds);
  }
  return nullptr;
}

// Parse an alternative within a variant.
//
//    alternative-decl ::= expr declared-type
//
// NOTE: We allow arbitrary expressions here as the tag of
// the alterantive, but they must be constant expresssions.
Tree*
parse_alternative_decl(Parser& p) {
  if (Tree* t1 = parse_expr(p))
    if (Tree* t2 = parse_required_declared_type(p))
      if (expect(p, semicolon_tok))
        return new Alt_tree(t1, t2);
  return nullptr;
}

// Parse a list of alternatives.
//
//    alternative-list ::= semicolon-separated(alternative-decl)
//
// TODO: Semicolon or comma? Eh... or | ?
//
// TODO: This should be implemented as a combinator. It's the same as 
// for the field list.
Tree_seq*
parse_alternative_list(Parser& p) {
  Tree_seq* as = new Tree_seq();
  while (next_token_is_not(p, rbrace_tok)) {
    if (Tree* a = parse_alternative_decl(p)) {
      as->push_back(a);
    }
    else
      return nullptr;
  }
  return as;
}

// Parse a variant type.
//
//    variant-type ::= 'variant' [variant-base] variant-body
//    variant-base ::= '(' expr ')'
//    variant-body ::= '{' alternative-list '}'
Tree*
parse_variant_type(Parser& p) {
  if (const Token* k = accept(p, variant_tok)) {
    Tree* d = parse_paren_enclosed(p, parse_expr);
      if (Tree_seq* as = parse_brace_enclosed(p, parse_alternative_list))
        return new Variant_tree(k, d, as);
  }
  return nullptr;
}

// Returns true if t is an assignment expression.
bool
is_assignment_expr(Tree* t) {
  if (Binary_tree* b = as<Binary_tree>(t))
    return b->op()->kind == equal_tok;
  return false;
}

// Returns true if t is a range expression.
bool
is_range_expr(Tree* t) {
  return is<Range_tree>(t);
}

// Parse an enumerator. An enumerator specifies a valid value
// of an enumeration type. 
//
//    enumerator ::= assignment-expr | range-expr
//
// TODO: Can I enumerate literals? Can I use a constructor to
// enumerate values of of a user-defined type?
Tree*
parse_enumerator(Parser& p) {
  extern Tree* parse_assignment_expr(Parser&);
  if (Tree* t = parse_assignment_expr(p)) {
    if (is_assignment_expr(t) || is_range_expr(t))
      return t;
    else
      error(p) << format("invalid enumerator '{}'", debug(t));
  }
  return nullptr;
}

// Parse an enumerator list.
//
//    enumerator-list ::= comma-separated(enumerator)
Tree_seq*
parse_enumerator_list(Parser& p) {
  return parse_comma_separated(p, parse_enumerator);
}

// Parse an enum type.
//
//    enum-type ::= 'enum' [enum-base] enum-body
//    enum-base ::= '(' expr ')'
//    enum-body ::= '{' enumerator-list '}'
Tree*
parse_enum_type(Parser& p) {
  if (const Token* k = accept(p, enum_tok)) {
    Tree* b = parse_paren_enclosed(p, parse_expr);
    if (Tree_seq* es = parse_brace_enclosed(p, parse_enumerator_list))
      return new Enum_tree(k, b, es);
  }
  return nullptr;
}


// -------------------------------------------------------------------------- //
// Term expressions


// Parse an id expression. An id expression refers to a declaration.
//
//    id-term ::= identifier
//
// FIXME: What about special names (qualified names?).
Tree*
parse_id_expr(Parser& p) {
  if (const Token* k = accept(p, identifier_tok))
    return new Id_tree(k);
  return nullptr;
}


// Parse a literal expression. This inclues numeric as well as
// type literals.
//
//    literal-expr ::= boolean-literal 
//                   | integer-literal 
//                   | type-literal
//
//    boolean-literal ::= 'true' | 'false'
//
//    integer-literal ::= binary-integer-literal
//                      | octal-integer-literal
//                      | decimal-integer-literal
//                      | hexadecimal-integer-literal
//
//    type-literal ::= 'typename' | 'unit' | 'bool' | 'nat' 
//                   | 'int' | 'char'
//
// Note all literals correspond to tokens.
Tree*
parse_literal_expr(Parser& p) {
  const Token* k = peek(p);
  if (not k)
    return nullptr;
  switch (k->kind) {
  // boolean-literal
  case boolean_literal_tok:
  // integer-literal
  case binary_literal_tok:
  case octal_literal_tok:
  case decimal_literal_tok:
  case hexadecimal_literal_tok:
  // type-literal
  case typename_tok:
  case unit_tok:
  case bool_tok:
  case nat_tok:
  case int_tok:
  case char_tok:
    consume(p); 
    return new Lit_tree(k);
  default: 
    return nullptr;
  }
}

// Parse a grouped expression.
//
//    paren-expr ::= '(' comma-expr ')'
//
// Note that a grouping expression allows a nested comma expression.
Tree*
parse_paren_expr(Parser& p) {
  extern Tree* parse_comma_expr(Parser&); // Yes, this works.
  if (accept(p, lparen_tok))
    if (Tree* t = parse_expected(p, parse_comma_expr, "expression"))
      if (expect(p, rparen_tok))
        return t;
  return nullptr;
}

// Parse a primary expression.
//
//    primary-expr ::= literal-expr | id-expr | grouped-expr
//
// The last matched expression is a type. This allows types to be used
// as arguments in expressions. Note that types parsed as terms are
// wrapped in "carry" nodes, to support typing.
Tree*
parse_primary_expr(Parser& p) {
  if (Tree* t = parse_literal_expr(p))
    return t;
  if (Tree* t = parse_id_expr(p))
    return t;
  if (Tree* t = parse_paren_expr(p))
    return t;
  return nullptr;
}

// Parse a call expression.
//
//    call-expr ::= postfix-expr '(' argument-list ')'
Tree*
parse_call_expr(Parser& p, Tree* t) {
  if (Tree_seq* as = parse_paren_enclosed(p, parse_argument_list))
    return new Call_tree(t, as);
  return nullptr;
}

// Parse an index expression.
//
//    index-expr ::= postfix-expr '[' expr ']'
Tree*
parse_index_expr(Parser& p, Tree* t) {
  if (Tree* e = parse_bracket_enclosed(p, parse_expr))
    return new Index_tree(t, e);
  return nullptr;
}

// Parse a dot expression.
//
//    dot-expr ::= postfix-expr '.' postfix-expr
Tree*
parse_dot_expr(Parser& p, Tree* t) {
  if (accept(p, dot_tok))
    if (Tree* t2 = parse_primary_expr(p))
      return new Dot_tree(t, t2);
  return nullptr;
}

// Parse a postfix-expression.
//
//    range-expr :: = postfix-expr '..' postfix-expr
Tree*
parse_range_expr(Parser& p, Tree* t1) {
  if (accept(p, dot_dot_tok)) {
    if (Tree* t2 = parse_primary_expr(p))
      return new Range_tree(t1, t2);
    else
      error(p) << "expected postfix-expression after '..'";
  }
  return nullptr;
}

// Parse an application expression. Note that application.
Tree*
parse_app_expr(Parser& p, Tree* t1) {
  if (Tree* t2 = parse_primary_expr(p))
    return new App_tree(t1, t2);
  return nullptr;
}

// Parse a postfix expression. This is the entry point to all
// binary or n-ary expressions parsed at this precedence.
//
//    postfix-expr ::= call-expr
//                   | index-expr
//                   | range-expr
//                   | app-expr
//                   | primary-expr
//
// TODO: This could be made more efficient by looking at the token
// after the initial primary epxression and then dispatching to
// the appropriate parser.
Tree*
parse_postfix_expr(Parser& p) {
  if (Tree* t = parse_primary_expr(p)) {
    while (t) {
      if (Tree* c = parse_call_expr(p, t))
        t = c;
      else if (Tree* i = parse_index_expr(p, t))
        t = i;
      else if (Tree* d = parse_dot_expr(p, t))
        t = d;
      else if (Tree* r = parse_range_expr(p, t))
        t = r;
      else if (Tree* a = parse_app_expr(p, t))
        t = a;
      else 
        break;
    }
    return t;
  }
  return nullptr;
}

// Parse a unary expression. A unary expression is typically a single-operand
// expression whose operator preceds the operand (e.g., not e).
Tree*
parse_prefix_expr(Parser& p) {
  if (Tree* t = parse_record_type(p))
    return t;
  if (Tree* t = parse_variant_type(p))
    return t;
  if (Tree* t = parse_enum_type(p))
    return t;
  return parse_postfix_expr(p);
}

// Parse a sign expression.
//
//    sign-expr ::= prefix-expr | '-' sign-expr
Tree*
parse_sign_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept(p, minus_tok); 
  };
  return parse_unary(p, parse_prefix_expr, op, "prefix-expr");
}

// Parse a multiplicative expression.
//
//    multiplicative-expr ::= sign-expr 
//                          | multiplicative-expr ['*' | '/' | '%'] sign-expr
 Tree* 
parse_multiplicative_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept_one(p, {star_tok, slash_tok, percent_tok}); 
  };
  return parse_left(p, parse_sign_expr, op, "sign-expression");
}

// Parse an additive expression.
//
//    additive-expr ::= multiplicative-expr 
//                    | additive-expr ['+' | '-'] multiplicative-expr
Tree*
parse_additive_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept_one(p, {plus_tok, minus_tok}); 
  };
  return parse_left(p, parse_multiplicative_expr, op, "multiplicative-expression");
}

// Parse a shift expression.
//
//    shift-expr ::= additive-expr 
//                 | shift-expr ['<<' | '>>'] additive-expr
Tree*
parse_shift_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept_one(p, {langle_langle_tok, rangle_rangle_tok}); 
  };
  return parse_left(p, parse_additive_expr, op, "additive-expression");
}

// Parse a bitwise not expression.
//
//    bitwise-not-expr ::= shift-expr
//                       | '~' bitwise-not-expr
Tree*
parse_bitwise_not_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept(p, tilde_tok); 
  };
  return parse_unary(p, parse_shift_expr, op, "shift-expression");
}

// Parse a bitwise and expression.
//
//    bitwise-and-expr ::= bitwise-not-expr
//                       | bitwise-and-expr '&' bitwise-not-expr
Tree* 
parse_bitwise_and_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept(p, ampersand_tok); 
  };
  return parse_left(p, parse_bitwise_not_expr, op, "bitwise-not-expression");
}

// Parse a bitwise xor expression.
//
//    bitwise-xor-expr ::= bitwise-and-expr
//                       | bitwise-xor-expr '&' bitwise-not-expr
Tree*
parse_bitwise_xor_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept(p, caret_tok); 
  };
  return parse_left(p, parse_bitwise_and_expr, op, "bitwise-and-expression");
}

// Parse a bitwise or expression.
//
//    bitwise-or-expr ::= bitwise-xor-expr
//                      | bitwise-or-expr '&' bitwise-not-expr
Tree*
parse_bitwise_or_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept(p, pipe_tok); 
  };
  return parse_left(p, parse_bitwise_xor_expr, op, "bitwise-xor-expression");
}

// Parse an ordering expression.
//
//    ordering-expr ::= bitwise-or-expr
//                    | ordering-expr ['<' | '>' | '<=' | '>='] bitwise-or-expr
Tree*
parse_ordering_expr(Parser& p) {
  auto op = [](Parser& p) { 
    return accept_one(p, {langle_tok, rangle_tok, langle_equal_tok, rangle_equal_tok});
  };
  return parse_left(p, parse_bitwise_or_expr, op, "bitwise-or-expression");
}

// Parse an equality expresssion.
//
//    equality-expr ::= ordering-expr
//                    | equality-expr ['==' | '!='] ordering-expr
Tree*
parse_equality_expr(Parser& p) {
  auto op = [](Parser& p) -> const Token* {
    return accept_one(p, {equal_equal_tok, bang_equal_tok});
  };
  return parse_left(p, parse_ordering_expr, op, "ordering-expression");
}

// Parse a logical not expression.
//
//    logical-not ::= equality-expr
//                  | 'not' logical-not-expr
Tree*
parse_logical_not_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, not_tok); };
  return parse_unary(p, parse_equality_expr, op, "equality-expression");
}

// Parse a logical and expression.
//
//    logical-and-expr ::= logical-not-expr
//                       | logical-and-expr 'and' logical-not-expr
Tree*
parse_logical_and_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, and_tok); };
  return parse_left(p, parse_logical_not_expr, op, "logical-not-expression");
}

// Parse a logical or expression.
//
//    logical-or-expr ::= logical-and-expr
//                      | logical-or-expr 'or' logical-and-expr
Tree*
parse_logical_or_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, or_tok); };
  return parse_left(p, parse_logical_and_expr, op, "logical-and-expression");
}

// Parse a logical-if expression. Note that this is a right-associative
// operator.
//
//    logical-if-expr ::= logical-and-expr
//                      | logical-and-expr '->' logical-if-expr
Tree*
parse_logical_if_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, arrow_tok); };
  return parse_right(p, parse_logical_or_expr, op, "logical-or-expression");
}

// Parse an expression.
Tree*
parse_expr(Parser& p) {
  return parse_logical_if_expr(p);
}

// Parse an assignment expression. Note that this is a right-associative
// operator.
//
//    assignment-expr ::= logical-if-expr
//                      | logical-if-expr '=' assignment-expr
Tree*
parse_assignment_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, equal_tok); };
  return parse_right(p, parse_expr, op, "expression");
}

// Parse a comma expression: a sequence of comma separated expressions.
//
//    comma-expr ::= expr
//                 | comma-expr ',' expr
//
// Note that the comma epxr has lower precedence than a usual
// expression.
Tree*
parse_comma_expr(Parser& p) {
  auto op = [](Parser& p) { return accept(p, comma_tok); };
  return parse_left(p, parse_assignment_expr, op, "assignment-expression");
}


// -------------------------------------------------------------------------- //
// Declarations

// Parse an const-decl.
//
//    const-decl ::= 'def' name declared-type inititializer-clause;
//
// The point of declaration is after the declarator (n : t), before
// the initializer.
//
// TODO: Allow the declared-type to be ommitted so that it can be
// deduced from its initializer.
Tree*
parse_const_decl(Parser& p, Tree* n) {
  if (Tree* t = parse_declared_type(p))
    return new Value_tree(n, t);
  return nullptr;
}

// Parse a parmaeter-decl. Note that the initializer is option.
//
//    parmaeter-decl ::= name declared-type [initializer-clause]
//
// The point of declaration for a parmater is after its complete
// declarator (n : t).
Tree*
parse_parameter_decl(Parser& p) {
  if (Tree* n = parse_name(p))
    if (Tree* t = parse_required_declared_type(p)) {
      Tree* e = parse_initializer_clause(p);
      return new Parm_tree(n, t, e);
    }
  return nullptr;
}

// Parse a parm-list.
//
//    parm-list ::= parmaeter-decl | [',' parmaeter-decl]*
Tree_seq*
parse_parameter_list(Parser& p) {
  Tree_seq* ps = new Tree_seq();
  while (Tree* d = parse_expected(p, parse_parameter_decl, "parameter-decl")) {
    ps->push_back(d);
    // If there's no comma, we're done with the list. Otherwise,
    // consume and continue.
    if (next_token_is_not(p, comma_tok))
      return ps;
    consume(p);
  }
  return nullptr;
}

// Parse a parameter-clause.
//
//    parameter-clause ::= '(' parameter-list? ')'
//
// Note that the leading '(' has already been matched.
Tree_seq*
parse_parameter_clause(Parser& p) {
  if (accept(p, lparen_tok)) {
    // If the parm-clause is '()', then just return.
    if (accept(p, rparen_tok))
      return new Tree_seq();

    if (Tree_seq* ps = parse_expected(p, parse_parameter_list, "parameter-list"))
      if (expect(p, rparen_tok))
        return ps;
  }
  return nullptr;
}

// Parse a function declarator.
//
//    fn-decl ::= 'def' name parameter-clause result-clause initializer-clause
//
// Function scope begins immediately after the name in the function
// declarator. The point of declaration is after the declarator 
// (n(p1, p2, ...) -> t), before the function body. Note thta this
// means the name of the function is not available in the parameter list.
Tree*
parse_fn_decl(Parser& p, Tree* n) {
  if (Tree_seq* ps = parse_parameter_clause(p))
    if (Tree* t = parse_required_return_type(p))
      return new Fn_tree(n, ps, t);
  return nullptr;
}

// Parse a def-decl.
//
//    def-decl ::= const-decl | fn-decl
//
// Note that all definitions begin with a name. They are differentiated
// by the the tokens following the name.
Tree*
parse_def_decl(Parser& p) {
  if (const Token* k = accept(p, def_tok))
    if (Tree* n = parse_expected(p, parse_name, "name")) {

      // Parse the declarator.
      Tree* d1 = nullptr;
      if (Tree* d2 = parse_const_decl(p, n))
        d1 = d2;
      else if (Tree* d2 = parse_fn_decl(p, n))
        d1 = d2;
      if (not d1) {
        error(p) << "expected 'parameter-list' or ':' after 'name'";
        return nullptr;
      }

      // Parse the initializer.
      if (Tree* e = parse_required_initializer_clause(p))
        return new Def_tree(k, d1, e);
    }
  return nullptr;
}


// Parse an import-decl.
//
//    import-decl ::= 'import' expr
Tree*
parse_import_decl(Parser& p) {
  if (const Token* k = accept(p, import_tok)) {
    if (Tree* t = parse_expr(p))
      return new Import_tree(k, t);
    else
      error(p) << "expected 'expression' after 'import'";
  }
  return nullptr;
}

// Parse a using declaration.
//
//    using-decl ::= 'using' expr
Tree*
parse_using_decl(Parser& p) {
  if (const Token* k = accept(p, using_tok)) {
    if (Tree* t = parse_expr(p))
      return new Using_tree(k, t);
    else
      error(p) << "expected 'expression' after 'using'";
  }
  return nullptr;
}

// Parse a decl-stmt.
//
//    decl-stmt ::= def-decl | alias-decl | import-decl | using-decl
Tree*
parse_decl(Parser& p) {
  if (Tree* d = parse_def_decl(p))
    return d;
  if (Tree* d = parse_import_decl(p))
    return d;
  if (Tree* d = parse_using_decl(p))
    return d;
  return nullptr;
}

// -------------------------------------------------------------------------- //
// Top-level

// Parse a top-level list of statements.
//
//    top ::= decl*
Tree*
parse_top(Parser& p) {
  Tree_seq* ds = new Tree_seq();
  while (not end_of_stream(p)) {
    if (Tree* d = parse_expected(p, parse_decl, "declaration"))
      ds->push_back(d);
    else
      return nullptr;

    // Allow a semicolon after a declaration. It isn't strictly 
    // necessary, but some people like it.
    accept(p, semicolon_tok);
  }
  return new Top_tree(ds);
}


// -------------------------------------------------------------------------- //
// Parser

// Parse a range of tokens.
Tree*
Parser::operator()(Token_iterator f, Token_iterator l) {
  // An empty sequence of tokens is an empty program.
  if (f == l)
    return new Top_tree(new Tree_seq());

  // Set up the parsing state.
  first = f; 
  last = l;
  current = first;

  // All errors are emitted into these diagnostics.
  use_diagnostics(diags);
  
  return parse_top(*this);
}

// Parse a sequence of tokens.
Tree*
Parser::operator()(const Tokens& toks) {
  return (*this)(toks.begin(), toks.end());
}

} // namespace steve