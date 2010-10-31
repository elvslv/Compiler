#include "Parser.h"
#include "Symbols.h"
static int id = 0;
void PrintArgs(ostream& os, SymTable* args){
	if (!args->empty()){	
		os << "(";  
		int i = 0;
		for (SymTable::iterator it = args->begin(); it != args->end(); ++it)
			(*it->second).Print(os, false);
		os << ")";
	}
}
void SymProc::Print(ostream& os, bool f) { 
	os << "procedure "<< name;
	PrintArgs(os, args);
	os << ";";
	if (body){
		os << "\n";
		body->Print(os, 0);
	}
}
void SymFunc::Print(ostream& os, bool f) { 
	os << "function " << name;
	PrintArgs(os, args);
	os << ": ";
	type->Print(os, false);
	os << ";";
	if (body){
		os << "\n";
		body->Print(os, 0);
	}
}
string NodeExpr::GetValue() { return symbol->GetName(); }
bool Variable::IsInt() { return symbol->GetType()->IsInt(); }
bool Variable::IsReal() { return symbol->GetType()->IsReal(); }
bool Const::IsInt() {return symbol->GetType()->IsInt(); }
bool Const::IsReal() {return symbol->GetType()->IsReal(); }
bool BinaryOp::IsInt() {return right->IsInt() && left->IsInt() && IsIntOperator(GetValue()) || 
						 ((SymType*)right->GetType())->IsScalar() && ((SymType*)right->GetType())->IsScalar() && IsLogicOperator(GetValue()); }
bool ArrayAccess::IsInt() { return ((SymType*)GetType())->IsInt(); }
bool ArrayAccess::IsReal() { return ((SymType*)GetType())->IsReal(); }
bool RecordAccess::IsInt() { return ((SymType*)GetType())->IsInt(); }
bool RecordAccess::IsReal() { return ((SymType*)GetType())->IsReal(); }
bool FunctionCall::IsInt() {return ((SymType*)GetType())->IsInt(); }
bool FunctionCall::IsReal() { return ((SymType*)GetType())->IsReal(); }
bool UnaryOp::IsInt() {return child->IsInt() && IsIntOperator(GetValue()); }
bool UnaryOp::IsReal() { return child->IsReal(); }
bool EqTypes(SymType* t1, SymType* t2){
	while(t1->IsAlias())
		t1 = ((SymTypeAlias*)t1)->GetRefType();
	while (t2->IsAlias())
		t2 = ((SymTypeAlias*)t2)->GetRefType();
	return t1 == t2;
}

bool IToR(SymType* t1, SymType* t2){
	while(t1->IsAlias())
		t1 = ((SymTypeAlias*)t1)->GetRefType();
	while (t2->IsAlias())
		t2 = ((SymTypeAlias*)t2)->GetRefType();
	return t1->GetName() == "REAL" && t2->GetName() == "INTEGER";
}

Parser::Parser(Scanner& sc, ostream& o): scan(sc), os(o){
		SymTable* table = new SymTable();
		tableStack = new SymTableStack(); 
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
		SymTypeInteger* i = new SymTypeInteger("INTEGER");
		SymTypeReal* f = new SymTypeReal("REAL");
		table->insert(Sym("INTEGER", i));
		table->insert(Sym("REAL", f));
		tableStack->push_back(table);
		curIdent = "";
		nextScan = true;
		idFound = true;
	}

void CheckIsInt(NodeExpr* expr, int i, int j){
	if (!expr->IsInt())
		throw Error("Integer index expected", i, j);
}

void Parser::CheckEof(){
	scan.Next();
	if (TokType() == ttEOF)
		throw Error("Unexpected end of file", TokPos(), TokLine());
}
void Parser::ParseDecl(SymStIt curTable){
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
		case ttProcedure:
			ParseProcedure(curTable, true, false);
			break;
		case ttFunction:
			ParseProcedure(curTable, true, true);
		}
		tt = TokType();
	}
}

