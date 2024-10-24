//------------------------------------//
// Bản quyền © 2024 Nguyễn Bình Nam
// Được cấp phép theo Giấy phép Apache v2.0
//------------------------------------//

#include "interpreter.hpp"

Interpreter::Interpreter()
{
    // native functions
    globals->define("đồng_hồ", std::make_shared<NativeClock>());
    globals->define("thời_gian", std::make_shared<NativeTime>());
    globals->define("đầu_vào", std::make_shared<NativeInput>());
    globals->define("thoát", std::make_shared<NativeExit>());
    globals->define("chia_nguyên", std::make_shared<NativeFloorDiv>());
    globals->define("độ_dài", std::make_shared<NativeArrayLen>());
}

void Interpreter::interpret(const std::vector<std::shared_ptr<Stmt>>& statements)
{
    // interpret function, statement version
    try
    {
        for (const std::shared_ptr<Stmt>& statement : statements)
            execute(statement);
    }
    catch (RuntimeError error)
    {
        Error::runtime_error(error);
    }
}

std::string Interpreter::interpret(const std::shared_ptr<Expr>& expr)
{
    // interpret function, expression version
    try
    {
        std::any value = evaluate(expr);
        return stringify(value);
    }
    catch(RuntimeError error)
    {
        Error::runtime_error(error);
        return "";
    }
}

void Interpreter::resolve(std::shared_ptr<Expr> expr, int depth)
{
    // resolve local expression
    locals[expr] = depth;
}

std::any Interpreter::visitBlockStmt(std::shared_ptr<BlockStmt> stmt)
{
    // block statement evaluation
    execute_block(stmt->statements, std::make_shared<Environment>(environment));
    return {};
}

std::any Interpreter::visitExpressionStmt(std::shared_ptr<ExpressionStmt> stmt)
{
    // expression statement evaluation
    evaluate(stmt->expression);
    return {};
}

std::any Interpreter::visitPrintStmt(std::shared_ptr<PrintStmt> stmt)
{
    // print statement evaluation
    std::any value = evaluate(stmt->expression);
    std::cout << stringify(value) + "\n";
    return {};
}

std::any Interpreter::visitMutStmt(std::shared_ptr<MutStmt> stmt)
{
    // mut statement evaluation
    std::any value = nullptr;

    if (stmt->initializer != nullptr)
        value = evaluate(stmt->initializer);

    environment->define(stmt->name.lexeme, std::move(value));
    
    return {};
}

std::any Interpreter::visitIfStmt(std::shared_ptr<IfStmt> stmt)
{
    // if statement evaluation
    if (is_truthy(evaluate(stmt->condition)))
        execute(stmt->then_branch);
    else if (stmt->else_branch != nullptr)
        execute(stmt->else_branch);
    
    return {};
}

std::any Interpreter::visitWhileStmt(std::shared_ptr<WhileStmt> stmt)
{
    // while statement evaluation
    try
    {
        while (is_truthy(evaluate(stmt->condition)))
            execute(stmt->body);
    }
    catch (BreakException* bex)
    {
        // pass
    }

    return {};
}

std::any Interpreter::visitFunctionStmt(std::shared_ptr<FunctionStmt> stmt)
{
    // function statement evaluation
    std::string func_name = stmt->name.lexeme;
    auto function = std::make_shared<NblFunction>(func_name, stmt->fn, environment, false);
    environment->define(func_name, function);
    return {};
}

std::any Interpreter::visitReturnStmt(std::shared_ptr<ReturnStmt> stmt)
{
    // return statement evaluation
    std::any value = nullptr;

    if (stmt->value != nullptr)
        value = evaluate(stmt->value);

    throw NblReturn{value};
}

std::any Interpreter::visitBreakStmt(std::shared_ptr<BreakStmt> stmt)
{
    // break statement evaluation
    throw new BreakException();
}

