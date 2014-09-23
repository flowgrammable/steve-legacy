
"use strict";

(function(){

var fmt = require('../utils/formatter');

function Value(v) {
  this.value = v;
}

function GroundType(t) {
}

function FuncType() {
}

function Unary(op, rhs) {
  this.operator = op;
  this.rhs      = rhs;
}

function Binary(op, lhs, rhs) {
  this.operator = op;
  this.lhs      = lhs;
  this.rhs      = rhs;
}

function Func(name, params, returnType, body) {
  this.name       = name;
  this.params     = params;
  this.returnType = returnType;
  this.body       = body;
}

function call(name, args) {
  this.name = name;
  this.args = args;
}

})();

