#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include "Scanner.h"
using namespace std;

int FindOpPrior(string str);

class Expr
{
public:
	Expr(){ Value = ""; }
	void Print(ostream& os, int n);
	virtual int FillTree(int i, int j) {return 0;}
protected:
	string Value;
};

class Ident: public Expr
{
public:
	Ident(string val) : Expr()
	{ 
		Value = val;
	}
	int FillTree(int i, int j);
};

class Const: public Expr
{
public:
	Const(string val) : Expr()	
	{ 
		Value = val;
	}
	int FillTree(int i, int j);
};

class BinaryOp: public Expr
{
public: 
	BinaryOp(string val, Expr* l, Expr* r): Expr(), left(l), right(r) 
	{ 
		Value = val;
	}
	int FillTree(int i, int j);
private:
	Expr* right;
	Expr* left;
};

class UnaryOp: public Expr
{
public: 
	UnaryOp(string val, Expr* ch): Expr(), child(ch) 
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
	Parser(Scanner& sc): scan(sc), error(){ FillMaps(); scan.Next(); }
	Expr* ParseSimpleExpr(); 
	bool HasError() {return error.GetText() != ""; }
	void PrintError(ofstream& of) {of << error.GetErrorPos() << " " 
				<< error.GetErrorLine()<< " "<< error.GetText() ; }
private:
	Expr* ParseSimple(int prior);
	Expr* ParseFactor(); 
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return scan.GetToken()->GetValue(); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	Scanner& scan;
	Error error;
	void FillMaps();	
};

#endif //_PARSER_H_