std::any Interpreter::visitClassStmt(std::shared_ptr<ClassStmt> stmt)
{
    // class statement evaluation
    std::any superclass;
    if (stmt->superclass != nullptr)
    {
        superclass = evaluate(stmt->superclass);

        if (superclass.type() != typeid(std::shared_ptr<NblClass>))
            throw RuntimeError(stmt->superclass->name, "Lớp gốc phải là 1 lớp");
    }

    environment->define(stmt->name.lexeme, nullptr);

    if (stmt->superclass != nullptr)
    {
        environment = std::make_shared<Environment>(environment);
        environment->define("gốc", superclass);
    }

    std::map<std::string, std::shared_ptr<NblFunction>> methods;
    for (std::shared_ptr<FunctionStmt> method : stmt->methods)
    {
        bool is_method_init = method->name.lexeme == "khởi_tạo";
        auto function = std::make_shared<NblFunction>(stmt->name.lexeme, method->fn, environment, is_method_init);
        methods[method->name.lexeme] = function;
    }

    std::shared_ptr<NblClass> superklass = nullptr;
    if (superclass.type() == typeid(std::shared_ptr<NblClass>))
        superklass = std::any_cast<std::shared_ptr<NblClass>>(superclass);

    auto klass = std::make_shared<NblClass>(stmt->name.lexeme, superklass, methods);

    if (superklass != nullptr)
        environment = environment->enclosing;

    environment->assign(stmt->name, std::move(klass));

    return {};
}

std::any Interpreter::visitImportStmt(std::shared_ptr<ImportStmt> stmt)
{
    // import statement evaluation
    std::string target = std::any_cast<std::string>(stmt->target->value);
    run_file(target, *this);
    return {};
}


std::any Interpreter::visitAssignExpr(std::shared_ptr<AssignExpr> expr)
{
    // assign expression evaluation
    std::any value = evaluate(expr->value);
    // environment->assign(expr->name, value);

    auto element = locals.find(expr);

    if (element != locals.end())
    {
        int distance = element->second;
        environment->assign_at(distance, expr->name, value);
    }
    else
    {
        globals->assign(expr->name, value);
    }

    return value;
}

std::any Interpreter::visitBinaryExpr(std::shared_ptr<BinaryExpr> expr)
{
    // binary expression evaluation
    std::any left = evaluate(expr->left);
    std::any right = evaluate(expr->right);

    switch (expr->op.type)
    {
        // comparisors
        case BANG_EQUAL: return !is_equal(left, right);
        case EQUAL_EQUAL: return is_equal(left, right);
        case GREATER:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) > std::any_cast<double>(right);
        case GREATER_EQUAL:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) >= std::any_cast<double>(right);
        case LESS:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) < std::any_cast<double>(right);
        case LESS_EQUAL:
            check_num_operands(expr->op, left, right);
            return std::any_cast<double>(left) <= std::any_cast<double>(right);
        case STAR_STAR:
            check_num_operands(expr->op, left, right);
            return pow(std::any_cast<double>(left), std::any_cast<double>(right));

        // arithmetics
        case PLUS: case PLUS_EQUAL:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) + std::any_cast<double>(right);

            if (left.type() == typeid(std::string) && right.type() == typeid(std::string))
                return std::any_cast<std::string>(left) + std::any_cast<std::string>(right);

            if (left.type() == typeid(std::string) && right.type() == typeid(double))
                return std::any_cast<std::string>(left) + int_or_double(right);

            if (left.type() == typeid(double) && right.type() == typeid(std::string))
                return int_or_double(left) + std::any_cast<std::string>(right);

            throw RuntimeError{expr->op, "Các phần tử cho phép tính cộng chỉ có thể là 2 số, 2 chuỗi, hoặc 1 số và 1 chuỗi"};
        case MINUS: case MINUS_EQUAL:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) - std::any_cast<double>(right);
        case STAR: case STAR_EQUAL:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) * std::any_cast<double>(right);
        case SLASH: case SLASH_EQUAL:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return std::any_cast<double>(left) / std::any_cast<double>(right);
        case PERCENT:
            if (left.type() == typeid(double) && right.type() == typeid(double))
                return fmod(std::any_cast<double>(left), std::any_cast<double>(right));
    
        default: break;
    }

    return {}; // unreachable, here to make the compiler happy
}

