
# Build configuration.
include_directories(.. ${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARY_DIR})

set(cppformat_src contrib/cppformat/format.cc)
set(contrib_src ${cppformat_src})

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
  ${contrib_src}
)
set_target_properties(steve-lib PROPERTIES ARCHIVE_OUTPUT_NAME steve)
target_link_libraries(steve-lib gc gmp boost_system boost_filesystem)

add_executable(steve Main.cpp Cli.cpp)
target_link_libraries(steve steve-lib)

add_subdirectory(Token.test)
add_subdirectory(Ast.test)
