#include <w/common/aabb.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string AABB::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, AABB const& box) { return os << box.to_string(); }

W_COMMON_NAMESPACE_END
