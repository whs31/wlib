#include <w/common/frustum.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Frustum::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Frustum const& frustum)
{
  return os << frustum.to_string();
}

W_COMMON_NAMESPACE_END
