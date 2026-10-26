#include <w/common/colour.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

std::string Rgba::to_string() const { return fmt::format("{}", *this); }

std::string Rgba8::to_hex() const
{
  if(this->m_a == 255)
    return fmt::format("#{:02x}{:02x}{:02x}", this->m_r, this->m_g, this->m_b);
  return fmt::format("#{:02x}{:02x}{:02x}{:02x}", this->m_r, this->m_g, this->m_b, this->m_a);
}

std::string Rgba8::to_string() const { return this->to_hex(); }

std::ostream& operator<<(std::ostream& os, Rgba const& colour) { return os << colour.to_string(); }

std::ostream& operator<<(std::ostream& os, Rgba8 const& colour) { return os << colour.to_string(); }

W_COMMON_NAMESPACE_END
