
#include <steve/Node.hpp>
#include <steve/Debug.hpp>

#include <unordered_map>

namespace steve {

namespace {

// Global table of node names. This is used primarily for debugging
// purposes.
static std::unordered_map<Node_kind, String> node_names_;

} // namespace

// Return a name associated with the node category.
String
node_name(Node_kind k) {
  auto iter = node_names_.find(k);
  if (iter != node_names_.end())
    return iter->second;
  else
    return "<unknown node>";
}

// Register the given name with the node category.
void
init_node(Node_kind k, const char* s) {
  steve_assert(node_names_.count(k) == 0, 
               format("node kind '{}' already named", s));
  node_names_.insert({k, s});
}

} // namespace steve
