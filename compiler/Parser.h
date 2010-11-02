#ifndef _PARSER_H_
#define _PARSER_H_

#include "Scanner.h"
using namespace std;

class SyntaxNode{};
class NodeExpr: public SyntaxNode{
public:
	NodeExpr(Symbol* symb, int p, int l): symbol(symb), pos(p), line(l){type = NULL;};
	virtual int FillTree(int i, int j){return FillTreeIdentConst(i, j, GetValue());}
	string GetValue();
	virtual bool LValue(){return true;}
	virtual bool IsFunction() {return false; }
	virtual bool IsInt() {return false; }
	virtual bool IsReal() {return false; }
	virtual bool IsConst() {return false; }
	virtual bool IsBinaryOp() {return false; }
	void SetType(Symbol* t) {type = t; }
	Symbol* GetType() {return type;}
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
	Symbol* type;
	int pos;
	int line;
};
int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right);
class Statement: public SyntaxNode{
public:
	virtual int FillTree(int i, int j){return 0;}
	void Print(ostream& os, int n) {
		ClearArr();
		int h = FillTree(0, 0) - 1;
		PrintExpr(os, h);
	}
};

class StmtAssign: public Statement{
public:
	StmtAssign(NodeExpr* l, NodeExpr* r): left(l), right(r){};
	int FillTree(int i, int j){ return FillTreeBinOp(i, j, ":=", left, right);}
private:
	NodeExpr* left;
	NodeExpr* right;
};

class StmtBlock: public Statement{
public:
	StmtBlock(list<Statement*> stmts): statements(stmts){};
	int FillTree(int i, int j){
		int j1 = j;
		j1 = FillTreeOp(i, j, "begin");
		for (list<Statement*>::iterator it = statements.begin(); it != statements.end(); ++it)
			j1 = PaintBranch(i, j1, j1, j1 + (*it)->FillTree(i + 4, j1), true);
		j1 += FillTreeIdentConst(i, j1, "end ");
		return j1 - j;
	}
	void PrintAssignment(ostream& os){
		(*(statements.begin()))->Print(os, 0);
	}
private:
	list<Statement*> statements;
};

class StmtIf: public Statement{
public:
	StmtIf(NodeExpr* exp, Statement* frst, Statement* scnd): expr(exp), first(frst), second(scnd){};
	StmtIf(NodeExpr* exp, Statement* frst): expr(exp), first(frst), second(NULL){};
	int FillTree(int i, int j){ 
		int j1 = j;
		j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "if"), true);
		j = PaintBranch(i, j, j, j + expr->FillTree(i + 4, j), false);
		j = FillTreeOp(i, j, "then");
		if (second){
			j = PaintBranch(i, j, j, j + first->FillTree(i + 4, j), false);
			j = FillTreeOp(i, j, "else");
			j += second->FillTree(i + 4, j);
		}
		else
			j +=  first->FillTree(i + 4, j);
		return j - j1;
	}
private:
	NodeExpr* expr;
	Statement* first;
	Statement* second;
};

class StmtWhile: public Statement{
public:
	StmtWhile(NodeExpr* exp, Statement* b): expr(exp), body(b){};
	int FillTree(int i, int j){
		int j1 = j;
		j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "while"), true);
		j = PaintBranch(i, j, j, j + expr->FillTree(i + 4, j), false);
		j = FillTreeOp(i, j, "do");
		j += body->FillTree(i + 4, j);
		return j - j1;
	}
private:
	NodeExpr* expr;
	Statement* body;
};

class StmtRepeat: public Statement{
public:
	StmtRepeat(NodeExpr* exp, Statement* b): expr(exp), body(b){};
	int FillTree(int i, int j){
		int j1 = j;
		j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "repeat"), true);
		j = PaintBranch(i, j, j, j + body->FillTree(i + 4, j), false);
		j = FillTreeOp(i, j, "until");
		j += expr->FillTree(i + 4, j);
		return j - j1;
	}
private:
	NodeExpr* expr;
	Statement* body;
};

class Variable: public NodeExpr{
public:
	Variable(Symbol* symb, int p, int l): NodeExpr(symb, p, l) {};
	bool LValue() {return true; }
	bool IsInt();
	bool IsReal();
	int FillTree(int i, int j){ return FillTreeIdentConst(i, j, GetValue());}
};

class StmtFor: public Statement{
public:
	StmtFor(Symbol* v, NodeExpr* init, NodeExpr* finit, Statement* b, bool t): var(v), initVal(init), finitVal(finit), body(b), to(t){};
	int FillTree(int i, int j){
		int j1 = j;
		j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "for"), true);
		Variable* v = new Variable(var, 0, 0);
		StmtAssign* assign = new StmtAssign(v, initVal);
		j = PaintBranch(i, j, j, j + assign->FillTree(i + 4, j), false);
		j = to ? FillTreeOp(i, j, "to") : FillTreeOp(i, j, "downto");
		j = PaintBranch(i, j, j, j + finitVal->FillTree(i + 4, j), false);
		j = FillTreeOp(i, j, "do");
		j += body->FillTree(i + 4, j);
		delete v;
		delete assign;
		return j - j1;
	}
private:
	Symbol* var;
	NodeExpr* initVal;
	NodeExpr* finitVal;
	Statement* body;
	bool to;
};

class StmtBreak: public Statement{
	int FillTree(int i, int j){
		return FillTreeIdentConst(i, j, "break");
	}
};

class StmtContinue: public Statement{
	int FillTree(int i, int j){
		return FillTreeIdentConst(i, j, "continue");
	}
};

