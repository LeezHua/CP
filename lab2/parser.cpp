#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>

using namespace std;


// 算符优先表
//----------------------------------
//     +   -   *   /   (   )   i   #
// +   >   >   <   <   <   >   <   >
// -   >   >   <   <   <   >   <   >
// *   >   >   >   >   <   >   <   >
// /   >   >   >   >   <   >   <   >
// (   <   <   <   <   <   =   <   ?
// )   >   >   >   >   ?   >   ?   >
// i   >   >   >   >   ?   >   ?   >
// #   <   <   <   <   <   ?   <   =
//----------------------------------

// 算符优先文法
// E->E+T|E-T|T
// T->T*F|T/F|F
// F->(E)|i


#define N 8
#define inf 0x3f3f3f3f

const char sp = 32, cr = 10;

string operators[N] = {"+", "-", "*", "/", "(", ")", "i", "#"};
int grade[N][N] = {
	{1, 1, -1, -1, -1, 1, -1, 1},
	{1, 1, -1, -1, -1, -1, 1, -1},
	{1, 1, 1, 1, -1, 1, -1, 1},
	{1, 1, 1, 1, -1, 1, -1, 1},
	{-1, -1, -1, -1, -1, 0, -1, inf},
	{1, 1, 1, 1, inf, 1, inf, 1},
	{1, 1, 1, 1, inf, 1, inf, 1},
	{-1, -1, -1, -1, -1, inf, -1, 0}
};

bool is_operator(string& str) {
	for(int i = 0; i < N; ++i)
		if(operators[i] == str)
			return true;
	return false;
}


set<string> NT;
map<string, string> grammer_left;

bool is_NT(string& str) {
	return NT.find(str) != NT.end();
}

void grammer_init() {
	NT.insert("E");
	NT.insert("T");
	NT.insert("F");

	// 最左素短语
	grammer_left["#T#"] = "E";
	grammer_left["#F#"] = "E";
	grammer_left["F+F"] = "E";
	grammer_left["F-F"] = "E";
	grammer_left["F*F"] = "T";
	grammer_left["F/F"] = "T";
	grammer_left["(E)"] = "F";
	grammer_left["i"] = "F";
}

// 查每个算符的 id
int id(string& str) {
	if (str == "#")
		return 7;
	for(int i = 0; i < 6; ++i) {
		if (str == operators[i])
			return i;
	}
	return 6;
}

enum token_type {
	keyword,
	operate,
	delimiter,
	identifier,
	label,
	literal,
	number,
};

string token_type_name[] = {
	"keyword",
	"operator",
	"delimiter",
	"identifier",
	"label",
	"literal",
	"number"
};

struct token {
	string::size_type row, col;
	token_type type;
	string value;

    token() {}
    token(string& str) {
		vector<int> ps;
		for(int i = 0; i < str.length(); ++i) {
			if(str[i] == ',') {
				ps.emplace_back(i);
			}

		}
		row = stoi(str.substr(1, ps[0] - 1));
		col = stoi(str.substr(ps[0] + 2, ps[1] - ps[0] - 2));
		int type_id = -1;
		for(int i = 0; i < 7; ++i) {
			string t = str.substr(ps[1] + 2, ps[2] - ps[1] - 2);
			if(t ==  token_type_name[i]) {
				type_id = i;
			}
		}
		type = token_type(type_id);
		value = str.substr(ps[2] + 2, str.length() - 1 - ps[2] - 2);
    }

	friend ostream& operator << (ostream& os, token& t) {
		os << "[" << t.row << ", " << t.col << ", " << token_type_name[t.type] << ", " << t.value << "]";
		return os;
	}
};

ifstream ifs = ifstream("token.txt", ios::in);

void error();

