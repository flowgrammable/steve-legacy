
include_directories(.. ${GMP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

set(cppformat_src contrib/cppformat/format.cc)
set(contrib_src ${cppformat_src})

# TODO: Factor this into smaller libraries (core, lang, cli, etc.).
add_library(steve-lib
  Meta.cpp
  Debug.cpp
  Algorithm.cpp
  Memory.cpp
  String.cpp
  Integer.cpp
  Format.cpp
  Print.cpp
  Error.cpp
  File.cpp
  Location.cpp
  Token.cpp
  Comment.cpp
  Node.cpp
  Syntax.cpp
  Ast.cpp
  Same.cpp
  Value.cpp
  Type.cpp
  Decl.cpp
  Scope.cpp
  Overload.cpp
  Variant.cpp
  Module.cpp
  Subst.cpp
  Conv.cpp
  Intrinsic.cpp
  Language.cpp
  Config.cpp
  Lexer.cpp
  Parser.cpp
  Elaborator.cpp
  Evaluator.cpp
  Extract.cpp
  JSON.cpp
  ${contrib_src}
)
set_target_properties(steve-lib PROPERTIES ARCHIVE_OUTPUT_NAME steve)
target_link_libraries(steve-lib ${GMP_LIBRARIES} ${Boost_LIBRARIES})

add_executable(steve Main.cpp Cli.cpp)
target_link_libraries(steve steve-lib)

# add_subdirectory(Token.test)
# add_subdirectory(Ast.test)
