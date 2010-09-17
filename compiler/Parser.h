#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include "Scanner.h"
using namespace std;

enum ExprType
{
	etExpr,
	etBinaryOp, 
	etUnaryOp,
	etIdent,
	etConst,
	etMinus,
	etPlus,
	etMul, 
	etDiv,
	etIntDiv,
	etMod,
	etNot, 
	etAnd, 
	etOr, 
	etXor,
	etShl, 
	etShr, 
	etEqual,
	etNotEq,
	etLess,
	etLessOrEq,
	etMore,
	etMoreOrEq,
};

ExprType FindExprType(string val);
int FindOpPrior(ExprType t);

class Expr
{
public:
	Expr(){ type = etExpr; }
	ExprType GetExprType() {return type; }
	void Print(ostream& os, int n);
	virtual int FillTree(int i, int j) {return 0;}
protected:
	ExprType type;
	string Value;
};

class Ident: public Expr
{
public:
	Ident(string val) : Expr()
	{ 
		type = etIdent; 
		Value = val;
	}
	int FillTree(int i, int j);
};

class Const: public Expr
{
public:
	Const(string val) : Expr()	
	{ 
		type = etIdent; 
		Value = val;
	}
	int FillTree(int i, int j);
};

class BinaryOp: public Expr
{
public: 
	BinaryOp(string val, Expr* l, Expr* r): Expr(), left(l), right(r) 
	{ 
		type = FindExprType(val); 
		prior = FindOpPrior(type);
		Value = val;
	}
	int FillTree(int i, int j);
private:
	Expr* right;
	Expr* left;
	int prior;
};

class UnaryOp: public Expr
{
public: 
	UnaryOp(string val, Expr* ch): Expr(), child(ch) 
	{ 
		type = FindExprType(val); 
		prior = FindOpPrior(type);
		if (type == etMinus || type == etPlus)
			prior = 1;
		Value = val;
	}
	int FillTree(int i, int j);
private:
	Expr* child;
	int prior;
};

class Parser
{
public:
	Parser(Scanner& sc): scan(sc), error(){ FillMaps(); scan.Next(); }
	Expr* ParseSimpleExpr(); 
	Expr* ParseSimple(int prior);
	Expr* ParseFactor(); 
	bool HasError() {return error.GetText() != ""; }
	void PrintError(ofstream& of) {of << error.GetErrorPos() << " " 
				<< error.GetErrorLine()<< " "<< error.GetText() ; }
private:
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return scan.GetToken()->GetValue(); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	Scanner& scan;
	Error error;
	void FillMaps();	
};

#endif //_PARSER_H_