std::any Interpreter::visitGroupingExpr(std::shared_ptr<GroupingExpr> expr)
{
    // parentheses evaluation
    return evaluate(expr->expression);
}

std::any Interpreter::visitLiteralExpr(std::shared_ptr<LiteralExpr> expr)
{
    // literal expresison evaluation
    return expr->value;
}

std::any Interpreter::visitUnaryExpr(std::shared_ptr<UnaryExpr> expr)
{
    // unary expression evaluation
    std::any right = evaluate(expr->right);

    switch (expr->op.type)
    {
        case BANG:
            return !is_truthy(right);

        case MINUS:
            check_num_operand(expr->op, right);
            return -std::any_cast<double>(right);
        
        default:
            break;
    }

    return {}; // unreachable, here to make the compiler happy
}

std::any Interpreter::visitMutExpr(std::shared_ptr<MutExpr> expr)
{
    // mutable expression evaluation
    return lookup_mut(expr->name, expr);
}

std::any Interpreter::visitLogicalExpr(std::shared_ptr<LogicalExpr> expr)
{
    // logical expression evaluation
    std::any left = evaluate(expr->left);

    if (expr->op.type == OR)
    {
        if (is_truthy(left))
            return left;
    }
    else
    {
        if (!is_truthy(left))
            return left;
    }

    return evaluate(expr->right);
}

std::any Interpreter::visitCallExpr(std::shared_ptr<CallExpr> expr)
{
    // call expression evaluation
    std::any callee = evaluate(expr->callee);
    std::vector<std::any> arguments;

    for (const std::shared_ptr<Expr>& argument : expr->arguments)
        arguments.push_back(evaluate(argument));

    // pointers in a std::any wrapper must be unwrapped before they can be cast
    std::shared_ptr<NblCallable> function;

    if (callee.type() == typeid(std::shared_ptr<NblFunction>))
    {
        function = std::any_cast<std::shared_ptr<NblFunction>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NblClass>))
    {
        function = std::any_cast<std::shared_ptr<NblClass>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeClock>))
    {
        function = std::any_cast<std::shared_ptr<NativeClock>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeTime>))
    {
        function = std::any_cast<std::shared_ptr<NativeTime>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeInput>))
    {
        function = std::any_cast<std::shared_ptr<NativeInput>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeExit>))
    {
        std::shared_ptr<NativeExit> func = std::any_cast<std::shared_ptr<NativeExit>>(callee);
        func->param_count = arguments.size() > 0 ? 1 : 0;
        function = func;
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeFloorDiv>))
    {
        function = std::any_cast<std::shared_ptr<NativeFloorDiv>>(callee);
    }
    else if (callee.type() == typeid(std::shared_ptr<NativeArrayLen>))
    {
        function = std::any_cast<std::shared_ptr<NativeArrayLen>>(callee);
    }
    else
    {
        throw RuntimeError(expr->paren, "Chỉ có thể gọi hàm");
    }

    if (arguments.size() != function->arity())
        throw RuntimeError(expr->paren, "Cần nhận " + std::to_string(function->arity()) + " tham số nhưng đã nhận được " + std::to_string(arguments.size()) + " tham số");

    return function->call(*this, std::move(arguments));
}

std::any Interpreter::visitFunctionExpr(std::shared_ptr<FunctionExpr> expr)
{
    // function expression evaluation
    return std::make_shared<NblFunction>("", expr, environment, false);
}

std::any Interpreter::visitGetExpr(std::shared_ptr<GetExpr> expr)
{
    // get expression evaluation
    std::any object = evaluate(expr->object);

    if (object.type() == typeid(std::shared_ptr<NblInstance>))
        return std::any_cast<std::shared_ptr<NblInstance>>(object)->get(expr->name);

    throw RuntimeError(expr->name, "Chỉ thực thể mới có trường dữ liệu");
}

