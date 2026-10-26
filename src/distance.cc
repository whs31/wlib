#include <w/common/distance.h>

#include <cmath>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Distance::to_string() const
{
  auto const magnitude = std::abs(this->magnitude());
  if(magnitude < Real {0.1})
    return fmt::format("{:.6g} mm", this->as_mm());
  if(magnitude < 1)
    return fmt::format("{:.6g} cm", this->as_cm());
  if(magnitude < 1'000)
    return fmt::format("{:.6g} m", this->as_m());
  return fmt::format("{:.6g} km", this->as_km());
}

W_COMMON_NAMESPACE_END
