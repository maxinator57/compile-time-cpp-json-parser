#pragma once


#include <stdexcept>


namespace NCompileTimeJsonParser::NError {
    struct TParsingError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    struct TDereferencingError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}
