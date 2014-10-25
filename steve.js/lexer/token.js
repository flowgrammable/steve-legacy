
(function(){

var _   = require('underscore');
var fmt = require('../utils/formatter');

function Token(file, line, col, val, tp) {
  fmt.Formattable.call(this);
  this.filename = file;
  this.lineno = line;
  this.colno = col;
  this.value = val;
  this.type = tp;
}
Token.prototype = Object.create(fmt.Formattable);
Token.constructor = Token;

Token.prototype.toFormatter = function(f) {
  f.begin('Token');
  f.addPair('File', this.filename);
  f.addPair('Line #', this.lineno);
  f.addPair('Col #', this.colno);
  if(this.type !== 'NEWLINE') {
    f.addPair('Value', this.value);
  }
  f.addPair('Type', this.type);
  f.end();
};

function mkToken(file, line, col, val, tp) {
  return new Token(file, line, col, val, tp);
}

function mkDigits(file, line, col, val) {
  return mkToken(file, line, col, val, 'DIGITS');
}

function mkIdent(file, line, col, val) {
  return mkToken(file, line, col, val, 'IDENT');
}

function mkError(file, line, col, val) {
  return new Token(file, line, col, val, 'ERROR');
}

exports.Token    = Token;
exports.mkToken  = mkToken;
exports.mkDigits = mkDigits;
exports.mkIdent  = mkIdent;
exports.mkError  = mkError;

function Graph(symbols) {
  this.result = [];
  var that = this;
  _.each(symbols, function(sym) {
    if(sym in Symbols) {
      that.result.push({
        symbol: sym,
        value: Symbols[sym]
      });
    } else {
      throw 'Unknown symbol: ' + sym;
    }
  });
}
exports.Graph = Graph;

Graph.prototype.match = function(input) {
  return _.find(this.result, function(item) {
    if(input.substr(0, item.value.length) == item.value) {
      return true;
    } else {
      return false;
    }
  });
};

exports.Types = {
  DIGITS:  'DIGITS',
  IDENT:   'IDENT',
  NEWLINE: 'NEWLINE',
  WHITESPACE: 'WHITESPACE',
  SQUOTE:  'SQUOTE',
  DQUOTE:  'DQUOTE'
};

var Symbols = {
  NEWLINE: '\n',
  DOT:     '.',
  COLON:   ':',
  SEMI:    ';',
  COMMA:   ',',
  LPAREN:  '(',
  RPAREN:  ')',
  LBRACK:  '{',
  RBRACK:  '}',
  LBRACE:  '[',
  RBRACE:  ']',
  PLUS:    '+',
  MINUS:   '-',
  MULT:    '*',
  DIV:     '/',
  MOD:     '%',
  NEG:     '~',
  AMP:     '&',
  BAR:     '|',
  XOR:     '^',
  AT:      '@',
  POUND:   '#',
  DOLLAR:  '$',
  SLASH:   '\\',
  EQ:      '=',
  NOT:     '!',
  LT:      '<',
  GT:      '>',
  EQEQ:    '==',
  NTEQ:    '!=',
  LTEQ:    '<=',
  GTEQ:    '>='
};

})();

