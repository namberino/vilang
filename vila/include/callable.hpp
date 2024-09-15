//------------------------------------//
// Bản quyền © 2024 Nguyễn Bình Nam
// Được cấp phép theo Giấy phép Apache v2.0
//------------------------------------//

#ifndef CALLABLE_HPP
#define CALLABLE_HPP

#pragma once
#include <vector>
#include <any>
#include <string>

class Interpreter;

class NblCallable
{
    public:
        virtual ~NblCallable() = default;
        virtual int arity() = 0;
        virtual std::any call(Interpreter& interpreter, std::vector<std::any> arguments) = 0;
        virtual std::string to_string() = 0;
};

#endif