int parser() {
	vector<string> input;

	string str;
	while(getline(ifs, str)) {
		token tk = token(str);
		if(tk.value == "(" || tk.value == ")")
			input.emplace_back(tk.value);
		else if(tk.type == number || tk.type == operate || tk.type == identifier)
			input.emplace_back(tk.value);
		else
			break;
	}

	for(int i = 0; i < 50; ++i) cout << '-';
	cout << cr;
	cout << "表达式: ";
	for(string& str : input)
		cout << str;
	cout << cr;

	input.emplace_back("#");
	string stack[128] = {"#"};
	int top = 1;

	cout << left << setw(20) << "符号栈";
	cout << left << setw(20) << "输入串";
	cout << left << setw(16) << "操作";
	cout << left << setw(20) << "规约式";
	cout << cr;

	for(int p = 0; p < input.size(); ) {
		// 输出符号栈
		str.clear();
		for(int i = 0; i < top; ++i)
			str += stack[i];
		cout << left << setw(17) << str;

		// 输出输入串
		str.clear();
		for(int i = p; i < input.size(); ++i)
			str += input[i];
		cout << left << setw(17) << str;

		// 输出操作，以及可能的规约串
		
		// 寻找符号栈最右端的非终结符
		string right_t;
		for(int i = top -1; i >= 0; --i)
			// 没有在终结符集合中找到，说明是非终结符
			if(!is_NT(stack[i])) {
				right_t = stack[i];
				break;
			}
		// 对应的优先关系 (a, b) = <
		if(grade[id(right_t)][id(input[p])] == -1) {
			cout << left << setw(10) << "移进";
			stack[top++] = input[p];
			++p;
		}
		// 对应的优先关系 (a, b) = >
		else if(grade[id(right_t)][id(input[p])] == 1) {
			cout << left << setw(16) << "规约";
			// 寻找可规约串
			string expr, left_t;
			bool f = false;
			for(int i = 0; i < top; ++i) {
				for(int j = i; j < top; ++j) {
					if(is_NT(stack[j]) || is_operator(stack[j])) {
						expr += stack[j];
					}
					else 
						expr += "i";
				}
				
				// 找到可规约串
				if(grammer_left.count(expr) > 0) {
					left_t = grammer_left[expr];
					f = true;
					break;
				}
				expr.clear();
			}
			// 找到可规约串
			if(f) {
				top -= expr.length();
				stack[top++] = left_t;
				left_t += "->" + expr;
				cout << left << setw(20) << left_t;
			}
			else {
				error();
				return -1;
			}
		}
		// 对应的优先关系 (a, b) = =
		else if(grade[id(right_t)][id(input[p])] == 0) {
			cout << left << setw(18) << "移进规约";
			stack[top++] = input[p++];
			
			// 寻找可规约串
			string expr, left_t;
			bool f = false;
			for(int i = 0; i < top; ++i) {
				for(int j = i; j < top; ++j) {
					if(is_NT(stack[j]) || is_operator(stack[j])) {
						expr += stack[j];
					}
					else 
						expr += "i";
				}
				// 找到可规约串
				if(grammer_left.count(expr) > 0) {
					left_t = grammer_left[expr];
					f = true;
					break;
				}
				expr.clear();
			}
			// 找到可规约串
			if(f) {
				top -= expr.length();
				stack[top++] = left_t;
				left_t += "->" + expr;
				cout << left << setw(20) << left_t;
			}
			else {
				error();
				return -1;
			}
		}
		// 对应的优先关系 (a, b) = ?
		else {
			error();
			return -1;
		}

		cout << cr;
	}

	str.clear();
	for(int i = 0; i < top; ++i)
		str += stack[i];
	cout << left << setw(18) << str << cr;
	if(top == 1 && stack[0] == "E") {
		cout << "规约成功" << cr;
		return 0;
	}
	else {
		error();
		return -1;
	}
}

void error() {
	cout << cr << "规约失败" << cr;
}

int main() {
	grammer_init();

    string str;
    while(getline(ifs, str)) {
		token tk = token(str);
		if(tk.type == token_type::operate && tk.value == "=") {
			int res = parser();
		}
    }

    return 0;
}