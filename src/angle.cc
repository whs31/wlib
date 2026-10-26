#include <w/common/angle.h>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Angle::to_string() const { return fmt::format("{:.6g} deg", this->degrees()); }

W_COMMON_NAMESPACE_END
