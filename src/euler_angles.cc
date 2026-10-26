#include <w/common/euler_angles.h>
#include <w/common/quaternion.h>

#include <ostream>

#include <fmt/format.h>

#ifdef W_COMMON_WITH_QT
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

EulerAngles EulerAngles::from_quaternion(Quaternion const& attitude) noexcept
{
  return attitude.to_yaw_pitch_roll();
}

Quaternion EulerAngles::to_quaternion() const noexcept
{
  return Quaternion::from_yaw_pitch_roll(*this);
}

std::string EulerAngles::to_string() const
{
  return fmt::format("roll {}, pitch {}, yaw {}", this->m_roll, this->m_pitch, this->m_yaw);
}

#ifdef W_COMMON_WITH_QT
QString EulerAngles::to_qstring() const { return QString::fromStdString(this->to_string()); }
#endif

std::ostream& operator<<(std::ostream& os, EulerAngles const& angles)
{
  return os << angles.to_string();
}

W_COMMON_NAMESPACE_END
