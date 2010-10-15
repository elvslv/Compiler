#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include <list>
#include "Scanner.h"
using namespace std;

class Symbol{
public:
	Symbol(){name = "";}
	Symbol(string s): name(s) {};
	virtual bool IsType() {return false;}
	virtual bool IsConst() {return false; }
	virtual string GetName() {return name; }
	virtual void SetName(string s) {name = s;}
	virtual void Print(ostream& os){};
protected:
	string name;
};

typedef pair<string, Symbol*> Sym;
typedef map <string, Symbol*> SymTable;

class SymType: public Symbol{
public:
	SymType(string s): Symbol(s) {};
	virtual bool IsType() {return true;}
	virtual bool IsConst() {return false; }
	virtual void Print(ostream& os) { os << name << " - type"; }
public:
	SymType(){};
};

class SymVar: public Symbol{
public:
	SymVar(string s, SymType* t): Symbol(s), type(t){};
	virtual bool IsConst() {return false; }
	virtual SymType* GetType() {return type; }
	virtual void Print(ostream& os) { os << name << " - variable, type: " << type->GetName(); }
private:
	SymType* type;
};

class SymProc: public Symbol{
public:
	SymProc(string s, SymTable* ar, SymTable* loc): Symbol(s), args(ar), locals(loc) {};
	virtual bool IsConst() {return false; }
	virtual void Print(ostream& os) { 
		os << name << " - procedure, args: ";  
		for (SymTable::iterator it = args->begin(); it != args->end(); ++it){
			(*it->second).Print(os);
			os << ", ";
		}
	}
protected:
	SymTable* args;
	SymTable* locals; 
};

class SymFunc: public SymProc{
public:
	SymFunc(string s, SymTable* ar, SymTable* loc, SymType* t): SymProc(s, ar, loc), type(t) {};
	void Print(ostream& os) { 
		os << name << " - function, args: ";  
		for (SymTable::iterator it = args->begin(); it != args->end(); ++it){
			(*it->second).Print(os);
			os << ", ";
		}
		type->Print(os);
	}
private:
	SymType* type;
};

class SymTypeScalar: public SymType{
public:
	SymTypeScalar(string s): SymType(s){};
};

class SymTypeFloat: public SymTypeScalar{
public:
	SymTypeFloat(string s): SymTypeScalar(s){};
};

class SymTypeInteger: public SymTypeScalar{
public:
	SymTypeInteger(string s): SymTypeScalar(s){};
};

class SymTypeRecord: public SymType{
public:
	SymTypeRecord(string s, SymTable* f): SymType(s), fields(f) {};
	void Print(ostream& os) { 
		os << name << " - record, fields: ";  
		for (SymTable::iterator it = fields->begin(); it != fields->end(); ++it){
			(*it->second).Print(os);
			os << ", ";
		}
	}
private:
	SymTable* fields;
};

class SymTypeAlias: public SymType{
public:
	SymTypeAlias(string s, SymType* rt): SymType(s), refType(rt){};
	void Print(ostream& os) { 
		os << name << " - type alias, ref type: ";  
		refType->Print(os);
	}
private:
	SymType* refType;
};

class SymVarLocal: public SymVar{
public:
	SymVarLocal(string s, SymType* t): SymVar(s, t){};
};

class SymVarGlobal: public SymVar{
public:
	SymVarGlobal(string s, SymType* t): SymVar(s, t){};
};

class SymVarConst: public SymVar{
public:
	SymVarConst(string s, SymType* t): SymVar(s, t){};
	bool IsConst() {return true; }
};

class SymVarParam: public SymVar{
public:
	SymVarParam(string s, SymType* t): SymVar(s, t){};
};

class SymVarParamByValue: public SymVarParam{
public:
	SymVarParamByValue(string s, SymType* t): SymVarParam(s, t){};
};

class SymVarParamByRef: public SymVarParam{
public:
	SymVarParamByRef(string s, SymType* t): SymVarParam(s, t){};
};

class SymTypeArray: public SymType{
public:
	SymTypeArray(string s, SymType* type, SymVarConst* b, SymVarConst* t): SymType(s), elemType(type), bottom(b), top(t) {};
	void Print(ostream& os) { 
		os << name << " - array, type of elements: ";  
		elemType->Print(os);
		os << ", bottom: ";
		bottom->Print(os);
		os << ", top: ";
		top->Print(os);
	}
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
private:
	SymVar* symbol;
};

class BinaryOp: public Expr{
public: 
	BinaryOp(string val, Expr* l, Expr* r) : left(l), right(r) { Value = val; }
	int FillTree(int i, int j);
protected:
	Expr* right;
	Expr* left;
};

class ArrayAccess: public BinaryOp{
public: 
	ArrayAccess(string val, Expr* l, Expr* r): BinaryOp(val, l, r){};
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
};

class RecordAccess: public Expr{
public: 
	RecordAccess(string val, Expr* l, SymVar* r): left(l), right(r) {	Value = val; }
	int FillTree(int i, int j);
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
		table = new SymTable();
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
		SymTypeInteger* i = new SymTypeInteger("INTEGER");
		SymTypeFloat* f = new SymTypeFloat("FLOAT");
		table->insert(Sym("INTEGER", i));
		table->insert(Sym("FLOAT", f));
		curIdent = "";
	}
	Expr* ParseSimpleExpr() { return ParseSimple(4); } 
	void ParseDecl();
	void PrintTree(ostream& os){
		for( SymTable::iterator it = table->begin(); it != table->end(); ++it){
			if(it->first == "FLOAT" || it->first == "INTEGER")
				continue;
			(*it->second).Print(os);
			os << "\n";
		}
	}
private:
	Expr* ParseSimple(int prior);
	Expr* ParseFactor(); 
	Expr* ParseFunctionCall(Expr* res, int pos, int line);
	Expr* ParseArrayAccess(Expr* res,  int pos, int line);
	Expr* ParseRecordAccess(Expr* res,  int pos, int line);
	Expr* ParseNext();
	Expr* CallAccess(Expr* id, int pos, int line);
	void ParseAssignment();
	void ParseTypeBlock();
	SymType* ParseType(bool newType);
	void ParseVarBlock();
	SymVar* ParseVar();
	void ParseConstBlock();
	SymVarConst* ParseConst(bool newConst);
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
	string curIdent;
	void FillMaps();
	bool isRecord;
	bool isAccess;
};

#endif //_PARSER_H_