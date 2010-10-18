#ifndef _EXPR_PARSER_H_
#define _EXPR_PARSER_H_

#include <string>
#include <map>
#include <list>
#include "Scanner.h"
using namespace std;
int FillTreeBinOp(int i, int j, string Value, Expr* left, Expr* right);

class Expr{
public:
	Expr(){ Value = ""; }
	Expr(string str): Value(str){};
	virtual int FillTree(int i, int j){return 0;}
	string GetValue() { return Value; }
	virtual bool LValue() { return false; }
	virtual bool IsFunction() {return false; }
	void Print(ostream& os, int n) {
		int h = FillTree(0, 0) - 1;
		PrintExpr(os, h);
	}
protected:
	string Value;
};

class SimpleIdent: public Expr{
public:
	SimpleIdent(string val){ Value = val; }
	bool LValue() { return true; }
	int FillTree(int i, int j){	return FillTreeIdentConst(i, j, Value);	}
};

class SimpleConst: public Expr{
public:
	SimpleConst(string val){ Value = val; }
	int FillTree(int i, int j){	return FillTreeIdentConst(i, j, Value); }
};

class SimpleBinaryOp: public Expr{
public: 
	SimpleBinaryOp(string val, Expr* l, Expr* r) : left(l), right(r) { Value = val; }
	int FillTree(int i, int j){	return FillTreeBinOp(i, j, Value, left, right); }
protected:
	Expr* right;
	Expr* left;
};

class SimpleArrayAccess: public SimpleBinaryOp{
public: 
	SimpleArrayAccess(string val, Expr* l, Expr* r): SimpleBinaryOp(val, l, r){};
	bool LValue() { return true; }
	int FillTree(int i, int j){	return FillTreeBinOp(i, j, Value, left, right); }
};

class SimpleRecordAccess: public Expr{
public: 
	SimpleRecordAccess(string val, Expr* l, Expr* r): left(l), right(r) {Value = val;}
	bool LValue() { return true; }
	int FillTree(int i, int j){	return FillTreeBinOp(i, j, Value, left, right); }
private:
	Expr* right;
	Expr* left;
};

class SimpleFunctionCall: public Expr{
public: 
	SimpleFunctionCall(Expr* val, list<Expr*> ar){
		Value = val;
		args = ar;
	}
	int FillTree(int i, int j);
	bool IsFunction() {return true; }
private:
	list<Expr*> args;
	Expr* Value;
};


class SimpleUnaryOp: public Expr{
public: 
	SimpleUnaryOp(string val, Expr* ch): child(ch){ 
		Value = val;
	}
	int FillTree(int i, int j);
private:
	Expr* child;
};

class ExprParser{
public:
	ExprParser(Scanner& sc): scan(sc){
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
	}
	Expr* ParseSimpleExpr() { return ExprParse(4); } 
private:
	Expr* ExprParse(int prior);
	Expr* ExprParseFactor(); 
	Expr* ExprParseFunctionCall(Expr* res, int pos, int line);
	Expr* ExprParseArrayAccess(Expr* res,  int pos, int line);
	Expr* ExprParseRecordAccess(Expr* res,  int pos, int line);
	Expr* ExprParseNext();
	Expr* ExprCallAccess(Expr* id, int pos, int line);
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return UpCase(scan.GetToken()->GetValue()); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	Scanner& scan;
	bool isRecord;
	bool isAccess;
};

#endif //_EXPR_PARSER_H_