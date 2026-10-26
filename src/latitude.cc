#include <w/common/latitude.h>

#include <private/sexagesimal.h>

W_COMMON_NAMESPACE_BEGIN

std::optional<Latitude> Latitude::parse(std::string_view const text) noexcept
{
  auto const parsed = _sexagesimal_impl::parse(text);
  if(not parsed.has_value() or parsed->hemisphere == 'E' or parsed->hemisphere == 'W')
    return std::nullopt;
  auto const latitude = from_degrees(parsed->degrees);
  if(not latitude.is_valid())
    return std::nullopt;
  return latitude;
}

std::string Latitude::to_string() const
{
  return _sexagesimal_impl::
    format_decimal(this->m_degrees, this->m_degrees == 0.0 ? '\0' : this->hemisphere_letter());
}

std::string Latitude::to_dms_string(int const second_decimals) const
{
  return _sexagesimal_impl::format_dms(this->m_degrees, second_decimals, 'N', 'S', 0.0);
}

W_COMMON_NAMESPACE_END