std::any Interpreter::visitSetExpr(std::shared_ptr<SetExpr> expr)
{
    // set expression evaluation
    std::any object = evaluate(expr->object);

    if (object.type() != typeid(std::shared_ptr<NblInstance>))
        throw RuntimeError(expr->name, "Chỉ thực thể mới có trường dữ liệu");

    std::any value = evaluate(expr->value);
    std::any_cast<std::shared_ptr<NblInstance>>(object)->set(expr->name, value);

    return value;
}

std::any Interpreter::visitThisExpr(std::shared_ptr<ThisExpr> expr)
{
    // this expression evaluation
    return lookup_mut(expr->keyword, expr);
}

std::any Interpreter::visitSuperExpr(std::shared_ptr<SuperExpr> expr)
{
    // super expression evaluation
    int distance = locals[expr];
    auto superclass = std::any_cast<std::shared_ptr<NblClass>>(environment->get_at(distance, "gốc"));
    auto obj = std::any_cast<std::shared_ptr<NblInstance>>(environment->get_at(distance - 1, "đây"));
    std::shared_ptr<NblFunction> method = superclass->find_method(expr->method.lexeme);

    if (method == nullptr) // can't find method
        throw RuntimeError(expr->method, "Hàm chưa được định nghĩa: '" + expr->method.lexeme + "'");

    return method->bind(obj);
}

std::any Interpreter::visitListExpr(std::shared_ptr<ListExpr> expr)
{
    // list expression evaluation
    auto list = std::make_shared<ListType>();

    for (std::shared_ptr<Expr>& value : expr->elements)
        list->append(evaluate(value));

    return list;
}

std::any Interpreter::visitSubscriptExpr(std::shared_ptr<SubscriptExpr> expr)
{
    // subscript expression evaluation
    std::any name = evaluate(expr->name);
    std::any index = evaluate(expr->index);

    if (name.type() == typeid(std::shared_ptr<ListType>))
    {
        if (index.type() == typeid(double))
        {
            std::shared_ptr<ListType> list = std::any_cast<std::shared_ptr<ListType>>(name);
            int casted_index = std::any_cast<double>(index);

            if (expr->value != nullptr)
            {
                std::any value = evaluate(expr->value);

                if (list->set_element_at(casted_index, value))
                    return value;
                else
                    throw RuntimeError(expr->paren, "Chỉ mục lớn hơn so với độ dài mảng");
            }
            else
            {
                if (casted_index >= list->get_length() || casted_index < 0)
                    // throw RuntimeError(expr->paren, "Index out of bound");
                    return nullptr;
                return list->get_element_at(casted_index);
            }
        }
        else
        {
            throw RuntimeError(expr->paren, "Chỉ mục phải là số nguyên");
        }
    }
    else
    {
        throw RuntimeError(expr->paren, "Chỉ có thể truy cập các yếu tố trong mảng");
    }

    return {};
}


std::any Interpreter::lookup_mut(const Token& name, std::shared_ptr<Expr> expr)
{
    // find variable in local or global environment
    auto element = locals.find(expr);

    if (element != locals.end())
    {
        int distance = element->second;
        return environment->get_at(distance, name.lexeme);
    }
    else
    {
        return globals->get(name);
    }
}

std::any Interpreter::evaluate(std::shared_ptr<Expr> expr)
{
    // send expression back into interpreter's visitor methods for evaluation
    return expr->accept(*this);
}

void Interpreter::execute(std::shared_ptr<Stmt> stmt)
{
    // send statement back into interpreter's visitor methods for evaluation
    stmt->accept(*this);
}

void Interpreter::execute_block(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment)
{
    // execute a given block of statements
    std::shared_ptr<Environment> previous_env = this->environment;

    try
    {
        this->environment = environment;

        for (const std::shared_ptr<Stmt>& statement : statements)
            execute(statement);
    }
    catch (...)
    {
        this->environment = previous_env;
        throw;
    }

    this->environment = previous_env;
}

void Interpreter::check_num_operand(const Token& op, const std::any& operand)
{
    // check if operand is a number
    if (operand.type() == typeid(double))
        return;
    
    throw RuntimeError{op, "Phần tử phải là 1 số"};
}

