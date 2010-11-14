#include "Parser.h"
#include "Symbols.h"

static int id = 0;
void Parser::PrintTable(){
	SymTable* tbl = *lowerTable;
	for (SymTable::iterator it = tbl->begin(); it != tbl->end(); ++it){
		if (it->second->GetName() == "INTEGER" || it->second->GetName() == "REAL" || it->second->GetName() == "STRING")
			continue;
		it->second->Print(os, true);
		if (it->second->IsConst())
			os << ";";
		os << "\n";
	}
}
void Parser::Generate(){
	AsmProc* curProc = Asm->AddProc("data", false, true);
	curProc->Add(asmData);
	SymTable* tbl = *lowerTable;
	for (SymTable::iterator it = tbl->begin(); it != tbl->end(); ++it){
		if (it->second->GetName() == "INTEGER" || it->second->GetName() == "REAL" || it->second->GetName() == "STRING")
			continue;
		if (!it->second->IsProc())
			it->second->Generate(curProc);
	}
	curProc = Asm->AddProc("code", false, true);
	curProc->Add(asmCode);
	for (SymTable::iterator it = tbl->begin(); it != tbl->end(); ++it){
		if (!it->second->IsProc() || it->second->GetName() == "INTEGER" || it->second->GetName() == "REAL" || it->second->GetName() == "STRING")
			continue;
		curProc = Asm->AddProc(it->second->GetName(), false, false);
		it->second->Generate(curProc);
	}
	curProc = Asm->AddProc("main", true, false);
	program->Generate(curProc);
}
void SymVar::Generate(AsmProc* Asm){
	Asm->Add(asmDw, new AsmMem(name));
}
void SymVarConst::Generate(AsmProc* Asm){
	Asm->Add(asmDw, new AsmMem(name), new AsmMem(ToString()));
}
void Variable::Generate(AsmProc* Asm){ Asm->Add(asmPush, new AsmMem(symbol->GetName())); }
void Const::Generate(AsmProc* Asm){
	if (UnnamedSymb(symbol)){
		if (IsInt())
			Asm->Add(asmPush, new AsmImmInt(((SymVarConstInt*)symbol)->GetValue()));
		else if (IsReal())
			Asm->Add(asmPush, new AsmImmReal(((SymVarConstReal*)symbol)->GetValue()));
		else
			Asm->Add(asmPush, new AsmImmString(((SymVarConstString*)symbol)->GetValue()));
	}
	else
		Asm->Add(asmPush, new AsmMem(symbol->GetName()));
}
void BinaryOp::Generate(AsmProc* Asm){
	string op = symbol->GetName();
	left->Generate(Asm);
	right->Generate(Asm);
	Asm->Add(asmPop, ebx);
	Asm->Add(asmPop, eax);
	if (op == "*")
		Asm->Add(asmIMul, eax, ebx);
	if (op == "-")
		Asm->Add(asmSub, eax, ebx);
	if (op == "+")
		Asm->Add(asmAdd, eax, ebx);
	if (op == "DIV")
		Asm->Add(asmIDiv, eax, ebx);
	if (op == "MOD"){  //a mod b
		Asm->Add(asmPush, eax); // a->стек
		Asm->Add(asmIDiv, ebx); //eax = a div b
		Asm->Add(asmPop, edx); // edx = a
		Asm->Add(asmMov, ecx, eax);  //ecx = a div b
		Asm->Add(asmMov, eax, edx); // eax = a
		Asm->Add(asmIMul, ecx); //eax = a * (a div b)
		Asm->Add(asmSub, edx, eax); // edx = a - a*(a div b)
		Asm->Add(asmMov, eax, edx);
	}
	Asm->Add(asmPush, eax);
}
void UnaryOp::Generate(AsmProc* Asm){
	string op = symbol->GetName();
	child->Generate(Asm);
	if (op == "-"){
		Asm->Add(asmPop, eax);
		Asm->Add(asmIMul, new AsmImmInt(-1));
		Asm->Add(asmPush, eax);
	}
}
bool UnnamedType(SymType* type){ return type && type->GetName()[0] >= '0' && type->GetName()[0] <= '9'; }
bool UnnamedSymb(Symbol* symb){ return symb->GetName()[0] >= '0' && symb->GetName()[0] <= '9'; }
int StmtBlock::FillTree(int i, int j){
	int j1 = j;
	j1 = FillTreeOp(i, j, "begin");
	for (list<Statement*>::iterator it = statements.begin(); it != statements.end(); ++it)
		j1 = PaintBranch(i, j1, j1, j1 + (*it)->FillTree(i + 4, j1), true);
	j1 += FillTreeIdentConst(i, j1, "end ");
	return j1 - j;
}
int StmtIf::FillTree(int i, int j){ 
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
int FillTreeWhileRepeat(int i, int j, string s1, string s2, SyntaxNode* node1, SyntaxNode* node2){
	int j1 = j;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, s1), true);
	j = PaintBranch(i, j, j, j + node1->FillTree(i + 4, j), false);
	j = FillTreeOp(i, j, s2);
	j += node2->FillTree(i + 4, j);
	return j - j1;
}
int StmtFor::FillTree(int i, int j){
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
int StmtProcedure::FillTree(int i, int j){
	int j1 = j;
	FunctionCall* func = new FunctionCall(proc, 0, 0, *args);
	j += func->FillTree(i, j);
	delete func;
	return j - j1;
}
void Parser::ParseMainBlock() { 
	if (TokType() == ttEOF)
		return;
	program = ParseBlock(tableStack->begin(), true);
}
void PrintArgs(ostream& os, SymTable* args){
	if (args->empty())
		return;
	os << "(";  
	for (SymTable::iterator it = args->begin(); it != args->end(); ++it)
		(*it->second).Print(os, false);
	os << ")";
}
void SymProc::PrintBody(ostream& os) { 
	os << "\n";
	if (!body)
		return;
	body->Print(os, 0);
}
void SymProc::Print(ostream& os, bool f) { 
	os << "procedure "<< name;
	PrintArgs(os, args);
	os << ";";
	PrintBody(os);
}
void SymFunc::Print(ostream& os, bool f) { 
	os << "function " << name;
	PrintArgs(os, args);
	os << ": ";
	type->Print(os, false);
	os << ";";
	PrintBody(os);
}
string NodeExpr::GetValue() { return symbol->GetName(); }
bool SymIsInt(Symbol* type) { return type && ((SymType*)type)->IsInt(); }
bool SymIsReal(Symbol* type) { return type && ((SymType*)type)->IsReal(); }
bool Variable::IsInt() { return symbol->GetType()->IsInt(); }
bool Variable::IsReal() { return symbol->GetType()->IsReal(); }
bool Const::IsInt() {return symbol->GetType()->IsInt(); }
bool Const::IsReal() {return symbol->GetType()->IsReal(); }
bool BinaryOp::IsInt() {return right->IsInt() && left->IsInt() && IsIntOperator(GetValue()) || 
						 ((SymType*)right->GetType())->IsScalar() && ((SymType*)right->GetType())->IsScalar() && IsLogicOperator(GetValue()); }
bool ArrayAccess::IsInt() { return SymIsInt(GetType()); }
bool ArrayAccess::IsReal() { return SymIsReal(GetType());  }
bool RecordAccess::IsInt() { return SymIsInt(GetType());  }
bool RecordAccess::IsReal() { return SymIsReal(GetType());  }
bool FunctionCall::IsInt() { return SymIsInt(GetType());  }
bool FunctionCall::IsReal() { return SymIsReal(GetType()); }
bool UnaryOp::IsInt() {return child->IsInt() && IsIntOperator(GetValue()); }
bool UnaryOp::IsReal() { return child->IsReal(); }
bool EqTypes(SymType* t1, SymType* t2){ return t1->GetSourceType() == t2->GetSourceType(); }
bool IToR(SymType* t1, SymType* t2){ return t1->GetSourceType()->GetName() == "REAL" && 
											t2->GetSourceType()->GetName() == "INTEGER"; }
void FillMaps(){
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	priority[":="] = 5;
	cmdNames[asmMov] = "mov"; cmdNames[asmAdd] = "add"; cmdNames[asmSub] = "sub";
	cmdNames[asmDiv] = "div"; cmdNames[asmMul] = "mul"; cmdNames[asmIDiv] = "idiv"; 
	cmdNames[asmIMul] = "imul"; cmdNames[asmPush] = "push"; cmdNames[asmPop] = "pop"; 
	cmdNames[asmCmp] = "cmp"; cmdNames[asmJmp] = "jmp"; cmdNames[asmDw] = "dw"; 
	cmdNames[asmDb] = "db"; cmdNames[asmUnknown] = "?";cmdNames[asmCode] = ".code";
	cmdNames[asmData] = ".data";
}
int FindOpPrior(string str){
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}
Parser::Parser(Scanner& sc, ostream& o): scan(sc), os(o){
	SymTable* table = new SymTable();
	tableStack = new SymTableStack(); 
	FillMaps();
	scan.Next();
	isRecord = false;
	isAccess = false;
	nextScan = true;
	idFound = true;
	SymTypeInteger* i = new SymTypeInteger("INTEGER");
	SymTypeReal* f = new SymTypeReal("REAL");
	SymType* s = new SymTypeReal("STRING");
	table->insert(Sym("INTEGER", i));
	table->insert(Sym("REAL", f));
	table->insert(Sym("STRING", s));
	tableStack->push_back(table);
	lowerTable = tableStack->begin();
	curIdent = "";
	Asm = new AsmCode();
}
void Parser::TryError(bool f, string mes) {
	if (f)
		throw Error(mes, TokPos(), TokLine());
}
void Parser::TryError(bool f, string mes, int pos, int line) {
	if (f)
		throw Error(mes, pos, line);
}
void CheckIsInt(NodeExpr* expr, int i, int j){
	if (!expr->IsInt())
		throw Error("Integer index expected", i, j);
}
void Parser::CheckEof(){
	scan.Next();
	TryError(TokType() == ttEOF, "Unexpected end of file");
}
void Parser::ParseDecl(SymStIt curTable, bool main){
	TokenType tt = TokType();
	while (tt == ttType || tt == ttVar || tt == ttConst || tt == ttProcedure || tt == ttFunction){
		scan.Next();
		switch(tt){
		case ttType:
			ParseTypeConstBlock(curTable, ttType);
			break;
		case ttVar:
			ParseVarRecordBlock(curTable, ttVar);
			break;
		case ttConst:
			ParseTypeConstBlock(curTable, ttConst);
			break;
		case ttProcedure: case ttFunction:
			TryError(!main, "Embedded procedures are not allowed");
			tt == ttProcedure ? ParseProcedure(curTable, true, false) : ParseProcedure(curTable, true, true);
			break;
		}
		tt = TokType();
	}
}
SymTable::iterator Parser::FindS(string s, SymStIt curTable, SymStIt end){
	SymStIt curIt = curTable;
	SymTable::iterator it;
	idFound = true;
	bool f = false;
	while(true) {
		it = (**curIt).find(UpCase(s));
		if (it != (*curIt)->end()){
			f = true;
			break;
		}
		else if (curIt != end)
				--curIt;
		else
			break;
	};
	if (!f)
		idFound = false;
	return it;
}
SymTable::iterator Parser::FindS(string s, SymTable* curTable){	return curTable->find(UpCase(s)); }
Statement* Parser::ParseStatement(SymStIt curTable){
	Statement* res = NULL;
	TryError(TokType() != ttIdentifier && !IsKeyWord(TokType()), "Unexpected lexem found");
	if (IsKeyWord(TokType())){
		switch(TokType()){
			case ttIf:
				res = ParseIf(curTable);
				break;
			case ttWhile:
				res = ParseWhile(curTable);
				break;
			case ttRepeat:
				res = ParseRepeat(curTable);
				break;
			case ttFor:
				res = ParseFor(curTable);
				break;
			case ttBreak:
				res = new StmtBreak();
				scan.Next();
				break;
			case ttContinue:
				res = new StmtContinue();
				scan.Next();
				break;
			case ttBegin:
				res = ParseBlock(curTable, false);
				break;
			default:
				throw Error("Unexpected keyword found", TokPos(), TokLine());				
		}
	} 
	else{
		SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		TryError(!idFound, "Unknown identifier");
		TryError(!(it->second->IsProc() || it->second->IsFunc() || it->second->IsVar()), "Invalid lexem");
		res = it->second->IsVar() ? (Statement*)ParseAssignment(curTable) : (Statement*)ParseProcedure(curTable);
	}
	return res;
}
StmtBlock* Parser::ParseBlock(SymStIt curTable, bool main){
	list<Statement*> stmts;
	StmtBlock* res = NULL;
	if (TokType() == ttBegin){
		scan.Next();
		while (TokType() != ttEnd){
			stmts.push_back(ParseStatement(curTable));
			if (TokType() != ttSemi){
				TryError(TokType() != ttEnd, "; expected, but " + TokVal() + " found");
				break;
			}
			else
				scan.Next();
		}
		scan.Next();
	}
	else
		stmts.push_back(ParseStatement(curTable));
	TryError(main && TokType() != ttDot, ". expected, but " + TokVal() + " found");
	res = new StmtBlock(stmts);
	return res;
}
StmtWhile* Parser::ParseWhile(SymStIt curTable){
	NodeExpr* expr = ParseNext(curTable);
	TryError(!expr->IsInt(), "Condition must be integer");
	Statement* body = NULL;
	TryError(TokType() != ttDo, "Do expected, but " + TokVal() + " found");
	scan.Next();
	body = ParseBlock(curTable, false);
	return new StmtWhile(expr, body);
}
StmtRepeat* Parser::ParseRepeat(SymStIt curTable){
	list<Statement*> stmts;
	StmtBlock* body = NULL;
	scan.Next();
	while (TokType() != ttUntil){
		bool b = TokType() == ttBegin;
		b ? stmts.push_back(ParseBlock(curTable, false)) : stmts.push_back(ParseStatement(curTable));
		if (TokType() != ttSemi){
			string str = b ? "Until" : ";";
			TryError(TokType() != ttUntil, str + " expected, but " + TokVal() + " found");
			if (!b) 
				break;
		}
		else
			scan.Next();
	}
	body = new StmtBlock(stmts);
	NodeExpr* expr = ParseNext(curTable);
	TryError(!expr->IsInt(), "Condition must be integer");
	return new StmtRepeat(expr, body);
}
StmtProcedure* Parser::ParseProcedure(SymStIt curTable){
	NodeExpr* expr =  ParseSimple(curTable, 5);
	if (!expr->IsFunction()){
		TryError(expr->IsBinaryOp() && expr->GetSymbol()->GetName() == ":=", "Left part of assignment must be lvalue", expr->GetPos(), expr->GetLine());
		throw Error("Invalid expression", TokPos(), TokLine());
	}
	SymProc* proc = (SymProc*)expr->GetSymbol();
	return new StmtProcedure(proc, ((FunctionCall*)expr)->GetArgs());
}
StmtIf* Parser::ParseIf(SymStIt curTable){
	scan.Next();
	NodeExpr* expr = ParseSimple(curTable, 5);
	TryError(!expr->IsInt(), "Condition must be integer");
	TryError(TokType() != ttThen, "Then expected, but " + TokVal() + " found");
	scan.Next();
	Statement* first = ParseBlock(curTable, false);
	Statement* second = NULL;
	if (TokType() == ttElse){
		scan.Next();
		second = ParseBlock(curTable, false);
	}
	return new StmtIf(expr, first, second);
}
StmtFor* Parser::ParseFor(SymStIt curTable){
	bool to = true;
	scan.Next();
	TryError(TokType() != ttIdentifier, "Identifier expected");
	SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	TryError(!idFound, "Unknown identifier");
	idFound = true;
	Symbol* symb = it->second;
	TryError(!symb->IsVar() || symb->IsConst() || symb->GetType()->IsArray() || symb->GetType()->IsRecord(), 
				"Illegal counter variable");
	TryError(!symb->GetType()->IsInt(), "Loop variable must be integer");
	scan.Next();
	TryError(TokType() != ttAssign, "Invalid identifier initialization");
	NodeExpr* init = ParseNext(curTable);
	TryError(!init->IsInt(), "Init expression must be integer");
	TokType() == ttDownto ? to = false : TryError(TokType() != ttTo,"To expected, but " + TokVal() + " found");
	NodeExpr* finit = ParseNext(curTable);
	TryError(!finit->IsInt(), "Final expression must be integer");
	TryError(TokType() != ttDo, "Do expected, but " + TokVal() + " found");
	scan.Next();
	StmtBlock* body = ParseBlock(curTable, false);
	return new StmtFor((SymVar*)it->second, init, finit, body, to);
}
StmtAssign* Parser::ParseAssignment(SymStIt curTable){
	NodeExpr* expr = ParseSimple(curTable, 5);
	BinaryOp* res = new BinaryOp(expr->GetSymbol(), expr->GetPos(), expr->GetLine(), expr, NULL);
	TryError(expr->GetValue() != ":=", "Illegal expression");
	res = (BinaryOp*)expr;
	NodeExpr* lExpr = expr;
	while(lExpr->IsBinaryOp())
		lExpr = ((BinaryOp*)lExpr)->GetLeft();
	TryError(!res->GetLeft()->LValue(), "Left part of assignment must be lvalue", lExpr->GetPos(), lExpr->GetLine());
	Symbol* t1 = res->GetLeft()->GetType();
	Symbol* t2 = res->GetRight()->GetType();
	TryError(!(t1 && t2 && (EqTypes((SymType*)t1, (SymType*)t2) || IToR((SymType*)t1,(SymType*) t2))), 
				"Incompatible types in assignment", lExpr->GetPos(), lExpr->GetLine());
	return new StmtAssign(res->GetLeft(), res->GetRight());
}
string Parser::CheckCurTok(SymStIt curTable, string blockName, SymTable* tbl){
	TryError(TokType() != ttIdentifier, "Invalid lexem in " + blockName + " block");
	string s = TokVal();
	SymTable::iterator it = FindS(TokVal(), tbl);
	TryError(it != tbl->end(), "Identifier already declared");
	idFound = true;
	CheckEof();
	return s;
}
SymTable* Parser::GetArgs(SymStIt curTable, string name, list<string>** arg_names){
	list<string> args;
	SymTable* argsTable = new SymTable();
	SymType* type = NULL;
	int args_num = 0;
	if (TokType() == ttLeftParentheses){
		while (TokType() != ttRightParentheses){
			CheckEof();
			TokenType tt = TokType();
			if (tt == ttVar)
				CheckEof();
			while (TokType() != ttSemi && TokType() != ttRightParentheses){
				string par = CheckCurTok(curTable, "function", argsTable);
				args.push_back(par);
				(*arg_names)->push_back(par);
				++args_num;
				if (TokType() == ttPoint)
					CheckEof();
				else if (TokType() == ttColon){
					CheckEof();
					type = (SymType*)ParseType(curTable, false);
					TryError(UnnamedType(type), "Invalid type");
					CheckEof();
					break;
				}
				else
					TryError(TokType() != ttRightParentheses && TokType() != ttSemi, "Invalid lexem in function declaration");
			}
			for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
				SymTable::iterator it1 = FindS(*it, argsTable);
				TryError(it1 != argsTable->end() || *it == name, "Identifier already declared");
				(*argsTable)[*it] = (tt == ttVar) ? (SymVarParam*)new SymVarParamByRef(*it, type) 
									: (SymVarParam*)new SymVarParamByValue(*it, type);
			}
			args.clear();			
		}
		scan.Next();
	}
	return argsTable;
}
Symbol* Parser::ParseProcedure(SymStIt curTable, bool newProc, bool func){
	SymProc* res = NULL;
	string ident = TokVal();
	if (newProc)
		string ident = func ? CheckCurTok(curTable, "function", *curTable) : 
							CheckCurTok(curTable, "procedure", *curTable);
	list<string>* arg_names = new list<string>;
	SymTable* argTable = GetArgs(curTable, ident, &arg_names);
	SymTable* locals = new SymTable();
	SymType* type = NULL;
	if (func){
		TryError(TokType() != ttColon, "Function type expected");
		CheckEof();
		type = (SymType*)ParseType(curTable, false);
		CheckEof();
	}
	tableStack->push_back(argTable);
	tableStack->push_back(locals);
	if (TokType() != ttSemi)
		TryError(TokType() == ttEOF, "Unexpected end of file");
	TryError(TokType() != ttSemi, "; expected end of file, but " + TokVal() + " found");
	CheckEof();
	StmtBlock* body = NULL;
	res = func ? (SymProc*)new SymFunc(ident, argTable, locals, arg_names, body, type) : new SymProc(ident, argTable, locals, arg_names, body);
	if (func){
		SymVarLocal* var = new SymVarLocal("RESULT", type);
		locals->insert(Sym("RESULT", var));
	}
	if (newProc)
		(**curTable)[ident] = res;
	/*if (TokType() != ttForward)
		os << "\n";
	res->Print(os, false);*/
	if (TokType() != ttForward){
		SymStIt it;
		for (SymStIt i = tableStack->begin(); i != tableStack->end(); ++i)
			it = i;
		ParseDecl(it, false);
		body = ParseBlock(it, false);
		res->SetBody(body);
		/*res->PrintBody(os);
		os << "\n";*/
	}else {
		CheckEof();
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
	}
	tableStack->pop_back();
	tableStack->pop_back();
	scan.Next();
	return res;
}
SymTable* Parser::ParseVarRecordBlock(SymStIt curTable, TokenType tt){ 
	list<string> idents;
	SymTable* curTbl = NULL;
	curTbl = (tt == ttVar) ? *curTable : new SymTable();
	string b_name = (tt == ttVar) ? "var" : "record";
	while(true){
		string s = CheckCurTok(curTable, b_name, curTbl);
		idents.push_back(s);
		if (TokType() == ttPoint)
			CheckEof();
		else {
			TryError(TokType() != ttColon, "Invalid lexem in " + b_name + " block");
			CheckEof();
			SymType* type = (SymType*)ParseType(curTable, false);
			CheckEof();
			if (tt == ttVar && type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, curTbl);
				TryError(it != curTbl->end(), "Identifier already declared");
				SymVar* v = (tt == ttVar) ? (SymVar*)new SymVarGlobal(*it1, type) : (SymVar*)new SymVarLocal(*it1, type);
				(*curTbl)[*it1] = v;
				/*if (tt == ttVar){
					v->Print(os, UnnamedType(type));
					os << "\n";
				}*/
			}
			idents.clear();
			SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
			if (tt == ttVar){
				scan.Next();
				if (TokType() == ttVar)
					scan.Next();
				else
					if (AnothBlock(TokVal()))
						return curTbl;		
			}
			else{
				CheckEof();
				if (TokType() == ttEnd)
					break;
			}
		}
			
	}
	if (tt == ttVar)
		scan.Next();
	return curTbl;
}
Symbol* Parser::ParseConst(SymStIt curTable, bool newConst){
	SymVarConst* res = NULL;
	SymType* type = NULL;
	string curId = curIdent;
	if (IsConstType(TokType())){
		if (IsIntType(TokType())){
			type = (SymType*)(*lowerTable)->find("INTEGER")->second;
			res = new SymVarConstInt(curId, type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)(*lowerTable)->find("REAL")->second;
			res = new SymVarConstReal(curId, type, ((RealLiteral*)scan.GetToken())->GetVal());
		}
	} 
	else {
		TryError(TokType() != ttIdentifier, "Invalid lexem in constant declaration");
		string s = TokVal();
		SymTable::iterator it = FindS(s, curTable, tableStack->begin());
		TryError(!idFound, "Unknown constant name");
		idFound = true;
		TryError(!it->second->IsConst(), "Invalid constant name");
		type = ((SymVarConst*)(it->second))->GetType();
		if (!newConst)
			curId = s;
		res = type->IsInt() ? (SymVarConst*)new SymVarConstInt(curId, type, ((SymVarConstInt*)(it->second))->GetValue()):
							  (SymVarConst*)new SymVarConstReal(curId, type, ((SymVarConstReal*)(it->second))->GetValue());
	}
	return res;
}
void Parser::ParseTypeConstBlock(SymStIt curTable, TokenType tt){
	Symbol * res = NULL;
	string b_name = (tt == ttType) ? "type" : "const";
	while (true){
		string ident = CheckCurTok(curTable, b_name, *curTable);
		TryError(TokType() != ttEq, "Invalid lexem in " + b_name + " declaration");
		CheckEof();
		if (tt == ttType){
			res = ParseType(curTable, true);
			CheckEof();
			res->SetName(ident);
			//res->Print(os, true);
		}
		else{
			curIdent = ident;
			res = ParseConst(curTable, true);
			CheckEof();
			curIdent = "";
			//res->Print(os, true);
		}
		//os << ";\n";
		(**curTable)[ident] = res;
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
		scan.Next();
		if (TokType() == ttType && tt == ttType || TokType() == ttConst && tt == ttConst)
				scan.Next();
		else
			if (AnothBlock(TokVal()))
				return;	
	}
}
Symbol* Parser::ParseType(SymStIt curTable, bool newType){
	SymType* res = NULL;
	TokenType tt = TokType();
	string s = TokVal();
	bool isPntr =  false;
	if (TokType() == ttCaret){
		CheckEof();
		isPntr = true;
		s = TokVal();
	}
	SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	if (idFound){
		if (isPntr)
			res = new SymTypePointer(curIdent, (SymType*)it->second);	
		else{
			if (newType){
				TryError(!it->second->IsType(), "Invalid lexem in type declaration");
				res = new SymTypeAlias(curIdent, (SymType*)it->second);	
			} 
			else
				res = (SymType*)it->second;
		}
	} 
	else {
		TryError(isPntr, "Invalid pointer declaration");
		switch(tt){
		case ttRecord:
			CheckEof();
			res = new SymTypeRecord(toString(id++), ParseVarRecordBlock(curTable, ttRecord));
			break;
		case ttArray:
			CheckEof();
			res = (SymType*)ParseArray(curTable);
			break;
		default:
			TryError(it == (*curTable)->end() && !newType, "Unknown type");
		}
	}
	return res;
}
Symbol* Parser::ParseArray(SymStIt curTable){
	Symbol* res = NULL;
	TryError(TokType() != ttLeftBracket, "Invalid array declaration");
	list<Symbol*> left, right;
	CheckEof();
	while (TokType() != ttRightBracket){
		Symbol* l = ParseConst(curTable, false);
		CheckEof();
		TryError(TokType() != ttDblDot, "Invalid array declaration");
		CheckEof();
		Symbol* r = ParseConst(curTable, false);
		CheckEof();
		TryError(!(l->GetType()->IsInt() && r->GetType()->IsInt()), "Invalid array bounds");
		TryError(((SymVarConstInt*)l)->GetValue() > ((SymVarConstInt*)r)->GetValue(), "Upper bound less than lower bound");
		left.push_back(l);
		right.push_back(r);
		if (TokType() == ttPoint)
			CheckEof();
	}
	scan.Next();
	TryError(TokType() != ttOf, "Invalid array declaration");
	CheckEof();
	Symbol* type = ParseType(curTable, false);
	list<Symbol*>::reverse_iterator itl = left.rbegin();
	list<Symbol*>::reverse_iterator itr = right.rbegin();
	for (; itl != left.rend() && itr != right.rend(); ++itl, ++itr)
	{
		res = new SymTypeArray(toString(id++), (SymType*)type, (SymVarConst*)*itl, (SymVarConst*)*itr);
		type = res;
	}
	return res;
}
int FunctionCall::FillTree(int i, int j){
	unsigned int k = 0;
	int tmp = j;
	j = FillTreeOp(i, j, "()");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i + 4, j, GetValue()) - 2, false);
	int j1 = j;
	for (list<NodeExpr*>::iterator it = args.begin() ;it != args.end(); ++it){
		PaintBranch(i, j1, 0,  j - j1 + 2, true);
		j1 = j;
		j += (*it)->FillTree(i + 4, j + 2);
	}
	return j - tmp + 2;
}
int UnaryOp::FillTree(int i, int j){
	j = FillTreeOp(i, j, GetValue());
	int h = child->FillTree(i + 4, j);
	return h + 2;
}
int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

