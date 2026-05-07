#pragma once

#include <QtCore/QtGlobal>

#if defined(QT_ANT_DESIGN_STATIC_DEFINE)
#  define QT_ANT_DESIGN_EXPORT
#elif defined(QT_ANT_DESIGN_LIBRARY)
#  define QT_ANT_DESIGN_EXPORT Q_DECL_EXPORT
#else
#  define QT_ANT_DESIGN_EXPORT Q_DECL_IMPORT
#endif
