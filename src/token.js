
(function(){

var formatter = require('./formatter');

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
  f.addPair('Value', this.value);
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

exports.Types = {
  NEWLINE: '\n',
  DOT: '.',
  COLON: ':',
  SEMI: ';',
  COMMA: ',',
  LPAREN: '(',
  RPAREN: ')',
  LBRACK: '{',
  RBRACK: '}',
  LBRACE: '[',
  RBRACE: ']',
  PLUS: '+',
  MINUS: '-',
  MULT: '*',
  DIV: '/',
  MOD: '%',
  NEG: '~',
  AND: '&',
  OR: '|',
  XOR: '^',
  EQ: '=',
  LT: '<',
  GT: '>',
  EQEQ: '==',
  NTEQ: '!=',
  LTEQ: '<=',
  GTEQ: '>='
};

})();

