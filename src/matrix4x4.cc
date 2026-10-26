#include <w/common/matrix4x4.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Matrix4x4::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Matrix4x4 const& matrix)
{
  return os << matrix.to_string();
}

W_COMMON_NAMESPACE_END
