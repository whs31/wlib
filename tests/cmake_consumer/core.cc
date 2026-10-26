#include <w/common/angle.h>

int main()
{
  auto const angle = w::common::Angle::from_degrees(180.0);
  return angle.to_string().empty() ? 1 : 0;
}
