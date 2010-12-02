#ifndef _PARSER_H_
#define _PARSER_H_

#include "CodeGen.h"
#include "Scanner.h"
#include "common.h"

using namespace std;

AsmMem* mem(string name);
static AsmReg* eax = new AsmReg("EAX");
static AsmReg* ebx = new AsmReg("EBX");
static AsmReg* ecx = new AsmReg("ECX");
static AsmReg* edx = new AsmReg("EDX");
static AsmReg* esi = new AsmReg("ESI");
static AsmReg* edi = new AsmReg("EDI");
static AsmReg* ebp = new AsmReg("EBP");
static AsmReg* esp = new AsmReg("ESP");
static AsmMem* writeBuf = mem("writebuf");
static AsmOffset* writeBufOff = new AsmOffset(writeBuf);

bool UnnamedSymb(Symbol* symb);
AsmMem* mem(string name);
class Parser;
class SyntaxNode{
public:
	virtual int FillTree(int i, int j) = 0;
	virtual void Print(ostream& os, int n) {
		ClearArr();
		int h = FillTree(0, 0) - 1;
		PrintExpr(os, h);
	}
	virtual void Generate(AsmProc* Asm){return;}
};

class NodeExpr: public SyntaxNode{
public:
	NodeExpr(Symbol* symb, int p, int l): symbol(symb), pos(p), line(l){type = NULL;};
	virtual int FillTree(int i, int j){return FillTreeIdentConst(i, j, GetValue());}
	string GetValue();
	virtual bool LValue(){return true;}
	virtual bool IsFunction() {return false; }
	virtual bool IsInt() {return false; }
	virtual bool IsReal() {return false; }
	virtual bool IsString() {return false; }
	virtual bool IsConst() {return false; }
	virtual bool IsBinaryOp() {return false; }
	void SetType(Symbol* t) {type = t; }
	Symbol* GetType() {return type;}
	Symbol* GetSymbol() {return symbol; }
	int GetPos() {return pos;}
	int GetLine() {return line;}
	virtual void Generate(AsmProc* Asm){};
	virtual void GenLValue(AsmProc* Asm){};
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
};

class StmtAssign: public Statement{
public:
	StmtAssign(NodeExpr* l, NodeExpr* r): left(l), right(r){};
	int FillTree(int i, int j){ return FillTreeBinOp(i, j, ":=", left, right);}
	void Generate(AsmProc* Asm);
private:
	NodeExpr* left;
	NodeExpr* right;
};

class StmtBlock: public Statement{
public:
	StmtBlock(list<Statement*> stmts): statements(stmts){};
	int FillTree(int i, int j);
	void PrintAssignment(ostream& os){(*(statements.begin()))->Print(os, 0);}
	void Generate(AsmProc* Asm){
		for (list<Statement*>::iterator it = statements.begin(); it != statements.end(); ++it)
			(*it)->Generate(Asm);
	}
private:
	list<Statement*> statements;
};

class StmtIf: public Statement{
public:
	StmtIf(NodeExpr* exp, Statement* frst, Statement* scnd): expr(exp), first(frst), second(scnd){};
	StmtIf(NodeExpr* exp, Statement* frst): expr(exp), first(frst), second(NULL){};
	int FillTree(int i, int j);
private:
	NodeExpr* expr;
	Statement* first;
	Statement* second;
};
int FillTreeWhileRepeat(int i, int j, string s1, string s2, SyntaxNode* node1, SyntaxNode* node2);
class StmtWhile: public Statement{
public:
	StmtWhile(NodeExpr* exp, Statement* b): expr(exp), body(b){};
	int FillTree(int i, int j){ return FillTreeWhileRepeat(i, j, "while", "do", expr, body); }
private:
	NodeExpr* expr;
	Statement* body;
};

class StmtRepeat: public Statement{
public:
	StmtRepeat(NodeExpr* exp, Statement* b): expr(exp), body(b){};
	int FillTree(int i, int j){ return FillTreeWhileRepeat(i, j, "repeat", "until", body, expr); }
private:
	NodeExpr* expr;
	Statement* body;
};

