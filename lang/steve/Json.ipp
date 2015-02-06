#include "Json.hpp"

namespace steve {
namespace json {

inline
Number::Number(double d) : value(d) { }

inline
Bool::Bool(bool b) : value(b) { }

} // namespace json
} // namespace steve