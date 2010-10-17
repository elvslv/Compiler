#include "common.h"

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
	return type == ttIntLit || type == ttRealLit;
}
string UpCase(string s){
	transform(s.begin(), s.end(), s.begin(), toupper);
	return s;
}