#ifndef GZ_ERROR_HPP
#define GZ_ERROR_HPP

// GL Check error
#define GL(func) Geez::glClearError(); \
    func; \
    Geez::glCheckError(#func, __FILE__, __LINE__)

namespace Geez
{
    void glClearError();
    bool glCheckError(const char* f, const char* file, const int line);
}

#endif