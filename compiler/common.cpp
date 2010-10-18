#include "common.h"

void FillMaps(){
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	priority[":="] = 5;
}

int FindOpPrior(string str){
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}

void ClearArr(){
	memset(arr, ' ', arrSize * arrSize);
}

void PrintExpr(ostream& os, int h){
	for (int i = 0; i < h; ++i){
		for (int j = 0; j < maxLength; ++j)
			os << arr[i][j];
		os << "\n";
	}
}

int PaintBranch(int i, int j, int k, int h, bool f){
	for (; k < h; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	if (f)
		for (unsigned int k = 0; k < 3; ++k)
			arr[j][i + k + 1] = '_';
	return j;
}

int FillTreeIdentConst(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int FillTreeOp(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	j = PaintBranch(i, j, 0, 2, true);
	return j;
}

bool IsAlpha(char ch) {	return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z';}
bool IsDigit(char ch) {	return ch >= '0' && ch <= '9';}
bool IsWhiteSpace(char ch) {	return ch >= 0 && ch <= 32;}
bool IsConcreteType(char ch, TokenType type);

bool IsHexDigit(char ch){ return IsDigit(ch) || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f';}
bool IsBinDigit(char ch){ return ch == '1' || ch == '0';}
bool IsOctDigit(char ch){ return ch >= '0' && ch <= '7';}

bool IsUnaryOp(string str){
	transform(str.begin(), str.end(), str.begin(), toupper);
	return str == "-" || str == "+" || str == "NOT";
}

bool IsLiteral(TokenType tt){
	return tt == ttIntLit || tt == ttRealLit || tt == ttStringLit || tt == ttHexLiteral || tt == ttBinLiteral 
		|| tt == ttOctLiteral;
}
bool IsConstType(TokenType type){
	return type == ttRealLit ||IsIntType(type);
}

bool IsIntType(TokenType type){
	return type == ttIntLit || type == ttHexLiteral || type == ttBinLiteral || type == ttOctLiteral;
}

string UpCase(string s){
	transform(s.begin(), s.end(), s.begin(), toupper);
	return s;
}