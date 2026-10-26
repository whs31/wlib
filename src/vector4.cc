#include <w/common/vector4.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Vector4::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Vector4 const vector)
{
  return os << vector.to_string();
}

W_COMMON_NAMESPACE_END