void Interpreter::check_num_operands(const Token& op, const std::any& left, const std::any& right)
{
    // check if operands are numbers
    if (left.type() == typeid(double) && right.type() == typeid(double))
        return;
    
    throw RuntimeError{op, "Các phần tử phải là số"};
}

bool Interpreter::is_truthy(const std::any& obj)
{
    // handle boolean values
    if (obj.type() == typeid(nullptr))
        return false;
    
    if (obj.type() == typeid(bool))
        return std::any_cast<bool>(obj);
    
    return true;
}

bool Interpreter::is_equal(const std::any& obj1, const std::any& obj2)
{
    // handle equality
    if (obj1.type() == typeid(nullptr) && obj2.type() == typeid(nullptr))
        return true;

    if (obj1.type() == typeid(nullptr))
        return false;

    if (obj1.type() == typeid(std::string) && obj2.type() == typeid(std::string))
        return std::any_cast<std::string>(obj1) == std::any_cast<std::string>(obj2);

    if (obj1.type() == typeid(double) && obj2.type() == typeid(double))
        return std::any_cast<double>(obj1) == std::any_cast<double>(obj2);

    if (obj1.type() == typeid(bool) && obj2.type() == typeid(bool))
        return std::any_cast<bool>(obj1) == std::any_cast<bool>(obj2);

    return false;
}

std::string Interpreter::int_or_double(const std::any& obj)
{
    // checking for weather the object is an int or a double
    std::string text;

    if (std::any_cast<double>(obj) == static_cast<int>(std::any_cast<double>(obj)))
    {
        // is an integer
        text = std::to_string(static_cast<int>(std::any_cast<double>(obj)));
    }
    else
    {
        // is a double
        text = std::to_string(std::any_cast<double>(obj));

        if (text[text.length() - 2] == '.' && text[text.length() - 1] == '0')
            text = text.substr(0, text.length() - 2);
    }

    return text;
}

std::string Interpreter::stringify(const std::any& obj)
{
    // stringifying each types
    if (obj.type() == typeid(nullptr))
        return "trống";

    if (obj.type() == typeid(double))
        return int_or_double(obj);

    if (obj.type() == typeid(std::string))
        return std::any_cast<std::string>(obj);

    if (obj.type() == typeid(bool))
        return std::any_cast<bool>(obj) ? "đúng" : "sai";
    
    if (obj.type() == typeid(std::shared_ptr<NblFunction>))
        return std::any_cast<std::shared_ptr<NblFunction>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NblClass>))
        return std::any_cast<std::shared_ptr<NblClass>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NblInstance>))
        return std::any_cast<std::shared_ptr<NblInstance>>(obj)->to_string();
    
    if (obj.type() == typeid(std::shared_ptr<NativeClock>))
        return std::any_cast<std::shared_ptr<NativeClock>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NativeTime>))
        return std::any_cast<std::shared_ptr<NativeTime>>(obj)->to_string();
    
    if (obj.type() == typeid(std::shared_ptr<NativeInput>))
        return std::any_cast<std::shared_ptr<NativeInput>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NativeExit>))
        return std::any_cast<std::shared_ptr<NativeExit>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NativeFloorDiv>))
        return std::any_cast<std::shared_ptr<NativeFloorDiv>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<NativeArrayLen>))
        return std::any_cast<std::shared_ptr<NativeArrayLen>>(obj)->to_string();

    if (obj.type() == typeid(std::shared_ptr<ListType>))
    {
        std::string result = "[";
        std::shared_ptr<ListType> list = std::any_cast<std::shared_ptr<ListType>>(obj);
        std::vector<std::any> elements = list->elements;

        for (auto i = elements.begin(); i != elements.end(); i++)
        {
            auto next = i + 1;

            result.append(stringify(*i));

            if (next != elements.end())
                result.append(", ");
        }

        result.append("]");
        return result;
    }

    return "Lỗi trong hàm stringify: Loại đối tượng không hợp lệ";
}
