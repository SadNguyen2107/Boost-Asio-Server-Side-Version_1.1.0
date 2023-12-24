#ifndef IMPORT_LIBS_H
#define IMPORT_LIBS_H

#ifdef _WIN32
// Windows specific
#define IMPORT_LIBS __declspec(dllimport)
#else
// Linux specific
#define IMPORT_LIBS
#endif

#endif