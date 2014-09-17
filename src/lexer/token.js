
(function(){

var _         = require('underscore');
var formatter = require('../utils/formatter');

function Token(file, line, col, val, tp) {
  this.filename = file;
  this.lineno = line;
  this.colno = col;
  this.value = val;
  this.type = tp;
}

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

Token.prototype.toString = function() {
  var f, s;
  f = new formatter.Formatter();
  this.toFormatter(f);
  s = f.toString();
  return s;
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
  AND:     '&',
  OR:      '|',
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