SymTable::iterator Parser::FindS(string s, SymStIt curTable, SymStIt end){
	SymStIt curIt = curTable;
	SymTable::iterator it;
	idFound = true;
	bool f = false;
	do {
		it = (**curIt).find(UpCase(s));
		if (it != (*curIt)->end()){
			f = true;
			break;
		}
		else{
			if (curIt != end)
				--curIt;
		}
	} while (curIt != end);
	if (!f)
		idFound = false;
	return it;
}

SymTable::iterator Parser::FindS(string s, SymTable* curTable){
	SymTable::iterator it = curTable->find(UpCase(s));
	return it;
}

Statement* Parser::ParseStatement(SymStIt curTable){
	Statement* res = NULL;
	if (TokType() != ttIdentifier && !IsKeyWord(TokType()))
		throw Error("Unexpected lexem found", TokPos(), TokLine());
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
		if (!&it)
			throw Error("Unknown identifier", TokPos(), TokLine());
		if (it->second->IsProc() || it->second->IsFunc())
			res = ParseProcedure(curTable);
		else if (it->second->IsVar())
				res = ParseAssignment(curTable);
		else
			throw Error("Invalid lexem", TokPos(), TokLine());
	}
	return res;
}

StmtBlock* Parser::ParseBlock(SymStIt curTable, bool main){
	list<Statement*> stmts;
	StmtBlock* res = NULL;
	bool f = false;
	if (TokType() == ttBegin){
		f = true;
		scan.Next();
		while (TokType() != ttEnd){
			stmts.push_back(ParseStatement(curTable));
			if (TokType() != ttSemi){
				if (TokType() != ttEnd)
					throw Error("; expected, but " + TokVal() + " found", TokPos(), TokLine());
				break;
			}
			else
				scan.Next();
		}
		scan.Next();
	}
	else
		stmts.push_back(ParseStatement(curTable));
	res = new StmtBlock(stmts);
	return res;
}

StmtWhile* Parser::ParseWhile(SymStIt curTable){
	NodeExpr* expr = ParseNext(curTable);
	if (!expr->IsInt())
		throw Error("Condition must be integer", TokPos(), TokLine());
	Statement* body = NULL;
	if (TokType() != ttDo)
		throw Error("Do expected, but " + TokVal() + " found", TokPos(), TokLine());
	scan.Next();
	body = ParseBlock(curTable, false);
	return new StmtWhile(expr, body);
}

StmtRepeat* Parser::ParseRepeat(SymStIt curTable){
	list<Statement*> stmts;
	StmtBlock* body = NULL;
	scan.Next();
	while (TokType() != ttUntil){
		stmts.push_back(ParseStatement(curTable));
		scan.Next();
	}
	body = new StmtBlock(stmts);
	NodeExpr* expr = ParseNext(curTable);
	if (!expr->IsInt())
		throw Error("Condition must be integer", TokPos(), TokLine());
	return new StmtRepeat(expr, body);
}

StmtProcedure* Parser::ParseProcedure(SymStIt curTable){
	NodeExpr* expr =  ParseSimple(curTable, 5);
	if (!expr->IsFunction()){
		if (expr->IsBinaryOp() && expr->GetSymbol()->GetName() == ":=")
			throw Error("Left part of assignment must be lvalue", expr->GetPos(), expr->GetLine());
		throw Error("Invalid expression", TokPos(), TokLine());
	}
	SymProc* proc = (SymProc*)expr->GetSymbol();
	return new StmtProcedure(proc, ((FunctionCall*)expr)->GetArgs());
}

