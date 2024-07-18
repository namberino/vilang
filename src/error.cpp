#include "error.hpp"

bool Error::has_error = false;
bool Error::has_runtime_error = false;

void Error::report(int line, const std::string& where, const std::string& msg)
{
    std::cout << "Dòng: " + std::to_string(line) + ", Lỗi tại" + where + ": " + msg + "\n";
    has_error = true;
}

void Error::error(int line, const std::string& msg)
{
    report(line, "", msg);
}

void Error::error(const Token& token, std::string msg)
{
    if (token.type == TOKEN_EOF)
        report(token.line, " ở cuối", msg);
    else
        report(token.line, " ở '" + token.lexeme + "'", msg);
}

RuntimeError::RuntimeError(const Token& token, std::string msg)
    : std::runtime_error(msg.data()), token(token) {}


void Error::runtime_error(const RuntimeError& error)
{
    std::cout << error.what() << "\nDòng " << error.token.line << "\n";
    has_runtime_error = true;
}