NodeExpr* Parser::ParseSimple(SymStIt curTable, int prior){
	if (prior == 1)
		return ParseFactor(curTable);
	SymType* type = NULL;
	int pos = TokPos();
	int line = TokLine();
	NodeExpr* res = ParseSimple(curTable, prior - 1);
	string s = TokVal();
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseSimple(curTable, prior - 1);
		TryError(!expr1, "Lexem expected, EOF found");
		TryError(!res->GetType(), "Variable must be typed", pos, line);
		TryError(!expr1->GetType(), "Variable must be typed");
		if (expr1->IsInt() && (((SymType*)res->GetType())->IsScalar() && !res->IsInt())){
			expr1 = new UnaryOp(new Symbol("ItoR"), pos, line, expr1);
			expr1->SetType((SymType*)((**lowerTable)["REAL"]));
		}
		else if (prior != 5 && (res->IsInt() && (((SymType*)expr1->GetType())->IsScalar() && !expr1->IsInt()))){
			res = new UnaryOp(new Symbol("ItoR"), pos, line, res);
			res->SetType((SymType*)((**lowerTable)["REAL"]));
		}
		res = new BinaryOp(new Symbol(str), pos, line, res, expr1);
		if (prior == 5)
			break;
		type = (res->IsInt()) ? (SymType*)((**lowerTable)["INTEGER"]) : (SymType*)((**lowerTable)["REAL"]);
		res->SetType(type);
		TryError(!((SymType*)res->GetType())->IsScalar() || !((SymType*)expr1->GetType())->IsScalar(), 
				"Invalid types in binary operation", pos, line);
		pos = TokPos();
		line = TokLine();
	}
	return res;
}
NodeExpr* Parser::ParseNext(SymStIt curTable){
	scan.Next();
	NodeExpr* expr1 = ParseSimple(curTable, 4);
	TryError(!expr1, "Lexem expected, EOF found");
	return expr1;
}
Const* Parser::ParseConst(SymStIt curTable){
	Const* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	scan.Next();
	CheckAccess(TokVal(), TokPos(), TokLine());
	nextScan = false;
	res = new Const((SymVarConst*)(it->second), pos, line);
	res->SetType(((SymVarConst*)it->second)->GetType());
	return res;
}
NodeExpr* Parser::ParseVariable(SymStIt curTable, NodeExpr* res){
	string s;
	Symbol* var = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (!res) {
		SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		var = (SymVar*)(it->second);
		s = TokVal();
		Symbol* symb = new Symbol(s);
		res = new NodeExpr(symb, pos, line);
	}
	else{
		s = res->GetValue();
		var = new SymVar(res->GetSymbol()->GetName(), (SymType*)res->GetType());
	}
	TryError(!var->GetType(), "Variable must be typed", pos, line);
	if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
			res = new Variable(var, pos, line);
	else
		while (true){
			if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
				break;
			if (var->IsFunc()){
				res = ParseFunc(curTable, res, &var, pos, line);
				nextScan = false;
			}
			else{
				if (nextScan)
					scan.Next();
				if (var->GetType()->IsArray() && TokType() != ttLeftBracket || var->GetType()->IsRecord() && TokType() != ttDot){
					CheckAccess(TokVal(), TokPos(), TokLine());
					if (res->GetValue() == s)
						res = new Variable(var, pos, line);
					nextScan = false;
					break;
				}
				else
					res = (var->GetType()->IsArray()) ? (NodeExpr*)ParseArr(curTable, res, &var, pos, line) : (NodeExpr*)ParseRecord(curTable, res, &var, pos, line);
			}
			pos = TokPos();
			line = TokLine();
		}
	res->SetType(var->GetType());
	return res;
}
ArrayAccess* Parser::ParseArr(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	SymType* type = (*var)->GetType();
	if (TokType() == ttLeftBracket){
		scan.Next();
		while (true){
			NodeExpr* expr = ParseSimple(curTable, 4);
			CheckIsInt(expr, expr->GetPos(), expr->GetLine());
			Symbol* symb = new Symbol("[]");
			res = new ArrayAccess(symb, pos, line, res, expr);
			if (expr->IsConst()){
				int val = ((SymVarConstInt*)expr->GetSymbol())->GetValue();
				int top = ((SymVarConstInt*)((SymTypeArray*)type)->GetTop())->GetValue();
				int bottom = ((SymVarConstInt*)((SymTypeArray*)type)->GetBottom())->GetValue();
				TryError(val > top || val < bottom, "Range check error", expr->GetPos(), expr->GetLine());
			}
			type = ((SymTypeArray*)type)->GetElemType();
			res->SetType(type);
			if(TokType() == ttRightBracket){
				if (!nextScan)
					scan.Next();
				break;
			}
			if (TokType() == ttPoint){
				TryError(!type->IsArray(), "Invalid num of indexes");
				scan.Next();
				TryError(TokType() == ttRightBracket, "Invalid array access");
			}
		}
		pos = TokPos();
		line = TokLine();
	}
	*var = new SymVar(toString(id++), type);
	res->SetType(type);
	return (ArrayAccess*)res;
}
FunctionCall* Parser::ParseFunc(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	SymType* type = ((SymFunc*)*var)->GetType();
	string s = TokVal();
	scan.Next();
	list<string>* arg_names = ((SymFunc*)*var)->GetArgNames();
	SymTable* arg_table = ((SymFunc*)*var)->GetArgsTable();
	list<NodeExpr*>args;
	list<string>::iterator it = arg_names->begin();
	SymTable::iterator it1;
	if (TokType() == ttLeftParentheses){
		scan.Next();
		while (TokType() != ttRightParentheses){
			it1 = FindS(*it, arg_table);
			NodeExpr* expr = ParseSimple(curTable, 4);
			SymVar* curVar = ((SymVar*)(it1->second));
			if (curVar->IsParamByRef()){
				TryError(!(EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType())), 
					"Incompatible types", expr->GetPos(), expr->GetLine());
				TryError(!expr->LValue(), "Variable identifier expected", expr->GetPos(), expr->GetLine());
			}
			else{
				TryError(!EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType()) && 
					!IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType()), "Incompatible types", 
					expr->GetPos(), expr->GetLine());
				if (IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType())){
					expr = new UnaryOp(new Symbol("ItoR"), TokPos(), TokLine(), expr);
					expr->SetType((SymType*)((**lowerTable)["REAL"]));
				}
			}
			args.push_back(expr);
			++it;
			if (TokType() == ttPoint){
				TryError(it == arg_names->end(), "Invalid parameters num");
				scan.Next();
				TryError(TokType() == ttRightParentheses, "Invalid function call");
			}
		}
		scan.Next();
	}
	else
		nextScan = false;
	TryError(it != arg_names->end(), "Invalid parameters num");
	if (!type)
		nextScan = false;
	res = new FunctionCall(*var, pos, line, args);
	if (type)
		*var = new SymVar(toString(id++), type);
	res->SetType(type);
	return (FunctionCall*)res;
}
RecordAccess* Parser::ParseRecord(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	string s = TokVal();
	SymType* type = (*var)->GetType();
	NodeExpr* expr = NULL;
	Symbol* symb = NULL;
	if (TokType() == ttDot){
		scan.Next();
		while (true){
			TryError(TokType() != ttIdentifier, "Invalid record access");
			SymTable* recTable = ((SymTypeRecord*)(*var)->GetType())->GetFileds();
			SymTable::iterator it = FindS(TokVal(), recTable);
			TryError(it == recTable->end(), "Unknown identifier in record access");
			*var = (SymVar*)(it->second);
			symb = new Symbol(TokVal());
			expr = new NodeExpr(symb, pos, line);
			res = new RecordAccess(*var, pos, line, res, expr);
			res->SetType(type);
			scan.Next();
			if (TokType() == ttDot){
				type = (*var)->GetType();
				TryError(!type->IsRecord(), "Invalid record access", pos, line);
				scan.Next();
			}
			else {
				nextScan = false;
				break;
			}
		}
		pos = TokPos();
		line = TokLine();
	}
	nextScan = false; 
	res->SetType(type);
	return (RecordAccess*)res;
}
NodeExpr* Parser::ParseFactor(SymStIt curTable){
	NodeExpr* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	string s = TokVal();
	if (TokType() == ttEOF)
		return NULL;
	TryError(TokType() == ttBadToken, "Invalid lexem", pos, line);
	if (IsUnaryOp(TokVal())){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseFactor(curTable);
		TryError(!expr1, "Lexem expected, EOF found");
		Symbol* symb = new Symbol(str);
		res = new UnaryOp(symb, pos, line, expr1);
		TryError(!((SymType*)expr1->GetType())->IsScalar(), "Invalid unary operation", pos, line);
		res->SetType(expr1->GetType()); 
	}
	else if (TokType() == ttIdentifier){
		SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		TryError(!idFound, "Unknown identifier");
		TryError(it->second->IsType(), "Invalid identifier");
		if (it->second->IsConst())
			res = ParseConst(curTable);
		else if (it->second->IsVar() || it->second->IsFunc())
			res = ParseVariable(curTable, NULL);
		else if (it->second->IsProc())
			res = ParseFunc(curTable, new NodeExpr(new Symbol(TokVal()), pos, line), &(it->second), pos, line);
		if (nextScan)
			scan.Next();
		nextScan = true;
	}
	else if (IsLiteral(TokType())){
		SymVarConst* symb = NULL;
		SymType* type = NULL;
		if (IsIntType(TokType())){
			type = (SymType*)((**lowerTable)["INTEGER"]);
			symb = new SymVarConstInt(TokVal(),type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)((**lowerTable)["REAL"]);
			symb = new SymVarConstReal(TokVal(),type, ((RealLiteral*)scan.GetToken())->GetVal());
		}
		else{
			type = (SymType*)((**lowerTable)["STRING"]);
			symb = new SymVarConstString(TokVal(),type, ((StringLiteral*)scan.GetToken())->GetVal());
		}
		res = new Const(symb, pos, line);
		res->SetType(type);
		scan.Next();
	}
	else if (TokType() == ttLeftParentheses){
		scan.Next();
		TryError(TokType() == ttRightParentheses, "Empty bracket sequence", pos, line);
		res = ParseSimple(curTable, 5);
		TryError(TokType() != ttRightParentheses, "Unclosed bracket", pos, line);
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(curTable, res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}