StmtIf* Parser::ParseIf(SymStIt curTable){
	NodeExpr* expr = ParseNext(curTable);
	if (!expr->IsInt())
		throw Error("Condition must be integer", TokPos(), TokLine());
	if (TokType() != ttThen)
		throw Error("Then expected, but " + TokVal() + " found", TokPos(), TokLine());
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
	if (TokType() != ttIdentifier)
		throw Error("Identifier expected", TokPos(), TokLine());
	SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	if (!idFound)
		throw Error("Unknown identifier", TokPos(), TokLine());
	idFound = true;
	if (!it->second->IsVar())
		throw Error("Variable identifier expected", TokPos(), TokLine());
	if (!it->second->GetType()->IsInt())
		throw Error("Loop variable must be integer", TokPos(), TokLine());
	scan.Next();
	if (TokType() != ttAssign)
		throw Error("Invalid identifier initialization", TokPos(), TokLine());
	NodeExpr* init = ParseNext(curTable);
	if (!init->IsInt())
		throw Error("Init expression must be integer", TokPos(), TokLine());
	if (TokType() == ttDownto)
		to = false;
	else
		if (TokType() != ttTo)
			throw Error("To expected, but " + TokVal() + " found", TokPos(), TokLine());
	NodeExpr* finit = ParseNext(curTable);
	if (!finit->IsInt())
		throw Error("Final expression must be integer", TokPos(), TokLine());
	if (TokType() != ttDo)
		throw Error("Do expected, but " + TokVal() + " found", TokPos(), TokLine());
	scan.Next();
	StmtBlock* body = ParseBlock(curTable, false);
	return new StmtFor((SymVar*)it->second, init, finit, body, to);
}

string Parser::CheckCurTok(SymStIt curTable, string blockName, SymTable* tbl){
	if (TokType() != ttIdentifier)
		throw Error("Invalid lexem in " + blockName + " block", TokPos(), TokLine());
	string s = TokVal();
	SymTable::iterator it = FindS(TokVal(), tbl);
	if (it != tbl->end())
		throw Error("Identifier already declared", TokPos(), TokLine());
	idFound = true;
	CheckEof();
	return s;
}

SymTable* Parser::GetArgs(SymStIt curTable, string name, list<string>** arg_names){
	list<string> args;
	SymTable* argsTable = new SymTable();
	SymType* type = NULL;
	int args_num = 0;
	int index = 0;
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
					if (type->GetName()[0] >= '0' && type->GetName()[0] <= '9')
						throw Error("Invalid type", TokPos(), TokLine());
					CheckEof();
					break;
				}
				else if (TokType() != ttRightParentheses && TokType() != ttSemi)
					throw Error("Invalid lexem in function declaration", TokPos(), TokLine());
			}
			for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
				SymTable::iterator it1 = FindS(*it, argsTable);
				if (it1 != argsTable->end() || *it == name)
					throw Error("Identifier already declared", TokPos(), TokLine());
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
		if (TokType() != ttColon)
			throw Error("Function type expected", TokPos(), TokLine());
		CheckEof();
		type = (SymType*)ParseType(curTable, false);
		CheckEof();
	}
	tableStack->push_back(argTable);
	tableStack->push_back(locals);
	if (TokVal() != ";")
		if (TokVal() == "EOF")
			throw Error("Unexpected end of file", TokPos(), TokLine());
		else
			SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
	CheckEof();
	StmtBlock* body = NULL;
	if (TokType() != ttForward){
		if (TokType() != ttBegin)
			throw Error("Begin expected, but " + TokVal() + " found", TokPos(), TokLine());
		scan.Next();
		SymStIt it;
		for (SymStIt i = tableStack->begin(); i != tableStack->end(); ++i)
			it = i;
		ParseDecl(it);
		body = ParseBlock(it, false);
	}else
	{
		CheckEof();
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
	}
	if (func)
		res = new SymFunc(ident, argTable, locals, arg_names, body, type);
	else
		res = new SymProc(ident, argTable, locals, arg_names, body);
	if (newProc)
		(**curTable)[ident] = res;
	scan.Next();
	res->Print(os, false);
	os << "\n";
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
			if (TokType() != ttColon)
				throw Error("Invalid lexem in " + b_name + " block", TokPos(), TokLine());
			CheckEof();
			SymType* type = (SymType*)ParseType(curTable, false);
			CheckEof();
			if (tt == ttVar && type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, curTbl);
				if (it != curTbl->end())
					throw Error("Identifier already declared", TokPos(), TokLine());
				SymVar* v = (tt == ttVar) ? (SymVar*)new SymVarGlobal(*it1, type) : (SymVar*)new SymVarLocal(*it1, type);
				(*curTbl)[*it1] = v;
				if (tt == ttVar){
					v->Print(os, type->GetName()[0] >= '0' && type->GetName()[0] <= '9');
					os << "\n";
				}
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
			type = (SymType*)(*curTable)->find("INTEGER")->second;
			res = new SymVarConstInt(curId, type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)(*curTable)->find("REAL")->second;
			res = new SymVarConstReal(curId, type, ((RealLiteral*)scan.GetToken())->GetVal());
		}
	} 
	else {
		if (TokType() != ttIdentifier)
			throw Error("Invalid lexem in constant declaration", TokPos(), TokLine());
		string s = TokVal();
		SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		if (!idFound)
			throw Error("Unknown constant name", TokPos(), TokLine());	
		idFound = true;
		if (!it->second->IsConst())
			throw Error("Invalid constant name", TokPos(), TokLine());
		type = ((SymVarConst*)(it->second))->GetType();
		if (!newConst)
			curId = s;
		if (type->IsInt())
			res = new SymVarConstInt(curId, type, ((SymVarConstInt*)(it->second))->GetValue());
		else
			res = new SymVarConstReal(curId, type, ((SymVarConstReal*)(it->second))->GetValue());
	}
	
	return res;
}

