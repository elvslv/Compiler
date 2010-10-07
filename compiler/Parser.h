#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include <list>
#include "Scanner.h"
using namespace std;

int FindOpPrior(string str);

class Expr
{
public:
	Expr(){ Value = ""; }
	void Print(ostream& os, int n);
	virtual int FillTree(int i, int j) = 0;
	string GetValue() { return Value; }
	virtual bool IsIdent() { return false; }
protected:
	string Value;
};

class Ident: public Expr
{
public:
	Ident(string val){ Value = val; }
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
};

class Const: public Expr
{
public:
	Const(string val){ Value = val; }
	int FillTree(int i, int j);
};

class BinaryOp: public Expr
{
public: 
	BinaryOp(string val, Expr* l, Expr* r) : left(l), right(r) { Value = val; }
	int FillTree(int i, int j);
protected:
	Expr* right;
	Expr* left;
};

class ArrayAccess: public BinaryOp
{
public: 
	ArrayAccess(string val, Expr* l, Expr* r): BinaryOp(val, l, r){};
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
};

class RecordAccess: public Expr
{
public: 
	RecordAccess(string val, Expr* l, Expr* r): left(l), right(r) {	Value = val; }
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
private:
	Expr* right;
	Expr* left;
};

class FunctionCall: public Expr
{
public: 
	FunctionCall(Expr* val, list<Expr*> ar)
	{
		Value = val;
		args = ar;
	}
	int FillTree(int i, int j);
	bool IsIdent() {return true; }
private:
	list<Expr*> args;
	Expr* Value;
};


class UnaryOp: public Expr
{
public: 
	UnaryOp(string val, Expr* ch): child(ch) 
	{ 
		Value = val;
	}
	int FillTree(int i, int j);
private:
	Expr* child;
};

class Parser
{
public:
	Parser(Scanner& sc): scan(sc){ FillMaps(); scan.Next(); isRecord = false; isAccess = false;}
	Expr* ParseSimpleExpr() { return ParseSimple(4); } 
private:
	Expr* ParseSimple(int prior);
	Expr* ParseFactor(); 
	Expr* ParseFunctionCall(Expr* res, int pos, int line);
	Expr* ParseArrayAccess(Expr* res,  int pos, int line);
	Expr* ParseRecordAccess(Expr* res,  int pos, int line);
	Expr* ParseNext();
	Expr* CallAccess(Expr* id, int pos, int line);
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return scan.GetToken()->GetValue(); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	Scanner& scan;
	void FillMaps();
	bool isRecord;
	bool isAccess;
};

#endif //_PARSER_H_