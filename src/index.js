#!/usr/bin/env node

(function(){

var fs      = require('fs');
var program = require('commander');
var _       = require('underscore');

var lex = require('./lexer');

program
  .version('0.0.1')
  .option('-l, --lex', 'Display the lexeme list')
  .option('-p, --parse', 'Display the parse tree')
  .option('-t, --check', 'Display the elaboration tree')
  .option('-i, --interpret', 'Interpret the source')
  .option('-c, --ansi-c', 'Generate C output')
  .option('--svg', 'Generate SVG output')
  .option('--c++', 'Generate C++ output')
  .option('--c++11', 'Generate C++ output')
  .option('--python', 'Generate Python output')
  .option('--javascript', 'Generate Javascript output')
  .parse(process.argv);

var files = _.map(program.args, function(file) {
  return lex.lex(file, fs.readFileSync(file, 'utf8'));
});

_.each(files, function(item) {
  console.log(item.filename);
  console.log(item.tokens.toString());
});

})();