void Parser::ParseTypeConstBlock(SymStIt curTable, TokenType tt){
	Symbol * res = NULL;
	string b_name = (tt == ttType) ? "type" : "const";
	while (true){
		string ident = CheckCurTok(curTable, b_name, *curTable);
		if (TokType() != ttEq)
			throw Error("Invalid lexem in " + b_name + " declaration", TokPos(), TokLine());
		CheckEof();
		if (tt == ttType){
			res = ParseType(curTable, true);
			CheckEof();
			res->SetName(ident);
			res->Print(os, true);
		}
		else{
			curIdent = ident;
			res = ParseConst(curTable, true);
			CheckEof();
			curIdent = "";
			res->Print(os, true);
		}
		os << ";\n";
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
				if (!it->second->IsType())
					throw Error("Invalid lexem in type declaration", TokPos(), TokLine());
				res = new SymTypeAlias(curIdent, (SymType*)it->second);	
			} 
			else
				res = (SymType*)it->second;
		}
	} 
	else {
		if (isPntr)
			throw Error("Invalid pointer declaration", TokPos(), TokLine());
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
			if (it == (*curTable)->end() && !newType)
				throw Error("Unknown type", TokPos(), TokLine());
		}
	}
	return res;
}

Symbol* Parser::ParseArray(SymStIt curTable){
	Symbol* res = NULL;
	if (TokType() != ttLeftBracket)
		throw Error("Invalid array declaration", TokPos(), TokLine());
	list<Symbol*> left, right;
	CheckEof();
	while (TokType() != ttRightBracket){
		Symbol* l = ParseConst(curTable, false);
		CheckEof();
		if (TokType() != ttDblDot)
			throw Error("Invalid array declaration", TokPos(), TokLine());
		CheckEof();
		Symbol* r = ParseConst(curTable, false);
		CheckEof();
		if (!(l->GetType()->IsInt() && r->GetType()->IsInt()))
			throw Error("Invalid array bounds", TokPos(), TokLine());
		if (((SymVarConstInt*)l)->GetValue() > ((SymVarConstInt*)r)->GetValue())
			throw Error("Upper bound less than lower bound", TokPos(), TokLine());
		left.push_back(l);
		right.push_back(r);
		if (TokType() == ttPoint)
			CheckEof();
	}
	scan.Next();
	if (TokType() != ttOf)
		throw Error("Invalid array declaration", TokPos(), TokLine());
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
	int s = 0;
	int h = 0;
	for (list<NodeExpr*>::iterator it = args.begin() ;it != args.end(); ++it){
		h = (*it)->FillTree(i + 4, j + 2);
		j = PaintBranch(i, j, 0, 2, true);
		s += h;
	}
	return max(s - tmp + 2, j - tmp + 2);
}

