#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
using namespace std;


const char sp = 32, cr = 10;

// token 类型名称
string token_type_name[] = {
	"keyword",
	"operator",
	"delimiter",
	"identifier",
	"label",
	"literal",
	"number"
};

enum token_type {
	keyword,
	operate,
	delimiter,
	identifier,
	label,
	literal,
	number,
};

class token {
	token_type type;
	string value;

public:
	token() {};
	token(token_type _type, string& _value) {
		type = _type;
		value = _value;
	}

	friend ostream& operator << (ostream& os, token& t) {
		os << "[" << token_type_name[t.type] << ", " << t.value << "]";
		return os;
	}
};

// 词法分析器类
class texer {
	typedef size_t size_type;

private:
	set<string> keywords;
	set<char> operators;
	set<char> delimiters;

	vector<string> buffer;
	size_type row, col, n;

private:
	int skip();							// 跳过空白和注释
	size_type next_word(); 				// 返回下一个单词的右端，开区间
	bool is_delimiter(size_type);   	// 判断给定位置处字符是否是界符
	bool is_operator(size_type);		// 判断给定位置处字符是否是操作符

	bool is_delimiter(const string&);	// 判断给定字符串是否是界符
	bool is_operator(const string&);	// 判断给字符串是否是操作符
	bool is_number(const string&);		// 判断给定字符串是否是数字
	bool is_keyword(const string&);		// 判断给定字符串是否是关键字
	bool is_label(const string&); 		// 判断给定字符串是否是标签
	bool is_literal(const string&);		// 判断给定字符串是否是字符或字符串
	bool is_identifier(const string&);	// 判断给定字符串是否是标识符

	void error() {
		cout << "Invalid identifier at line " << row << ", col " << col << cr;
	}
public:
	texer() { row = 0, col = 0, n = 0; }

	void init(ifstream&);
	int preprocess();
	int get_tokens();
};
// 判断给定字符串是否是界符
bool texer::is_delimiter(const string& str) {
	return str.length() == 1 && delimiters.find(str[0]) != delimiters.end();
}
// 判断给字符串是否是操作符
bool texer::is_operator(const string& str) {
	return str.length() == 1 && operators.find(str[0]) != operators.end();
}
// 判断给定字符串是否是数字
bool texer::is_number(const string& str) {
	for(auto ch : str) {
		if(!isdigit(ch)) return false;
	}
	return true;
}
// 判断给定字符串是否是关键字
bool texer::is_keyword(const string& str) {
	return keywords.find(str) != keywords.end();
}
// 判断给定字符串是否是标签
bool texer::is_label(const string& str) {
	auto len = str.length();
	if(str[len - 1] == ':' && is_identifier(str.substr(0, len - 1)))
		return true;
	return false;
}
// 判断给定字符串是否是字符或字符串
bool texer::is_literal(const string& str) {
	if(str[0] == '"' && str[str.length() - 1] == '"') return true;
	if(str.length() == 3 && str[0] == '\'' && str[2] == '\'') return true;
	return false;
}
// 判断给定字符串是否是标识符
bool texer::is_identifier(const string& str) {
	if(str[0] == '_' || isalpha(str[0])) {
		for(auto ch : str) {
			if(ch != '_' && !isalpha(ch) && !isdigit(ch))
				return false;
		}
		return true;
	}
	else
		return false;
}

// 跳过空格注释等
int texer::skip() {
	bool f = true;
	while ( f && row < n ) {
		while(row < n && col >= buffer[row].length() )
			col = 0, ++row;
		if ( row >= n ) break;
		f = false;

		// 跳过空格
		if ( buffer[row][col] == sp ) {
			f = true;
			while ( col < buffer[row].length() && buffer[row][col] == sp )
				++col;
		}
		// 跳过块注释
		if ( buffer[row][col] == '/' && buffer[row][col + 1] == '*' ) {
			f = true;
			col += 2;
			while ( row < n && !(buffer[row][col] == '*' && buffer[row][col + 1] == '/') ) {
				++col;
				if ( col >= buffer[row].length() )
					col = 0, ++row;
			}
			col += 2;
		}
		// 跳过宏定义
		if ( buffer[row][col] == '#' ) {
			f = true;
			++row, col = 0;
		}
		// 跳过行注释
		if ( buffer[row][col] == '/' && buffer[row][col + 1] == '/' ) {
			f = true;
			++row, col = 0;
		}
		if ( col >= buffer[row].length() )
			col = 0, ++row;
	}
	if ( row < n ) return 0;
	return -1;
}

