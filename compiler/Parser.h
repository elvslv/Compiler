#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <map>
#include <list>
#include "ExprParser.h"
using namespace std;
string UpCase(string s);

class SymType;

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
protected:
	string name;
};

typedef pair<string, Symbol*> Sym;
typedef map <string, Symbol*> SymTable;

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
};

class SymVar: public Symbol{
public:
	SymVar(string s, SymType* t): Symbol(s), type(t){};
	virtual bool IsVar() {return true;}
	virtual bool IsParamByRef() {return false; }
	virtual SymType* GetType() {return type; }
	virtual void Print(ostream& os, bool f) { 
		os << "var "<< name << " : "; 
		type->Print(os, f); 
		os << ";";
	}
protected:
	SymType* type;
};

class SymProc: public Symbol{
public:
	SymProc(string s, SymTable* ar, SymTable* loc, list<string>* names): Symbol(s), args(ar), locals(loc), argNames(names) {};
	virtual bool IsProc() {return true;}
	virtual void Print(ostream& os, bool f) { 
		os << "procedure "<< name;
		if (!args->empty()){	
			os << "(";  
			int i = 0;
			for (SymTable::iterator it = args->begin(); it != args->end(); ++it)
				(*it->second).Print(os, false);
			os << ")";
		}
		os << ";";
	}
	SymTable* GetArgsTable() {return args;}
	list<string>* GetArgNames() {return argNames;}
protected:
	SymTable* args;
	SymTable* locals; 
	list<string>* argNames;
};

class SymFunc: public SymProc{
public:
	bool IsProc() {return false;}
	bool IsFunc() {return true;}
	SymFunc(string s, SymTable* ar, SymTable* loc, list<string>* names, SymType* t): SymProc(s, ar, loc, names), type(t) {};
	void Print(ostream& os, bool f) { 
		os << "function " << name;
		if (!args->empty()){	
			os << "(";  
			int i = 0;
			for (SymTable::iterator it = args->begin(); it != args->end(); ++it)
				(*it->second).Print(os, false);
			os << ")";
		}
		os << ": ";
		type->Print(os, false);
		os << ";";
	}
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

class SymTypeRecord: public SymType{
public:
	SymTypeRecord(string s, SymTable* f): SymType(s), fields(f) {};
	bool IsRecord() {return true;}
	void Print(ostream& os, bool f) {
		if (!f)
			os << name;
		else{
			if (name != "" && (name[0] < '0' || name[0] > '9'))
				os << "type " << name << " = ";
			os << "record \n";
			for (SymTable::iterator it = fields->begin(); it != fields->end(); ++it){
				os << "    ";
				if ((*it->second).IsVar() && (*it->second).GetType()->GetName()[0] >= '0' && (*it->second).GetType()->GetName()[0] <= '9')
					(*it->second).Print(os, true);
				else
					(*it->second).Print(os, false);
				os << "\n";
			}
			os << "end";
		}
	}
	SymTable* GetFileds() {return fields; }
private:
	SymTable* fields;
};

class SymTypeAlias: public SymType{
public:
	SymTypeAlias(string s, SymType* rt): SymType(s), refType(rt){};
	void Print(ostream& os, bool f) { 
		if (!f)
			os << name;
		else{
			if (name != "" && !(name[0] >= '0' && name[0] <= '9'))
				os << "type "<< name << " = ";
			refType->Print(os, false);
		}
	}
	bool IsInt() {return refType->IsInt(); }
	bool IsReal() {return refType->IsReal();}
	bool IsScalar() {return refType->IsScalar();}
	bool IsAlias() {return true; }
	SymType* GetRefType() {return refType; }
private:
	SymType* refType;
};

class SymTypePointer: public SymType{
public:
	SymTypePointer(string s, SymType* rt): SymType(s), refType(rt){};
	void Print(ostream& os, bool f) { 
		if (!f)
			os << name;
		else{
			if (name != "" && !(name[0] >= '0' && name[0] <= '9') && f)
				os << "type "<< name << " = ";
			os << "^";  
			refType->Print(os, false);
		}
	}
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
	SymVarConst(string s, string v, SymType* t): SymVar(s, t), val(v){};
	virtual void Print(ostream& os, bool f) { 
		if (!f && !(name == "" || (name[0] >= '0' && name[0] <= '9')))
			os << "const " << name;
		else{
			if (name == "" || (name[0] >= '0' && name[0] <= '9'))
				os << "const " << val;
			else
				os << "const " << name << " = " << val;
		}
	}
	bool IsConst() {return true; }
	string GetValue() {return val; }
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
	void Print(ostream& os, bool f) { 
		os << "var (by ref) "<< name << " : "; 
		type->Print(os, false); 
		os << ";";
	}
	virtual bool IsParamByRef() {return true; }
};

class SymTypeArray: public SymType{
public:
	SymTypeArray(string s, SymType* type, SymVarConst* b, SymVarConst* t): SymType(s), elemType(type), bottom(b), top(t) {};
	bool IsArray() {return true;}
	void Print(ostream& os, bool f) {
		if (!f)
			os << name;
		else{
			if (name != "" && (name[0] < '0' || name[0] > '9'))
				os << "type " << name << " = ";
			os << "array [";
			bottom->Print(os, false);
			os << "..";
			top->Print(os, false);
			os << "] of ";
			elemType->Print(os, elemType->GetName()[0] >= '0' && elemType->GetName()[0] <= '9');
		}
	}
	SymType* GetElemType() {return elemType; }
	SymVarConst* GetBottom() {return bottom; }
	SymVarConst* GetTop() {return top; }
private:
	SymType* elemType;
	SymVarConst* bottom; SymVarConst* top;
};

class SyntaxNode{
};

class NodeStatement: public SyntaxNode{
};

int FindOpPrior(string str);
int FillTreeIdentConst_(int i, int j, string val);
bool IsIntOperator(string val);

class NodeExpr: public SyntaxNode{
public:
	NodeExpr(Symbol* symb, int p, int l): symbol(symb), pos(p), line(l){type = NULL;};
	void Print(ostream& os, int n);
	virtual int FillTree(int i, int j){return FillTreeIdentConst_(i, j, GetValue());}
	string GetValue() { return symbol->GetName(); }
	virtual bool LValue(){return true;}
	virtual bool IsFunction() {return false; }
	virtual bool IsInt() {return false; }
	virtual bool IsConst() {return false; }
	virtual bool IsBinaryOp() {return false; }
	void SetType(SymType* t) {type = t; }
	SymType* GetType() {return type;}
	Symbol* GetSymbol() {return symbol; }
	int GetPos() {return pos;}
	int GetLine() {return line;}
protected:
	Symbol* symbol;
	SymType* type;
	int pos;
	int line;
};

class Variable: public NodeExpr{
public:
	Variable(SymVar* symb, int p, int l): NodeExpr(symb, p, l) {};
	int FillTree(int i, int j);
	bool LValue() {return true; }
	bool IsInt() {return symbol->GetType()->IsInt(); }
};

class Const: public NodeExpr{
public:
	Const(SymVarConst* symb, int p, int l): NodeExpr(symb, p, l){};
	int FillTree(int i, int j);
	bool LValue() {return false; }
	bool IsInt() {return symbol->GetType()->IsInt(); }
	bool IsConst() {return true; }
};

class BinaryOp: public NodeExpr{
public: 
	BinaryOp(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r) : NodeExpr(symb, p, ll), left(l), right(r) {};
	int FillTree(int i, int j);
	bool LValue() {return false; }
	bool IsInt() {return right->IsInt() && left->IsInt() && IsIntOperator(GetValue()); }
	bool IsBinaryOp() {return true; }
	NodeExpr* GetLeft() {return left;}
	NodeExpr* GetRight() {return right;}
protected:
	NodeExpr* right;
	NodeExpr* left;
};

class ArrayAccess: public BinaryOp{
public: 
	ArrayAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): BinaryOp(symb, p, ll, l, r){};
	int FillTree(int i, int j);
	bool IsIdent() { return true; }
	bool LValue() {return left->LValue(); }
	bool IsInt() {return left->IsInt(); }
};

