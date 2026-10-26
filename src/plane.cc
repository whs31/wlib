#include <w/common/plane.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Plane::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Plane const& plane) { return os << plane.to_string(); }

W_COMMON_NAMESPACE_END