int UnaryOp::FillTree(int i, int j){
	j = FillTreeOp(i, j, GetValue());
	int h = child->FillTree(i + 4, j);
	return j + h + 2;
}

int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

StmtAssign* Parser::ParseAssignment(SymStIt curTable){
	NodeExpr* expr = ParseSimple(curTable, 5);
	BinaryOp* res = new BinaryOp(expr->GetSymbol(), expr->GetPos(), expr->GetLine(), expr, NULL);
	if (expr->GetValue() == ":="){
		res = (BinaryOp*)expr;
		NodeExpr* lExpr = expr;
		while(lExpr->IsBinaryOp())
			lExpr = ((BinaryOp*)lExpr)->GetLeft();
		if (!res->GetLeft()->LValue())
			throw Error ("Left part of assignment must be lvalue", lExpr->GetPos(), lExpr->GetLine());
		Symbol* t1 = res->GetLeft()->GetType();
		Symbol* t2 = res->GetRight()->GetType();
		if (!(t1 && t2 && (EqTypes((SymType*)t1, (SymType*)t2) || IToR((SymType*)t1,(SymType*) t2))))
			throw Error("Incompatible types in assignment", lExpr->GetPos(), lExpr->GetLine());
	}
	//os << "\n";
	//expr->Print(os, 0);
	return new StmtAssign(res->GetLeft(), res->GetRight());
	
}

NodeExpr* Parser::ParseSimple(SymStIt curTable, int prior){
	if (prior == 1)
		return ParseFactor(curTable);
	SymType* type = NULL;
	int pos = TokPos();
	int line = TokLine();
	NodeExpr* res = ParseSimple(curTable, prior - 1);
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseSimple(curTable, prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		if (expr1->IsInt() && (((SymType*)res->GetType())->IsScalar() && !res->IsInt())){
			expr1 = new UnaryOp(new Symbol("ItoR"), pos, line, expr1);
			expr1->SetType((SymType*)((**curTable)["REAL"]));
		}
		else if (prior != 5 && (res->IsInt() && (((SymType*)expr1->GetType())->IsScalar() && !expr1->IsInt()))){
			res = new UnaryOp(new Symbol("ItoR"), pos, line, res);
			res->SetType((SymType*)((**curTable)["REAL"]));
		}
		res = new BinaryOp(new Symbol(str), pos, line, res, expr1);
		if (prior == 5)
			break;
		type = (res->IsInt()) ? (SymType*)((**curTable)["INTEGER"]) : (SymType*)((**curTable)["REAL"]);
		res->SetType(type);
		if (!((SymType*)res->GetType())->IsScalar() || !((SymType*)expr1->GetType())->IsScalar())
			throw Error("Invalid types in binary operation", pos, line);
		pos = TokPos();
		line = TokLine();
	}
	return res;
}

