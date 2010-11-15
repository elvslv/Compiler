#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_

#include <string>
#include <map>
#include <list>
#include <stack>
#include "ExprParser.h"
using namespace std;

class Statement;
class SymType;
class AsmProc;

class Symbol{
public:
	Symbol(){name = "";}
	Symbol(string s): name(s) {};
	virtual bool IsType() {return false;}
	virtual bool IsConst() {return false; }
	virtual bool IsVar() {return false;}
	virtual bool IsProc() {return false;}
	virtual bool IsFunc() {return false;}
	virtual string GetName() {return name; }
	virtual void SetName(string s) {name = s;}
	virtual void Print(ostream& os, bool f){};
	virtual SymType* GetType() {return NULL; }
	virtual void Generate(AsmProc* Asm) {};
protected:
	string name;
};

class SymType: public Symbol{
public:
	SymType(string s): Symbol(s) {};
	virtual bool IsType() {return true;}
	virtual bool IsArray() {return false;}
	virtual bool IsRecord() {return false;}
	virtual void Print(ostream& os, bool f) { os << name; }
	virtual bool IsInt() {return false;}
	virtual bool IsReal() {return false;}
	virtual bool IsScalar() {return false;}
	virtual bool IsAlias() {return false; }
	virtual SymType* GetSourceType() {return this; }
};

class SymVar: public Symbol{
public:
	SymVar(string s, SymType* t): Symbol(s), type(t){};
	virtual bool IsVar() {return true;}
	virtual bool IsParamByRef() {return false; }
	virtual SymType* GetType() {return type; }
	virtual void Print(ostream& os, bool f);
	virtual void Generate(AsmProc* Asm);
protected:
	SymType* type;
};

class SymProc: public Symbol{
public:
	SymProc(string s, SymTable* ar, SymTable* loc, list<string>* names, Statement* b): Symbol(s), args(ar), locals(loc), argNames(names), body(b) {};
	virtual bool IsProc() {return true;}
	void Print(ostream& os, bool f);
	virtual void PrintBody(ostream& os);
	SymTable* GetArgsTable() {return args;}
	list<string>* GetArgNames() {return argNames;}
	void SetBody(Statement* b) { body = b; }
protected:
	SymTable* args;
	SymTable* locals; 
	list<string>* argNames;
	Statement* body;
};

class SymFunc: public SymProc{
public:
	bool IsProc() {return false;}
	bool IsFunc() {return true;}
	SymFunc(string s, SymTable* ar, SymTable* loc, list<string>* names, Statement* b, SymType* t): SymProc(s, ar, loc, names, b), type(t) {};
	void Print(ostream& os, bool f);
	SymTable* GetArgsTable() {return args;}
	list<string>* GetArgNames() {return argNames;}
	SymType* GetType() {return type; }
private:
	SymType* type;
};

class SymTypeScalar: public SymType{
public:
	SymTypeScalar(string s): SymType(s){};
	virtual bool IsScalar() {return true;}
};

class SymTypeReal: public SymTypeScalar{
public:
	SymTypeReal(string s): SymTypeScalar(s){};
	bool IsReal() {return true;}
};

class SymTypeInteger: public SymTypeScalar{
public:
	SymTypeInteger(string s): SymTypeScalar(s){};
	bool IsInt() {return true; }
};

class SymTypeString: public SymTypeScalar{
public:
	SymTypeString(string s): SymTypeScalar(s){};
	bool IsString() {return true; }
};

class SymTypeRecord: public SymType{
public:
	SymTypeRecord(string s, SymTable* f): SymType(s), fields(f) {};
	bool IsRecord() {return true;}
	void Print(ostream& os, bool f);
	SymTable* GetFileds() {return fields; }
private:
	SymTable* fields;
};

class SymTypeAlias: public SymType{
public:
	SymTypeAlias(string s, SymType* rt): SymType(s), refType(rt){};
	void Print(ostream& os, bool f);
	bool IsInt() {return refType->IsInt(); }
	bool IsReal() {return refType->IsReal();}
	bool IsScalar() {return refType->IsScalar();}
	bool IsAlias() {return true; }
	SymType* GetRefType() {return refType; }
	SymType* GetSourceType() {
		SymType* t1 = this;
		while(t1->IsAlias())
			t1 = ((SymTypeAlias*)t1)->GetRefType();
		return t1;
	}
private:
	SymType* refType;
};

class SymTypePointer: public SymType{
public:
	SymTypePointer(string s, SymType* rt): SymType(s), refType(rt){};
	void Print(ostream& os, bool f);
	bool IsInt() {return false; }
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
	virtual void Print(ostream& os, bool f);
	virtual void PrintVal(ostream& os) = 0;
	bool IsConst() {return true; }
	virtual bool IsInt() { return false; }
	virtual bool IsReal() { return false; }
	virtual bool IsString() {return false; }
	virtual void Generate(AsmProc* Asm); 
	virtual string ToString() = 0;
};

class SymVarConstInt: public SymVarConst{
public:
	SymVarConstInt(string s, SymType* t, int v): SymVarConst(s, t), val(v){};
	int GetValue() {return val; }
	void PrintVal(ostream& os) {os << val; }
	bool IsInt() {return true; }
	string ToString () { return toString(val); }
private:
	int val;
};

class SymVarConstReal: public SymVarConst{
public:
	SymVarConstReal(string s, SymType* t, double v): SymVarConst(s, t), val(v){};
	double GetValue() {return val; }
	void PrintVal(ostream& os) {
		os.precision(10);
		os << val; 
	}
	bool IsReal() {return true; }
	string ToString () { return toString(val); }
private:
	double val;
};

class SymVarConstString: public SymVarConst{
public:
	SymVarConstString(string s, SymType* t, string v): SymVarConst(s, t), val(v){};
	string GetValue() {return val; }
	void PrintVal(ostream& os) { os << val; }
	bool IsString() { return true; }
	string ToString () { return val; }
private:
	string val;
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
	void Print(ostream& os, bool f);
	virtual bool IsParamByRef() {return true; }
};

class SymTypeArray: public SymType{
public:
	SymTypeArray(string s, SymType* type, SymVarConst* b, SymVarConst* t): SymType(s), elemType(type), bottom(b), top(t) {};
	bool IsArray() {return true;}
	void Print(ostream& os, bool f);
	SymType* GetElemType() {return elemType; }
	SymVarConst* GetBottom() {return bottom; }
	SymVarConst* GetTop() {return top; }
private:
	SymType* elemType;
	SymVarConst* bottom; SymVarConst* top;
};

#endif //_SYMBOLS_H_