class RecordAccess: public NodeExpr{
public: 
	RecordAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): NodeExpr(symb, p, ll), left(l), right(r){};
	int FillTree(int i, int j);
	bool LValue() {return left->LValue(); }
	bool IsInt() {return right->IsInt(); }
private:
	NodeExpr* left;
	NodeExpr* right;
};

class FunctionCall: public NodeExpr{
public: 
	FunctionCall(Symbol* symb, int p, int l, list<NodeExpr*> ar): NodeExpr(symb, p, l), args(ar){}
	int FillTree(int i, int j);
	bool LValue() {return false; }
	bool IsInt() {return symbol->GetType()->IsInt(); }
private:
	list<NodeExpr*> args;
};

class UnaryOp: public NodeExpr{
public: 
	UnaryOp(Symbol* symb, int p, int l, NodeExpr* ch): NodeExpr(symb, p, l), child(ch){}
	int FillTree(int i, int j);
	bool LValue() {return false; }
	bool IsInt() {return child->IsInt() && IsIntOperator(GetValue()); }
private:
	NodeExpr* child;
};

class Parser{
public:
	Parser(Scanner& sc, ostream& o): scan(sc), os(o){
		table = new SymTable();
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
		SymTypeInteger* i = new SymTypeInteger("INTEGER");
		SymTypeReal* f = new SymTypeReal("REAL");
		table->insert(Sym("INTEGER", i));
		table->insert(Sym("REAL", f));
		curIdent = "";
		nextScan = true;
	}
	void ParseDecl();
private:
	NodeExpr* ParseSimple(int prior);
	NodeExpr* ParseFactor(); 
	NodeExpr* ParseFunctionCall(NodeExpr* res, int pos, int line);
	NodeExpr* ParseArrayAccess(NodeExpr* res,  int pos, int line);
	NodeExpr* ParseRecordAccess(NodeExpr* res,  int pos, int line);
	NodeExpr* ParseNext();
	Const* ParseConst();
	NodeExpr* ParseVariable(NodeExpr* res);
	ArrayAccess* ParseArr(NodeExpr* res, SymVar** var, int pos, int line);
	RecordAccess* ParseRecord(NodeExpr* res, SymVar** var, int pos, int line);
	FunctionCall* ParseFunc(NodeExpr* res, SymVar** var, int pos, int line);
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
	SymTable* GetArgs(string name, list<string>** arg_names);
	void CheckEof();
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return UpCase(scan.GetToken()->GetValue()); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	string CheckCurTok(string blockName, SymTable* tbl);
	int FindOpPrior(string str);
	Scanner& scan;
	ostream& os;
	SymTable* table;
	string curIdent;
	void FillMaps();
	bool isRecord;
	bool isAccess;
	bool nextScan;
};

#endif //_PARSER_H_