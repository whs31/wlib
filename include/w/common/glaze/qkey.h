#pragma once

#include <w/common/config.h>

#ifndef W_COMMON_WITH_QT
#  error "Qt glaze adapters require wlib to be built with with_qt=True"
#endif

#include <qnamespace.h>

#include <w/common/glaze/detail.h>

// `Qt::Key` is a plain enum, which glaze reads and writes as its integer value without any
// metadata.
