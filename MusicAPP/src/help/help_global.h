#ifndef HELP_GLOBAL_H
#define HELP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(HELP_LIBRARY)
#  define HELPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define HELPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // HELP_GLOBAL_H
