#include <w/common/longitude.h>

#include <private/sexagesimal.h>

W_COMMON_NAMESPACE_BEGIN

std::optional<Longitude> Longitude::parse(std::string_view const text) noexcept
{
  auto const parsed = _sexagesimal_impl::parse(text);
  if(not parsed.has_value() or parsed->hemisphere == 'N' or parsed->hemisphere == 'S')
    return std::nullopt;
  if(parsed->degrees < -180.0 or parsed->degrees > 180.0)
    return std::nullopt;
  return from_degrees(parsed->degrees);
}

std::string Longitude::to_string() const
{
  auto const no_hemisphere = this->m_degrees == 0.0 or this->m_degrees == -180.0;
  return _sexagesimal_impl::
    format_decimal(this->m_degrees, no_hemisphere ? '\0' : this->hemisphere_letter());
}

std::string Longitude::to_dms_string(int const second_decimals) const
{
  return _sexagesimal_impl::format_dms(this->m_degrees, second_decimals, 'E', 'W', 180.0);
}

W_COMMON_NAMESPACE_END
