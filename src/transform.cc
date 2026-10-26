#include <w/common/transform.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Transform::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Transform const& transform)
{
  return os << transform.to_string();
}

W_COMMON_NAMESPACE_END
