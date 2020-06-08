#pragma once
#include "strfmt.h"
#include <iostream>
#include <string>
#include <GL/gl.h>

namespace console_log
{
    template <typename... Args>
    void inline cl_err_out(uint32_t line, const std::string& file, const std::string& fmt, Args&& ...args)
    {
        auto msg = stringfmt(fmt, std::forward<Args>(args)...);
        std::cerr << stringfmt("File: %s, line: %u\n%s", file.c_str(), line, msg.c_str()) << std::endl;
    }

    GLenum glCheckError_(uint32_t line, const std::string& file)
    {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::string error;
            switch (errorCode)
            {
                case GL_INVALID_ENUM:
                    error = "INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    error = "INVALID_VALUE";
                    break;
                case GL_INVALID_OPERATION:
                    error = "INVALID_OPERATION";
                    break;
                case GL_STACK_OVERFLOW:
                    error = "STACK_OVERFLOW";
                    break;
                case GL_STACK_UNDERFLOW:
                    error = "STACK_UNDERFLOW";
                    break;
                case GL_OUT_OF_MEMORY:
                    error = "OUT_OF_MEMORY";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error = "INVALID_FRAMEBUFFER_OPERATION";
                    break;
            }
            cl_err_out(line, file, "Error %s (code: %d).", error, errorCode);
        }
        return errorCode;
    }
}

#ifdef NO_CMESSAGE
    #define CMESSAGE(FMT, args...)
    #define glCheckError()
#else
    //must use macros, so __LINE__/__FILE__ are expanded to values where it is used
    //https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html  - this trick with ## to accept 0 args too because otherwise have extra coma
    #define CMESSAGE(FMT, ...) console_log::cl_err_out(__LINE__, __FILE__, (FMT), ##__VA_ARGS__)
    #define glCheckError() console_log::glCheckError_(__LINE__, __FILE__)
#endif
