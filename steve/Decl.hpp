
#ifndef STEVE_DECL_HPP
#define STEVE_DECL_HPP

namespace steve {

struct Name;
struct Decl;
struct Fn;

Name* get_name(Decl*);

Fn* get_declared_fn(Decl*);

} /// namespace steve

#endif
