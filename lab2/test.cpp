#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>

using namespace std;

// 定义终结符和非终结符
enum SymbolType
{
    TERMINAL,    // 终结符
    NON_TERMINAL // 非终结符
};

// 定义符号
struct Symbol
{
    string name;
    SymbolType type;
    int priority;
};

// 定义算符优先表
unordered_map<string, unordered_map<string, string>> op_table = {
    {"+", {{"+", "L"}, {"-", "L"}, {"*", "R"}, {"/", "R"}, {"(", "L"}, {")", "L"}, {"$", "R"}}},
    {"-", {{"+", "L"}, {"-", "L"}, {"*", "R"}, {"/", "R"}, {"(", "L"}, {")", "L"}, {"$", "R"}}},
    {"*", {{"+", "L"}, {"-", "L"}, {"*", "L"}, {"/", "L"}, {"(", "L"}, {")", "L"}, {"$", "R"}}},
    {"/", {{"+", "L"}, {"-", "L"}, {"*", "L"}, {"/", "L"}, {"(", "L"}, {")", "L"}, {"$", "R"}}},
    {"(", {{"+", "L"}, {"-", "L"}, {"*", "L"}, {"/", "L"}, {"(", "L"}, {")", "L"}, {"$", "N"}}},
    {")", {{"+", "R"},
           {"-", "R"},
           {"*", "R"},
           {"/", "R"},
           {"(", "N"},
           {")", "R"},
           {"$", {{"+", "R"}, {"-", "R"}, {"*", "R"}, {"/", "R"}, {"(", "N"}, {")", "R"}, {"$", "A"}}}};

// 定义运算符和操作数的栈
stack<Symbol> symbol_stack;

// 定义输入符号序列
string input = "3+4*2/(1-5)^2^3";

// 将输入字符串转换成符号序列
vector<Symbol> symbols;
for (int i = 0; i < input.size(); i++)
{
    if (isdigit(input[i]))
    {
        // 数字
        int j = i;
        while (j < input.size() && isdigit(input[j]))
        {
            j++;
        }
        string num_str = input.substr(i, j - i);
        Symbol num_symbol = {num_str, TERMINAL, 0};
        symbols.push_back(num_symbol);
        i = j - 1;
    }
    else
    {
        // 运算符
        string op_str = input.substr(i, 1);
        Symbol op_symbol = {op_str, TERMINAL, 0};
        symbols.push_back(op_symbol);
    }
}

// 向符号栈中压入$符号
Symbol end_symbol = {"$", TERMINAL, 0};
symbol_stack.push(end_symbol);

// 接上一段代码：

// 定义移进和规约操作
void shift(Symbol symbol)
{
    symbol_stack.push(symbol);
}

void reduce()
{
    // 查找可以规约的产生式
    bool found = false;
    while (!symbol_stack.empty())
    {
        Symbol top_symbol = symbol_stack.top();
        symbol_stack.pop();
        if (top_symbol.type == NON_TERMINAL)
        {
            for (auto &op_pair : op_table[top_symbol.name])
            {
                Symbol op_symbol = {op_pair.first, TERMINAL, 0};
                if (op_pair.second == "L" && symbol_stack.top().priority < op_symbol.priority)
                {
                    // 找到了可以规约的产生式
                    symbol_stack.push(top_symbol);
                    found = true;
                    break;
                }
                else if (op_pair.second == "R" && symbol_stack.top().priority <= op_symbol.priority)
                {
                    // 找到了可以规约的产生式
                    symbol_stack.push(top_symbol);
                    found = true;
                    break;
                }
            }
        }
        if (found)
        {
            break;
        }
    }
    // 根据产生式进行规约
    if (found)
    {
        vector<Symbol> reduce_symbols;
        while (!symbol_stack.empty() && symbol_stack.top().type != NON_TERMINAL)
        {
            reduce_symbols.push_back(symbol_stack.top());
            symbol_stack.pop();
        }
        if (!symbol_stack.empty() && symbol_stack.top().type == NON_TERMINAL)
        {
            Symbol reduce_symbol = symbol_stack.top();
            symbol_stack.pop();
            for (auto &op_pair : op_table[reduce_symbol.name])
            {
                if (op_pair.second == "N")
                {
                    // 找到了规约产生的非终结符
                    Symbol new_symbol = {op_pair.first, NON_TERMINAL, 0};
                    symbol_stack.push(new_symbol);
                    break;
                }
            }
        }
    }
}

// 逐个处理符号序列
for (auto &symbol : symbols)
{
    // 移进或规约
    while (!symbol_stack.empty())
    {
        Symbol top_symbol = symbol_stack.top();
        if (top_symbol.type == TERMINAL || symbol.priority > top_symbol.priority)
        {
            // 当前符号优先级高于栈顶符号，移进
            shift(symbol);
            break;
        }
        else
        {
            // 当前符号优先级低于或等于栈顶符号，规约
            reduce();
        }
    }
}