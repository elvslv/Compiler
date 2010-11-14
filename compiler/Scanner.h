#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include "common.h"
using namespace std;
void FillTokenTypes();
class Token
{
public:
	Token() {Value = ""; }
	Token(int pos, int line, string val): curPos(pos), curLine(line), Value(val){};
	virtual TokenType GetType(){return ttToken;}
	virtual string GetTypeName(){return "Token";}
	string GetValue(){return Value;}
	int GetPos() {return curPos + 1; }
	int GetLine() {return curLine + 1; }
	TokenType type() {return GetType(); }
protected:
	int curLine, curPos;
	string Value;
};

class Identifier: public Token
{
public:
	Identifier(){};
	Identifier(int pos, int line, string val) : Token(pos, line, val){};
	virtual TokenType GetType() {return ttIdentifier; }
	virtual string GetTypeName(){return "Identifier";}
};

class KeyWord: public Identifier
{
public:
	KeyWord(int pos, int line, string val);
	TokenType GetType() {return type; }
	string GetTypeName(){return "Key Word";}
private:
	TokenType type;

};

class Literal:  public Token 
{
public:
	Literal(int pos, int line, string val): Token(pos, line, val){};
	virtual TokenType GetType() {return ttLiteral; }
	virtual string GetTypeName(){return "Literal";}
};

class IntLiteral: public Literal
{
public:
	IntLiteral(int pos, int line, string val) : Literal(pos, line, val)
	{
		num = atoi(val.c_str());
	}
	virtual TokenType GetType() {return ttIntLit; }
	virtual string GetTypeName(){return "Integer Literal";}
	virtual int GetVal() {return num; }
protected:
	int num;
};

class HexLiteral: public IntLiteral
{
public:
	HexLiteral(int pos, int line, string val) : IntLiteral(pos, line, val)
	{
		num = 0;
		for (unsigned int i = 1; i < val.length(); ++i)
		{
			num *= 16;
			num += val[i] >= '0' && val[i] <= '9' ? val[i] - '0' : val[i] - 'A' + 10;
		}
	}
	TokenType GetType() {return ttHexLiteral; }
	string GetTypeName(){return "Hexadecimal Literal";}
};

class BinLiteral: public IntLiteral
{
public:
	BinLiteral(int pos, int line, string val) : IntLiteral(pos, line, val)
	{
		num = 0;
		for (unsigned int i = 1; i < val.length(); ++i)
		{
			num *= 2;
			num += val[i] - '0';
		}
	}
	TokenType GetType() {return ttBinLiteral; }
	string GetTypeName(){return "Binary Literal";}
};

class OctLiteral: public IntLiteral
{
public:
	OctLiteral(int pos, int line, string val) : IntLiteral(pos, line, val)
	{
		num = 0;
		for (unsigned int i = 1; i < val.length(); ++i)
		{
			num *= 8;
			num += val[i] - '0';
		}
	}
	TokenType GetType() {return ttOctLiteral; }
	string GetTypeName(){return "Octal Literal";}
};


class RealLiteral: public Literal
{
public:
	RealLiteral(int pos, int line, string val) : Literal(pos, line, val)
	{
		num = atof(val.c_str());
	}
	TokenType GetType() {return ttRealLit; }
	string GetTypeName(){return "Real Literal";}
	double GetVal() {return num; }
private:
	double num;
};

class StringLiteral: public Literal
{
public:
	StringLiteral(int pos, int line, string val) : Literal(pos, line, val){};
	TokenType GetType() {return ttStringLit; }
	string GetTypeName(){return "String Literal";}
	string GetVal() {return Value; }
};

class Operation: public Token
{
public:
	Operation(int pos, int line, string val);
	TokenType GetType() {return type; }
	string GetTypeName(){return "Operation";}
private:
	TokenType type;
};

class Separator: public Token
{
public:
	Separator(int pos, int line, string val);
	TokenType GetType() {return type; }
	string GetTypeName(){return "Separator";}
private:
	TokenType type;
};

class EndOfFile: public Token
{
public:
	EndOfFile(int pos, int line){ curPos = pos; curLine = line; Value = "EOF"; }
	TokenType GetType() {return ttEOF; }
	string GetTypeName(){return "End Of File";}
};

class BadToken: public Token
{
public:
	BadToken(int pos, int line, string val) : Token(pos, line, val){};
	TokenType GetType() {return ttBadToken; }
	string GetTypeName(){return "Incorrect Token";}
};

class Scanner
{
public:
	Scanner(istream& is): file(is)
	{
		curLine = 0;
		curPos = 0;
		pos = 0;
		curToken = new Token();
		FillTokenTypes();
		state = stNewLex;
		curStr = "";
	}
	Token* GetToken() { return curToken; }
	void Next();
private:
	istream& file;
	int curLine, curPos, pos, line;
	char curCh, prevch;
	Token* curToken;
	State state;
	string curStr;
	void SkipDelimeters();
	void SkipComments();
	void ChangePosIndex();
	void GC(); //get next char if it's possible
	void CurChPutBack(char ch);
	void NewToken(TokenType tt, int p, int l, string val, char ch);
	int StateStNewLex();
	int StateStDigConsq();
	int StateStIdent();
	int StateStRealLitWithoutE();
	int StateStStrLitWithHashDigs();
	int StateStCloseApostrophe();
	int StateStRealLitPointLast();
	int StateStDollarPercentAmpersand();
	string Change(string str);
	bool operator == (TokenType tt) { return curToken->GetType() == tt; }
};

#endif //_SCANNER_H_