class StmtWrite: public Statement{
public:
	StmtWrite(bool w, list<NodeExpr*>* p): writeln(w), params(p){};
	StmtWrite(bool w): writeln(w), params(NULL){};
	int FillTree(int i, int j);
	void Generate(AsmProc* Asm);
private:
	list<NodeExpr*>* params;
	bool writeln;
};

class Variable: public NodeExpr{
public:
	Variable(Symbol* symb, int p, int l): NodeExpr(symb, p, l) {};
	bool LValue() {return true; }
	bool IsInt();
	bool IsReal();
	int FillTree(int i, int j){ return FillTreeIdentConst(i, j, GetValue());}
	void Generate(AsmProc* Asm);
	void GenLValue(AsmProc* Asm);
};

class StmtFor: public Statement{
public:
	StmtFor(Symbol* v, NodeExpr* init, NodeExpr* finit, Statement* b, bool t): var(v), initVal(init), finitVal(finit), body(b), to(t){};
	int FillTree(int i, int j);
private:
	Symbol* var;
	NodeExpr* initVal;
	NodeExpr* finitVal;
	Statement* body;
	bool to;
};

class StmtBreak: public Statement{
	int FillTree(int i, int j){ return FillTreeIdentConst(i, j, "break"); }
};

class StmtContinue: public Statement{
	int FillTree(int i, int j){	return FillTreeIdentConst(i, j, "continue"); }
};

class Const: public NodeExpr{
public:
	Const(Symbol* symb, int p, int l): NodeExpr(symb, p, l){};
	bool LValue() {return false; }
	bool IsInt();
	bool IsReal();
	bool IsConst() {return true; }
	bool IsString();
	int FillTree(int i, int j) { return FillTreeIdentConst(i, j, GetValue());}
	void Generate(AsmProc* Asm);
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
	void Generate(AsmProc* Asm);
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
	void GenLValue(AsmProc* Asm);
	void Generate(AsmProc* Asm);
};

class RecordAccess: public NodeExpr{
public: 
	RecordAccess(Symbol* symb, int p, int ll, NodeExpr* l, NodeExpr* r): NodeExpr(symb, p, ll), left(l), right(r){};
	bool LValue() {return left->LValue(); }
	bool IsInt();
	bool IsReal();
	int FillTree(int i, int j) {return FillTreeBinOp(i, j, ".", left, right);}
	void GenLValue(AsmProc* Asm);
	void Generate(AsmProc* Asm);
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
	void Generate(AsmProc* Asm);
private:
	list<NodeExpr*> args;
};

class StmtProcedure: public Statement{
public:
	StmtProcedure(Symbol* pr, list<NodeExpr*>* ar): proc(pr), args(ar){};
	int FillTree(int i, int j);
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
	void Generate(AsmProc* Asm);
private:
	NodeExpr* child;
};

string UpCase(string s);

class Parser{
public:
	Parser(Scanner& sc, ostream& o);
	void ParseMainDecl() {ParseDecl(tableStack->begin(), true); }
	void ParseMainBlock();
	void Print(bool assignment){
		os << "\n";
		assignment ? program->PrintAssignment(os) : program->Print(os, 0);
	}
	void PrintTable();
	void Generate();
	void AsmCodePrint(){ Asm->Print(os); };
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
	StmtWrite* Parser::ParseWrite(SymStIt curTable, bool writeln);
	StmtWhile* ParseWhile(SymStIt curTable);
	StmtAssign* ParseAssignment(SymStIt curTable);
	StmtProcedure* ParseProcedure(SymStIt curTable);
	StmtBlock* ParseBlock(SymStIt curTable, bool main);
	StmtIf* ParseIf(SymStIt curTable);
	StmtRepeat* ParseRepeat(SymStIt curTable);
	StmtFor* ParseFor(SymStIt curTable);
	SymMap::iterator FindS(string s, SymStIt curTable, SymStIt end);
	SymMap::iterator FindS(string s, SymTable* curTable);
	void TryError(bool f, string mes);
	void TryError(bool f, string mes, int pos, int line);
	Scanner& scan;
	ostream& os;
	SymTableStack* tableStack;
	StmtBlock* program;
	SymStIt lowerTable;
	AsmCode* Asm;
	string curIdent;
	bool isRecord;
	bool isAccess;
	bool nextScan;
	bool idFound;
};

#endif //_PARSER_H_