NodeExpr* Parser::ParseNext(SymStIt curTable){
	scan.Next();
	NodeExpr* expr1 = ParseSimple(curTable, 4);
	if (!expr1)
		throw Error("Lexem expected, EOF found", TokPos(), TokLine());
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
	SymType* type = var->GetType();
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
	type = var->GetType();
	res->SetType(type);
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
				if (val > top || val < bottom)
					throw Error("Range check error", expr->GetPos(), expr->GetLine());
			}
			type = ((SymTypeArray*)type)->GetElemType();
			res->SetType(type);
			if(TokType() == ttRightBracket){
				if (!nextScan)
					scan.Next();
				break;
			}
			if (TokType() == ttPoint){
				if (!type->IsArray())
					throw Error("Invalid num of indexes", TokPos(), TokLine());
				scan.Next();
				if (TokType() == ttRightBracket)
					throw Error("Invalid array access", TokPos(), TokLine());
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
				if (!(EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType())))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
				if (!expr->LValue())
					throw Error("Variable identifier expected", expr->GetPos(), expr->GetLine());
			}
			else{
				if (!EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType()) && 
					!IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType()))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
				if (IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType())){
					expr = new UnaryOp(new Symbol("ItoR"), TokPos(), TokLine(), expr);
					expr->SetType((SymType*)((**curTable)["REAL"]));
				}
			}
			args.push_back(expr);
			++it;
			if (TokType() == ttPoint){
				if (it == arg_names->end())
					throw Error("Invalid parameters num", TokPos(), TokLine());
				scan.Next();
				if (TokType() == ttRightParentheses)
					throw Error("Invalid function call", TokPos(), TokLine());
			}
		}
		scan.Next();
	}
	else
		nextScan = false;
	if (it != arg_names->end())
		throw Error("Invalid parameters num", TokPos(), TokLine());
	Symbol* symb = new Symbol(s);
	res = new FunctionCall(symb, pos, line, args);
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
			if (TokType() != ttIdentifier)
				throw Error("Invalid record access", TokPos(), TokLine());
			SymTable* recTable = ((SymTypeRecord*)(*var)->GetType())->GetFileds();
			SymTable::iterator it = FindS(TokVal(), recTable);
			if (it == recTable->end())
				throw Error("Unknown identifier in record access", TokPos(), TokLine());
			*var = (SymVar*)(it->second);
			symb = new Symbol(TokVal());
			expr = new NodeExpr(symb, pos, line);
			res = new RecordAccess(*var, pos, line, res, expr);
			res->SetType(type);
			scan.Next();
			if (TokType() == ttDot){
				type = (*var)->GetType();
				if (!type->IsRecord())
					throw Error("Invalid record access", pos, line);
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
	string s;
	if (TokType() == ttEOF)
		return NULL;
	if (TokType() == ttBadToken)
		throw Error("Invalid lexem", pos, line);
	if (IsUnaryOp(TokVal())){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseFactor(curTable);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		Symbol* symb = new Symbol(str);
		res = new UnaryOp(symb, pos, line, expr1);
		if (!((SymType*)expr1->GetType())->IsScalar())
			throw Error("Invalid unary operation", pos, line);
		res->SetType(expr1->GetType()); 
	}
	else if (TokType() == ttIdentifier){
		SymTable::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		if (!idFound)
			throw Error("Unknown identifier", TokPos(), TokLine());
		idFound = true;
		if (it->second->IsType())
			throw Error("Invalid identifier", TokPos(), TokLine());
		if (it->second->IsConst())
			res = ParseConst(curTable);
		else if (it->second->IsVar() || it->second->IsFunc())
			res = ParseVariable(curTable, NULL);
		if (nextScan)
			scan.Next();
		nextScan = true;
	}
	else if (IsLiteral(TokType())){
		SymVarConst* symb = NULL;
		SymType* type = NULL;
		if (IsIntType(TokType())){
			type = (SymType*)((**curTable)["INTEGER"]);
			symb = new SymVarConstInt(TokVal(),type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else {
			type = (SymType*)((**curTable)["REAL"]);
			symb = new SymVarConstReal(TokVal(),type, ((RealLiteral*)scan.GetToken())->GetVal());
		}
		res = new Const(symb, pos, line);
		res->SetType(type);
		scan.Next();
	}
	else if (TokType() == ttLeftParentheses){
		scan.Next();
		if (TokType() == ttRightParentheses)
			throw Error("Empty bracket sequence", pos, line);
		res = ParseSimple(curTable, 4);
		if (TokType() != ttRightParentheses)
			throw Error("Unclosed bracket", pos, line);
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(curTable, res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}