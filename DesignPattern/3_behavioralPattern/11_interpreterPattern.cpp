#include <iostream>
#include <memory>
#include <unordered_map>
using namespace std;

class Context {
private:
    unordered_map<string, int> variables;

public:
    void setVariable(const string& var, int value) {
        variables[var] = value;
    }

    int getVariable(const string& var) const {
        return variables.at(var); // 使用at()进行安全访问
    }
};


// 抽象表达式接口
class Expression {
public:
    virtual ~Expression() = default;
    virtual int interpret(const Context& context) = 0;
};


// 终结符表达式：变量
class VariableExpression : public Expression {
private:
    string varName;

public:
    explicit VariableExpression(string var) : varName(move(var)) {}

    int interpret(const Context& context) override {
        return context.getVariable(varName);
    }
};

// 终结符表达式：数字常量
class NumberExpression : public Expression {
private:
    int number;

public:
    explicit NumberExpression(int num) : number(num) {}

    int interpret(const Context& context) override {
        return number;
    }
};

// 非终结符表达式：加法操作
class AddExpression : public Expression {
private:
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;

public:
    AddExpression(unique_ptr<Expression> l, unique_ptr<Expression> r)
        : left(move(l)), right(move(r)) {}

    int interpret(const Context& context) override {
        return left->interpret(context) + right->interpret(context);
    }
};

// 非终结符表达式：减法操作
class SubtractExpression : public Expression {
private:
    unique_ptr<Expression> left;
    unique_ptr<Expression> right;

public:
    SubtractExpression(unique_ptr<Expression> l, unique_ptr<Expression> r)
        : left(move(l)), right(move(r)) {}

    int interpret(const Context& context) override {
        return left->interpret(context) - right->interpret(context);
    }
};

int main() {
    Context context;
    context.setVariable("x", 10);
    context.setVariable("y", 5);

    // 构建表达式树：(x + 20) - (y + 5)
    auto expr = make_unique<SubtractExpression>(
        make_unique<AddExpression>(
            make_unique<VariableExpression>("x"),
            make_unique<NumberExpression>(20)
        ),
        make_unique<AddExpression>(
            make_unique<VariableExpression>("y"),
            make_unique<NumberExpression>(5)
        )
    );

    int result = expr->interpret(context);
    cout << "计算结果: " << result << endl; // 输出：20 = (10+20)-(5+5)

    return 0;
}