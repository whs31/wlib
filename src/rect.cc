#include <w/common/rect.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Rect::to_string() const { return fmt::format("{}", *this); }

std::string Recti::to_string() const { return fmt::format("{}", *this); }

std::ostream& operator<<(std::ostream& os, Rect const& rect) { return os << rect.to_string(); }

std::ostream& operator<<(std::ostream& os, Recti const& rect) { return os << rect.to_string(); }

W_COMMON_NAMESPACE_END
