#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include <list>
#include "Scanner.h"
using namespace std;

class Symbol{
public:
	virtual bool IsType() {return false;}
private:
	string name;
};

bool IsConst(TokenType type){
	return type == ttIntLit || type == ttRealLit;
}

typedef map <string, Symbol*> SymTable;

class SymType: public Symbol{
public:
	virtual bool IsType() {return true;}
public:
	SymType(){};
};

class SymVar: public Symbol{
public:
	SymVar(SymType* t): type(t){};
private:
	SymType* type;
};

class SymProc: public Symbol{
public:
	SymProc(SymTable* ar, SymTable* loc): args(ar), locals(loc) {};
private:
	SymTable* args;
	SymTable* locals; 
};

class SymFunc: public SymProc{
public:
	SymFunc(SymTable* ar, SymTable* loc, SymType* t): SymProc(ar, loc), type(t) {};
private:
	SymTable* args;
	SymTable* locals; 
	SymType* type;
};

class SymTypeScalar: public SymType{
};

class SymTypeFloat: public SymTypeScalar{
};

class SymTypeInteger: public SymTypeScalar{
};

class SymTypeRecord: public SymType{
public:
	SymTypeRecord(SymTable* f): fields(f) {};
private:
	SymTable* fields;
};

class SymTypeAlias: public SymType{
public:
	SymTypeAlias(SymType* rt): refType(rt){};
private:
	SymType* refType;
};

class SymVarLocal: public SymVar{
};

class SymVarGlobal: public SymVar{
};

class SymVarConst: public SymVar{
public:
	SymVarConst(SymType* t): SymVar(t){};
};

class SymVarParam: public SymVar{
public:
	SymVarParam(SymType* t): SymVar(t){};
};

class SymVarParamByValue: public SymVarParam{
public:
	SymVarParamByValue(SymType* t): SymVarParam(t){};
};

class SymVarParamByRef: public SymVarParam{
public:
	SymVarParamByRef(SymType* t): SymVarParam(t){};
};

class SymTypeArray: public SymType{
public:
	SymTypeArray(SymType* type, SymVarConst* b, SymVarConst* t): elemType(type), bottom(b), top(t) {};
private:
	SymType* elemType;
	SymVarConst* bottom; SymVarConst* top;
};

class SyntaxNode{
};

class NodeStatement: public SyntaxNode{
};

class StatementAssign: public NodeStatement{
};

int FindOpPrior(string str);

class Expr{
public:
	Expr(){ Value = ""; }
	void Print(ostream& os, int n);
	virtual int FillTree(int i, int j) = 0;
	string GetValue() { return Value; }
	virtual bool LValue() { return false; }
	virtual bool IsFunction() {return false; }
	virtual TokenType ExprType() {return ttToken;}
protected:
	string Value;
};

class Ident: public Expr{
public:
	Ident(SymVar* symb): symbol(symb) {};
	int FillTree(int i, int j);
private:
	SymVar* symbol;
};

class Const: public Expr{
public:
	Const(SymVar* symb): symbol(symb){};
	int FillTree(int i, int j);
	TokenType ExprType() {return type;}
private:
	TokenType type;
	SymVar* symbol;
};

class BinaryOp: public Expr{
public: 
	BinaryOp(string val, Expr* l, Expr* r) : left(l), right(r) { Value = val; }
	int FillTree(int i, int j);
	TokenType ExprType() {
		TokenType lt = left->ExprType();
		TokenType rt = right->ExprType();
		return lt == rt && IsConst(lt) ? lt : ttOperation;
	}
protected:
	Expr* right;
	Expr* left;
};

class ArrayAccess: public BinaryOp{
public: 
	ArrayAccess(string val, Expr* l, Expr* r): BinaryOp(val, l, r){};
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
	TokenType ExprType() {return ttOperation;}
};

class RecordAccess: public Expr{
public: 
	RecordAccess(string val, Expr* l, SymVar* r): left(l), right(r) {	Value = val; }
	int FillTree(int i, int j);
	TokenType ExprType() {return ttOperation;}
private:
	SymVar* right;
	Expr* left;
};

class FunctionCall: public Expr{
public: 
	FunctionCall(SymFunc* val, list<Expr*> ar){
		Value = val;
		args = ar;
	}
	int FillTree(int i, int j);
	TokenType ExprType() {return ttOperation;}
private:
	list<Expr*> args;
	SymFunc* Value;
};


class UnaryOp: public Expr{
public: 
	UnaryOp(string val, Expr* ch): child(ch){ 
		Value = val;
	}
	int FillTree(int i, int j);
	TokenType ExprType() {return child->ExprType();}
private:
	Expr* child;
};

class SimpleIdent: public Expr{
public:
	SimpleIdent(string val){ Value = val; }
	int FillTree(int i, int j);
	bool LValue() { return true; }
};

class SimpleConst: public Expr{
public:
	SimpleConst(string val){ Value = val; }
	int FillTree(int i, int j);
};

class SimpleBinaryOp: public Expr{
public: 
	SimpleBinaryOp(string val, Expr* l, Expr* r) : left(l), right(r) { Value = val; }
	int FillTree(int i, int j);
protected:
	Expr* right;
	Expr* left;
};

class SimpleArrayAccess: public SimpleBinaryOp{
public: 
	SimpleArrayAccess(string val, Expr* l, Expr* r): SimpleBinaryOp(val, l, r){};
	int FillTree(int i, int j);
	bool LValue() { return true; }
};

class SimpleRecordAccess: public Expr{
public: 
	SimpleRecordAccess(string val, Expr* l, Expr* r): left(l), right(r) {Value = val;}
	int FillTree(int i, int j);
	bool LValue() { return true; }
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

class Parser{
public:
	Parser(Scanner& sc): scan(sc){
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
		SymTypeInteger* i = new SymTypeInteger();
		SymTypeFloat* f = new SymTypeFloat();
		(*table)["INTEGER"] = i;
		(*table)["FLOAT"] = f;
	}
	Expr* ParseSimpleExpr() { return ParseSimple(4); } 
	SymTable* GetSymTable() {return table;}
private:
	Expr* ParseSimple(int prior);
	Expr* ParseFactor(); 
	Expr* ParseFunctionCall(Expr* res, int pos, int line);
	Expr* ParseArrayAccess(Expr* res,  int pos, int line);
	Expr* ParseRecordAccess(Expr* res,  int pos, int line);
	Expr* ParseNext();
	Expr* CallAccess(Expr* id, int pos, int line);
	void ParseAssignment();
	void ParseDecl();
	void ParseTypeBlock();
	SymType* ParseType(bool newType);
	void ParseVarBlock();
	SymVar* ParseVar();
	void ParseConstBlock();
	SymVarConst* ParseConst();
	SymFunc* ParseFunction(bool newFunc);
	SymProc* ParseProcedure(bool newProc);
	SymTable* ParseRecordBlock();
	SymTypeArray* ParseArray();
	SymTable* GetArgs();
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return scan.GetToken()->GetValue(); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	string CheckCurTok(string blockName, SymTable* tbl);
	Scanner& scan;
	SymTable* table;
	void FillMaps();
	bool isRecord;
	bool isAccess;
};

#endif //_PARSER_H_