#include <w/common/vector2.h>
#include <w/common/vector3.h>

#include <qmetatype.h>
#include <qpoint.h>
#include <qstring.h>
#include <qvector2d.h>
#include <qvector3d.h>

#include <catch2/catch_test_macros.hpp>

using namespace w::common;

TEST_CASE("Geometry values round-trip Qt Gui types", "[geometry][qt]")
{
  REQUIRE(Vector2::from_qpointf(QPointF(1.0, 2.0)).to_qpointf() == QPointF(1.0, 2.0));
  REQUIRE(Vector2::from_qvector2d(QVector2D(3.0, 4.0)).to_qvector2d() == QVector2D(3.0, 4.0));
  REQUIRE(
    Vector3::from_qvector3d(QVector3D(1.0, 2.0, 3.0)).to_qvector3d() == QVector3D(1.0, 2.0, 3.0)
  );
  REQUIRE(Vector3::from_qpointf(QPointF(5.0, 6.0)).to_qpointf() == QPointF(5.0, 6.0));
  REQUIRE(Vector2::from_xy(1.0, 2.0).to_qstring() == QString("(1, 2)"));
  REQUIRE(Vector3::from_xyz(1.0, 2.0, 3.0).to_qstring() == QString("(1, 2, 3)"));

  REQUIRE(QMetaType::fromType<Vector2>().isValid());
  REQUIRE(QMetaType::fromType<Vector3>().isValid());
  REQUIRE(Vector3::staticMetaObject.indexOfProperty("lengthSquared") >= 0);
}
