#ifndef _PARSER_H_
#define _PARSER_H_

#include "Symbols.h"
using namespace std;

class NodeExpr;

class SyntaxNode{};

class Statement: public SyntaxNode{};

class StmtAssign: public Statement{
public:
	StmtAssign(NodeExpr* l, NodeExpr* r): left(l), right(r){};
private:
	NodeExpr* left;
	NodeExpr* right;
};

class StmtProcedure: public Statement{
public:
	StmtProcedure(SymProc* pr): proc(pr){};
private:
	SymProc* proc;
};

class StmtBlock: public Statement{
public:
	StmtBlock(list<Statement*> stmts): statements(stmts){};
private:
	list<Statement*> statements;
};

class StmtIf: public Statement{
public:
	StmtIf(NodeExpr* exp, Statement* frst, Statement* scnd): expr(exp), first(frst), second(scnd){};
	StmtIf(NodeExpr* exp, Statement* frst): expr(exp), first(frst), second(NULL){};
private:
	NodeExpr* expr;
	Statement* first;
	Statement* second;
};

class StmtWhile: public Statement{
public:
	StmtWhile(NodeExpr* exp, Statement* b): expr(exp), body(b){};
private:
	NodeExpr* expr;
	Statement* body;
};

class StmtRepeat: public Statement{
public:
	StmtRepeat(NodeExpr* exp, Statement* b): expr(exp), body(b){};
private:
	NodeExpr* expr;
	Statement* body;
};

class StmtFor: public Statement{
public:
	StmtFor(SymVar* v, NodeExpr* init, NodeExpr* finit, bool t): var(v), initVal(init), finitVal(finit), to(t){};
private:
	SymVar* var;
	NodeExpr* initVal;
	NodeExpr* finitVal;
	bool to;
};

class StmtBreak: public Statement{};

class StmtContinue: public Statement{};

class NodeExpr: public SyntaxNode{
public:
	NodeExpr(Symbol* symb, int p, int l): symbol(symb), pos(p), line(l){type = NULL;};
	virtual int FillTree(int i, int j){return FillTreeIdentConst(i, j, GetValue());}
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
	void Print(ostream& os, int n) {
		ClearArr();
		int h = FillTree(0, 0) - 1;
		PrintExpr(os, h);
	}
protected:
	Symbol* symbol;
	SymType* type;
	int pos;
	int line;
};

class Variable: public NodeExpr{
public:
	Variable(SymVar* symb, int p, int l): NodeExpr(symb, p, l) {};
	bool LValue() {return true; }
	bool IsInt() {return symbol->GetType()->IsInt(); }
	int FillTree(int i, int j){ return FillTreeIdentConst(i, j, GetValue());}
};

class Const: public NodeExpr{
public:
	Const(SymVarConst* symb, int p, int l): NodeExpr(symb, p, l){};
	bool LValue() {return false; }
	bool IsInt() {return symbol->GetType()->IsInt(); }
	bool IsConst() {return true; }
	int FillTree(int i, int j) { return FillTreeIdentConst(i, j, GetValue());}
};

int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right);

class BinaryOp: public NodeExpr{
public: 
	BinaryOp(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r) : NodeExpr(symb, p, ll), left(l), right(r) {};
	bool LValue() {return false; }
	bool IsInt() {return right->IsInt() && left->IsInt() && IsIntOperator(GetValue()); }
	bool IsBinaryOp() {return true; }
	NodeExpr* GetLeft() {return left;}
	NodeExpr* GetRight() {return right;}
	int FillTree(int i, int j){ return FillTreeBinOp(i, j, GetValue(), left, right);}
protected:
	NodeExpr* right;
	NodeExpr* left;
};

class ArrayAccess: public BinaryOp{
public: 
	ArrayAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): BinaryOp(symb, p, ll, l, r){};
	bool IsIdent() { return true; }
	bool LValue() {return left->LValue(); }
	bool IsInt() { return GetType()->IsInt(); }
	int FillTree(int i, int j){return FillTreeBinOp(i, j, GetValue(), left, right);}
};

class RecordAccess: public NodeExpr{
public: 
	RecordAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): NodeExpr(symb, p, ll), left(l), right(r){};
	bool LValue() {return left->LValue(); }
	bool IsInt() { return GetType()->IsInt(); }
	int FillTree(int i, int j) {return FillTreeBinOp(i, j, ".", left, right);}
private:
	NodeExpr* left;
	NodeExpr* right;
};

class FunctionCall: public NodeExpr{
public: 
	FunctionCall(Symbol* symb, int p, int l, list<NodeExpr*> ar): NodeExpr(symb, p, l), args(ar){}
	bool LValue() {return false; }
	bool IsInt() {return GetType()->IsInt(); }
	int FillTree(int i, int j);
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

string UpCase(string s);

class Parser{
public:
	Parser(Scanner& sc, ostream& o);
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
	Statement* ParseStatement();
	void ParseTypeBlock();
	SymType* ParseType(bool newType);
	SymVar* ParseVar();
	void ParseConstBlock();
	void ParseTypeConstBlock(TokenType tt);
	SymTable* ParseVarRecordBlock(TokenType tt);
	SymVarConst* ParseConst(bool newConst);
	SymProc* ParseProcedure(bool newProc, bool func);
	SymTypeArray* ParseArray();
	SymTable* GetArgs(string name, list<string>** arg_names);
	void CheckEof();
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return UpCase(scan.GetToken()->GetValue()); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	string CheckCurTok(string blockName, SymTable* tbl);
	void CheckProcDecl();
	StmtWhile* ParseWhile();
	StmtAssign* ParseAssignment();
	StmtProcedure* ParseProcedure();
	StmtBlock* ParseBlock(bool main);
	StmtIf* ParseIf();
	StmtRepeat* ParseRepeat();
	StmtFor* ParseFor();
	Scanner& scan;
	ostream& os;
	SymTable* table() {
		return st->top();
	}
	SymTableStack* st;
	string curIdent;
	bool isRecord;
	bool isAccess;
	bool nextScan;
};

#endif //_PARSER_H_