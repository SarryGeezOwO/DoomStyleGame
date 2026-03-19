#include "error.hpp"
#include "log.hpp"
#include <gl/glew.h>
#include <iomanip>
#include <sstream>
#include <unordered_map>

/*
Yes I am a stupid all developer, a lazy one as well
so you will have to look at the error here lmao

>> GL_INVALID_ENUM, 0x0500
Given when an enumeration parameter is not a legal enumeration for that function. This is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.

>> GL_INVALID_VALUE, 0x0501
Given when a value parameter is not a legal value for that function. This is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead.

>> GL_INVALID_OPERATION, 0x0502
Given when the set of state for a command is not legal for the parameters given to that command. It is also given for commands where combinations of parameters define what the legal parameters are.

>> GL_STACK_OVERFLOW, 0x0503
Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size.

>> GL_STACK_UNDERFLOW, 0x0504
Given when a stack popping operation cannot be done because the stack is already at its lowest point.

>> GL_OUT_OF_MEMORY, 0x0505
Given when performing an operation that can allocate memory, and the memory cannot be allocated. The results of OpenGL functions that return this error are undefined; it is allowable for partial execution of an operation to happen in this circumstance.

>> GL_INVALID_FRAMEBUFFER_OPERATION, 0x0506
Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete.

>> GL_CONTEXT_LOST, 0x0507 (with OpenGL 4.5 or ARB_KHR_robustness)
Given if the OpenGL context has been lost, due to a graphics card reset.

>> GL_TABLE_TOO_LARGE1, 0x8031
Part of the ARB_imaging extension.

*/

static const char* get_err_code_str(GLenum err_code) 
{
    switch(err_code) 
    {
    case GL_NO_ERROR:                         return "GL_NO_ERROR";
    case GL_INVALID_ENUM:                     return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:                    return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:                return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:                   return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:                  return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:                    return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:    return "GL_INVALID_FRAMEBUFFER_OPERATION";
#ifdef GL_CONTEXT_LOST
    case GL_CONTEXT_LOST:                     return "GL_CONTEXT_LOST";
#endif
    // Some older / extension codes (fallback to numeric if not defined)
#ifdef GL_TABLE_TOO_LARGE1
    case GL_TABLE_TOO_LARGE1:                 return "GL_TABLE_TOO_LARGE1";
#else
    case 0x8031:                              return "GL_TABLE_TOO_LARGE1";
#endif
    default:                                  return "Unknown GL error";
    }
} 

void Geez::glClearError()
{
    while(glGetError() != GL_NO_ERROR);
}

bool Geez::glCheckError(const char *f, const char *file, const int line)
{
    while(GLenum code = glGetError()) {
        std::stringstream ss;
        ss << std::hex << (int)code;
        
        GZ_LOG(GZ_FATAL, 
            "OpenGL Error:"
            "\n\t[Code: %d | 0x%s] << %s" 
            "\n\t[Function: %s]"
            "\n\t[Line: %d]"
            "\n\t[File: %s]"
            "\n--------------------"
        // Values
            ,(int)code, 
            ss.str().c_str(), 
            get_err_code_str(code),
            f, 
            line, 
            file
        );
        return false;
    }
    return true;
}
