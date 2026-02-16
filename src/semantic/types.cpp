#include "druk/semantic/types.hpp"

namespace druk {

std::string type_to_string(Type type) {
  switch (type.kind) {
  case TypeKind::Int:
    return "Int";
  case TypeKind::String:
    return "String";
  case TypeKind::Bool:
    return "Bool";
  case TypeKind::Void:
    return "Void";
  case TypeKind::Function:
    return "Function";
  case TypeKind::Error:
    return "Error";
  }
  return "Unknown";
}

} // namespace druk