// 寻找下一个独立的单词
texer::size_type texer::next_word() {
	texer::size_type c = col;
	// 界符
	if ( is_delimiter(col) ) {
		return col + 1;
	}
	// 操作符
	else if ( is_operator(col) ) {
		return col + 1;
	}
	// 其他情况，遇到界符或空格停止
	else if ( buffer[row][col] == '"' || buffer[row][col] == '\'' ) {
		++c;
		if ( buffer[row][col] == '"' ) {
			while ( c < buffer[row].length() && buffer[row][c] != '"' ) ++c;
		}
		else {
			while ( c < buffer[row].length() && buffer[row][c] != '\'' ) ++c;
		}
		return c + 1;
	}
	else {
		while ( c < buffer[row].length() && buffer[row][c] != sp && !(is_delimiter(c) && buffer[row][c] != ':') && !is_operator(c) ) ++c;
		return c;
	}
}

bool texer::is_delimiter(texer::size_type index) {
	return delimiters.find(buffer[row][index]) != delimiters.end();
}

bool texer::is_operator(texer::size_type index) {
	return operators.find(buffer[row][index]) != operators.end();
}

void texer::init(ifstream& src) {
	string str;

	// 将源文件加载到缓冲区
	while ( getline(src, str) ) {
		++n;
		buffer.emplace_back(str);
	}
	// 加载关键字表
	keywords.insert("main");
	keywords.insert("int");
	keywords.insert("if");
	keywords.insert("else");
	keywords.insert("goto");
	keywords.insert("return");
	// 加载操作符表
	operators.insert('+');
	operators.insert('=');
	operators.insert('<');
	// 加载界符表
	delimiters.insert('(');
	delimiters.insert(')');
	delimiters.insert('{');
	delimiters.insert('}');
	delimiters.insert(',');
	delimiters.insert(';');
	delimiters.insert(':');
}

// 预处理
int texer::preprocess() {


	return 0;
}

// 对源文件进行词法分析
int texer::get_tokens() {
	string::size_type l, r;
	token tk;

	while ( row < n ) {
		if ( skip() < 0 ) break;
		r = next_word();
		l = col;

		string str = buffer[row].substr(l, r - l);
		if(is_delimiter(str))		// 判断是界符
			tk = token(token_type::delimiter, str);
		else if(is_operator(str))	// 判断是操作符
			tk = token(token_type::operate, str);
		else if(is_keyword(str))	// 判断是关键字
			tk = token(token_type::keyword, str);
		else if(is_number(str))		// 判断是数字
			tk = token(token_type::number, str);
		else if(is_literal(str))	// 判断是字符或字符串
			tk = token(token_type::literal, str);
		else if(is_identifier(str)) // 判断是标识符
			tk = token(token_type::identifier, str);
		else if(is_label(str)) {	// 判断是标签
			string t = str.substr(0, str.length() - 1);
			tk = token(token_type::label, t);
		}
		else {	// 错误类型，进行错误处理，词法分析结束
			error();
			break;
		}
		cout << tk << cr;	// 输出识别出的 token
		
		col = r;
	}

	return 0;
}

int main(int argc, char* argv [ ]) {
	string src;
	if ( argc > 1 ) src = argv[1];
	else src = "source.c";

	ifstream file = ifstream(src, ios::in);

	texer tx;
	tx.init(file);
	tx.preprocess();
	tx.get_tokens();


	file.close();
}