class Const: public NodeExpr{
public:
	Const(Symbol* symb, int p, int l): NodeExpr(symb, p, l){};
	bool LValue() {return false; }
	bool IsInt();
	bool IsReal();
	bool IsConst() {return true; }
	int FillTree(int i, int j) { return FillTreeIdentConst(i, j, GetValue());}
};

int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right);

class BinaryOp: public NodeExpr{
public: 
	BinaryOp(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r) : NodeExpr(symb, p, ll), left(l), right(r) {};
	bool LValue() {return false; }
	bool IsInt();
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
	bool IsInt();
	bool IsReal();
	int FillTree(int i, int j){return FillTreeBinOp(i, j, GetValue(), left, right);}
};

class RecordAccess: public NodeExpr{
public: 
	RecordAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): NodeExpr(symb, p, ll), left(l), right(r){};
	bool LValue() {return left->LValue(); }
	bool IsInt();
	bool IsReal();
	int FillTree(int i, int j) {return FillTreeBinOp(i, j, ".", left, right);}
private:
	NodeExpr* left;
	NodeExpr* right;
};

class FunctionCall: public NodeExpr{
public: 
	FunctionCall(Symbol* symb, int p, int l, list<NodeExpr*> ar): NodeExpr(symb, p, l), args(ar){}
	bool LValue() { return false; }
	bool IsInt();
	bool IsReal();
	bool IsFunction() { return true; }
	list<NodeExpr*>* GetArgs() { return &args; }
	int FillTree(int i, int j);
private:
	list<NodeExpr*> args;
};

class StmtProcedure: public Statement{
public:
	StmtProcedure(Symbol* pr, list<NodeExpr*>* ar): proc(pr), args(ar){};
	int FillTree(int i, int j){
		int j1 = j;
		FunctionCall* func = new FunctionCall(proc, 0, 0, *args);
		j += func->FillTree(i, j);
		delete func;
		return j - j1;
	}
private:
	Symbol* proc;
	list<NodeExpr*>* args;
};

class UnaryOp: public NodeExpr{
public: 
	UnaryOp(Symbol* symb, int p, int l, NodeExpr* ch): NodeExpr(symb, p, l), child(ch){}
	int FillTree(int i, int j);
	bool LValue() {return false; }
	bool IsInt();
	bool IsReal();
private:
	NodeExpr* child;
};

string UpCase(string s);

class Parser{
public:
	Parser(Scanner& sc, ostream& o);
	void ParseMainDecl() {ParseDecl(tableStack->begin(), true); }
	void ParseMainBlock(bool assignment) { 
		if (TokType() != ttEOF){
			StmtBlock* st = ParseBlock(tableStack->begin(), true);
			os << "\n";
			if (assignment)
				st->PrintAssignment(os);
			else			
				st->Print(os, 0);
		}
	}
private:
	void ParseDecl(SymStIt curTable, bool main);
	NodeExpr* ParseSimple(SymStIt curTable, int prior);
	NodeExpr* ParseFactor(SymStIt curTable); 
	NodeExpr* ParseFunctionCall(NodeExpr* res, int pos, int line);
	NodeExpr* ParseArrayAccess(NodeExpr* res,  int pos, int line);
	NodeExpr* ParseRecordAccess(NodeExpr* res,  int pos, int line);
	NodeExpr* ParseNext(SymStIt curTable);
	Const* ParseConst(SymStIt curTable);
	NodeExpr* ParseVariable(SymStIt curTable, NodeExpr* res);
	ArrayAccess* ParseArr(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line);
	RecordAccess* ParseRecord(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line);
	FunctionCall* ParseFunc(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line);
	Statement* ParseStatement(SymStIt curTable);
	void ParseTypeBlock();
	Symbol* ParseType(SymStIt curTable, bool newType);
	Symbol* ParseVar(SymStIt curTable);
	void ParseConstBlock(SymStIt curTable);
	void ParseTypeConstBlock(SymStIt curTable, TokenType tt);
	SymTable* ParseVarRecordBlock(SymStIt curTable, TokenType tt);
	Symbol* ParseConst(SymStIt curTable, bool newConst);
	Symbol* ParseProcedure(SymStIt curTable, bool newProc, bool func);
	Symbol* ParseArray(SymStIt curTable);
	SymTable* GetArgs(SymStIt curTable, string name, list<string>** arg_names);
	void CheckEof();
	TokenType TokType() {return scan.GetToken()->GetType(); }
	string TokVal() {return UpCase(scan.GetToken()->GetValue()); }
	int TokPos() {return scan.GetToken()->GetPos();}
	int TokLine() {return scan.GetToken()->GetLine();}
	string CheckCurTok(SymStIt curTable, string blockName, SymTable* tbl);
	void CheckProcDecl();
	StmtWhile* ParseWhile(SymStIt curTable);
	StmtAssign* ParseAssignment(SymStIt curTable);
	StmtProcedure* ParseProcedure(SymStIt curTable);
	StmtBlock* ParseBlock(SymStIt curTable, bool main);
	StmtIf* ParseIf(SymStIt curTable);
	StmtRepeat* ParseRepeat(SymStIt curTable);
	StmtFor* ParseFor(SymStIt curTable);
	SymTable::iterator FindS(string s, SymStIt curTable, SymStIt end);
	SymTable::iterator FindS(string s, SymTable* curTable);
	Scanner& scan;
	ostream& os;
	SymTableStack* tableStack;
	SymStIt lowerTable;
	string curIdent;
	bool isRecord;
	bool isAccess;
	bool nextScan;
	bool idFound;
};

#endif //_PARSER_H_