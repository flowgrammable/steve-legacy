
(function(){

var formatter = require('../utils/formatter');
var token     = require('./token');

var digits     = /^[0-9]+/;
var ident      = /^[a-zA-Z_][a-zA-Z0-9_]*/;
var whitespace = /^[ \t]+/;
var newline    = /^[\n]/;
var squote     = /^'[^'\n]?'/;
var dquote     = /^"[^"\n]*"/;

function count(pattern, input) {
  var str = pattern.exec(input);
  if( str === null || str.length === 0) {
    return 0;
  } else {
    return str[0].length;
  }
}

function countDigits(input) {
  return count(digits, input);
}

function countIdent(input) {
  return count(ident, input);
}

function countWhitespace(input) {
  return count(whitespace, input);
}

function countNewline(input) {
  return count(newline, input);
}

function countSQuote(input) {
  return count(squote, input);
}

function countDQuote(input) {
  return count(dquote, input);
}

function take(countF, type, ctx) {
  var amt;
  if(typeof countF === 'function') {
    amt = countF(ctx.remainder);
  } else if(typeof countF === 'number') {
    amt = countF;
  } else {
    throw 'Unknown countF: ' + typeof countF;
  }
  if(amt > 0) {
    return token.mkToken(
      ctx.filename,
      ctx.lineno,
      ctx.colno,
      ctx.remainder.substr(0, amt),
      type
    );
  } else {
    return null;
  }
}

function takeDigits(ctx) {
  return take(countDigits, token.Types.DIGITS, ctx);
}

function takeIdent(ctx) {
  return take(countIdent, token.Types.IDENT, ctx);
}

function takeNewline(ctx) {
  return take(countNewline, token.Types.NEWLINE, ctx);
}

function takeSQuote(ctx) {
  return take(countSQuote, token.Types.SQUOTE, ctx);
}

function takeDQuote(ctx) {
  return take(countDQuote, token.Types.DQUOTE, ctx);
}

function takeSymbol(graph) {
  var _graph = graph;
  return function(ctx) {
    var m = _graph.match(ctx.remainder);
    if(m) {
      return take(m.value.length, m.symbol, ctx);
    } else {
      return null;
    }
  };
}

function takeWhitespace(ctx) {
  return take(countWhitespace, token.Types.WHITESPACE, ctx);
}

function takeUnknown(ctx) {
  return token.mkError(
    ctx.filename,
    ctx.lineno,
    ctx.colno,
    ctx.remainder
  );
}

function Context(filename, input) {
  this.filename = filename;
  this.lineno = 1;
  this.colno = 1;
  this.tokens = [];
  this.remainder = input;
}

function isDone(ctx) {
  return ctx.remainder.length === 0;
}

Context.prototype.trim = function() {
  var amt = countWhitespace(this.remainder);
  if(amt > 0) {
    this.colno += amt;
    this.remainder = this.remainder.slice(amt);
  }
};

Context.prototype.step = function(tok) {
  if(tok) {
    if(tok.type == token.Types.NEWLINE) {
      this.lineno += 1;
      this.colno = 1;
    } else {
      this.colno += tok.value.length;
      if(tok.type != token.Types.WHITESPACE) {
        this.tokens.push(tok);
      }
    }
    this.remainder = this.remainder.slice(tok.value.length);
  }
  return this;
};

function tokenize(tokenizers) {
  var _tokenizers = arguments;
  return function(ctx) {
    var idx;
    for(idx = 0; idx < _tokenizers.length; ++idx) {
      var tok = _tokenizers[idx](ctx);
      if(tok) { return tok; }
    }
    return null;
  };
}

function lex(f, input) {
  var ctx = new Context(f, input);
  while(!isDone(ctx)) {
    ctx.step(tokenize(
      takeWhitespace,
      takeDigits,
      takeIdent,
      takeNewline,
      takeSQuote,
      takeDQuote,
      takeSymbol(diGraph),
      takeSymbol(uniGraph),
      takeUnknown
    )(ctx));
  }

  return {
    filename: f,
    tokens: ctx.tokens
  };
}

var uniGraph = new token.Graph([
  'DOT',
  'EQ',
  'SEMI',
  'COMMA',
  'BAR',
  'COLON',
  'LPAREN',
  'RPAREN',
  'LBRACE',
  'RBRACE',
  'LBRACK',
  'RBRACK',
  'PLUS',
  'MINUS',
  'MULT',
  'DIV',
  'MOD',
  'NOT',
  'LT',
  'GT'
]);

var diGraph = new token.Graph([
  'EQEQ',
  'NTEQ',
  'LTEQ',
  'GTEQ'
]);

exports.lex = lex;

})();

