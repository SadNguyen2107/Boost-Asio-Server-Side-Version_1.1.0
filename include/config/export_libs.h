#ifndef EXPORT_LIBS_H
#define EXPORT_LIBS_H

#ifdef _WIN32
// Windows specific
#define EXPORT_LIBS __declspec(dllexport)
#else
// Linux specific
#define EXPORT_LIBS __attribute__((visibility("default")))
#endif

#endif