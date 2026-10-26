#include <w/common/quaternion.h>

#include <ostream>

#include <fmt/format.h>

W_COMMON_NAMESPACE_BEGIN

#ifdef W_COMMON_WITH_QT
Quaternion::Quaternion(QQuaternion const& quaternion) noexcept
  : m_w(quaternion.scalar())
  , m_x(quaternion.x())
  , m_y(quaternion.y())
  , m_z(quaternion.z())
{}

QQuaternion Quaternion::to_qquaternion() const noexcept
{
  return {
    static_cast<float>(this->m_w),
    static_cast<float>(this->m_x),
    static_cast<float>(this->m_y),
    static_cast<float>(this->m_z)
  };
}
#endif

std::string Quaternion::to_string() const { return fmt::format("{}", *this); }

#ifdef W_COMMON_WITH_QT
QString Quaternion::to_qstring() const { return QString::fromStdString(this->to_string()); }
#endif

#ifdef W_COMMON_WITH_QT
Quaternion Quaternion::from_qquaternion(QQuaternion const& quaternion) noexcept
{
  return Quaternion(quaternion);
}
#endif

std::ostream& operator<<(std::ostream& os, Quaternion const& quaternion)
{
  return os << quaternion.to_string();
}

W_COMMON_NAMESPACE_END
