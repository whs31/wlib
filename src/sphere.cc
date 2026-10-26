#include <w/common/sphere.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Sphere::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Sphere const& sphere)
{
  return os << sphere.to_string();
}

W_COMMON_NAMESPACE_END
