"use strict";

(function(){

var fmt  = require('../utils/formatter');
var token = require('../lexer/token');
var tree = require('./tree');

function Context(tokens) {
  this.tree      = [];
  this.remainder = tokens;
}

function consume(symbol, ctx) {
  if(ctx.remainder[0].type === symbol) {
    ctx.remainder = ctx.remainder.splice(1);
    return symbol;
  } else {
    return null;
  }
}

function parse(symbol, cons, ctx) {
  var node;
  if(ctx.remainder[0].type === symbol) {
    node = cons(ctx.remainder[0]);
    ctx.remainder = ctx.remainder.splice(1);
    return node;
  } else {
    return null;
  }
}

function parseToplevel(ctx) {
  if(!consume('LPAREN', ctx)) {
    console.log(ctx.remainder[0].type);
    throw 'expecting lparen';
  }
  if(!consume('RPAREN', ctx)) {
    console.log(ctx.remainder[0]);
    throw 'expecting rparen';
  }
}

/*
 * exprs ::= expr ',' exprs
 *         | expr
 *
 * expr ::= 
 *
 * eqExpr ::= blah '==' relExpr
 *          | blah '!=' relExpr
 *          | blah
 *
 * relExpr ::= blah '<' relExpr
 *           | blah '>' relExpr
 *           | blah '<=' relExpr
 *           | blah '>=' relExpr
 *           | blah
 *
 * arithExpr ::= multExpr '+' arithExpr
 *             | multExpr '-' arithExpr
 *             | multExpr
 *
 * multExpr ::= unary '*' multExpr
 *            | unary '/' multExpr
 *            | unary '%' multExpr
 *            | unary
 *
 * unaryExpr ::= '-' primary
 *             | primary
 *
 * primary ::= '(' expression ')'
 *           | value '[' expr ']
 *           | value '.' expr
 *           | value '(' exprs ')'
 *           | value
 *
 */

exports.parse = function(lexeme) {
  var ctx = new Context(lexeme.tokens);
  while(ctx.remainder.length > 0) {
    parseToplevel(ctx);
  }
  return ctx.tree;
}

})();

