#include <w/common/ray.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Ray::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Ray const& ray) { return os << ray.to_string(); }

W_COMMON_NAMESPACE_END
