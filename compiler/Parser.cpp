#include "Parser.h"
static int id = 0;
Parser::Parser(Scanner& sc, ostream& o): scan(sc), os(o){
		SymTable* table = new SymTable();
		st = new SymTableStack(); 
		FillMaps();
		scan.Next();
		isRecord = false;
		isAccess = false;
		SymTypeInteger* i = new SymTypeInteger("INTEGER");
		SymTypeReal* f = new SymTypeReal("REAL");
		table->insert(Sym("INTEGER", i));
		table->insert(Sym("REAL", f));
		st->push(table);
		curIdent = "";
		nextScan = true;
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
void Parser::ParseDecl(){
	TokenType tt = TokType();
	while (tt == ttType || tt == ttVar || tt == ttConst || tt == ttProcedure || tt == ttFunction){
		scan.Next();
		switch(tt){
		case ttType:
			ParseTypeConstBlock(ttType);
			break;
		case ttVar:
			ParseVarRecordBlock(ttVar);
			break;
		case ttConst:
			ParseTypeConstBlock(ttConst);
			break;
		case ttProcedure:
			ParseProcedure(true, false);
			break;
		case ttFunction:
			ParseProcedure(true, true);
		}
		tt = TokType();
	}
}

SymTable::iterator FindS(string s, SymTable* tbl){
	transform(s.begin(), s.end(), s.begin(), toupper);
	SymTable::iterator it = tbl->find(s);
	return it;
}

Statement* Parser::ParseStatement(){
	Statement* res = NULL;
	if (TokType() != ttIdentifier && !IsKeyWord(TokType()))
		throw Error("Unexpected lexem found", TokPos(), TokLine());
	if (IsKeyWord(TokType())){
		switch(TokType()){
			case ttIf:
				res = ParseIf();
				break;
			case ttWhile:
				res = ParseWhile();
				break;
			case ttRepeat:
				res = ParseRepeat();
				break;
			case ttFor:
				res = ParseFor();
				break;
			case ttBreak:
				res = new StmtBreak();
				break;
			case ttContinue:
				res = new StmtContinue();
				break;
			case ttBegin:
				res = ParseBlock(false);
				break;
			default:
				throw Error("Unexpected keyword found", TokPos(), TokLine());
		}
	} 
	else{
		SymTable::iterator it = FindS(TokVal(), table());
		if (it == table()->end())
			throw Error("Unknown identifier", TokPos(), TokLine());
		if (it->second->IsProc() || it->second->IsFunc())
			res = ParseProcedure();
		else if (it->second->IsVar())
				res = ParseAssignment();
		else
			throw Error("Invalid lexem", TokPos(), TokLine());
	}
	return res;
}

StmtBlock* Parser::ParseBlock(bool main){
	list<Statement*> stmts;
	StmtBlock* res = NULL;
	bool f = false;
	if (TokType() == ttBegin){
		f = true;
		scan.Next();
		while (TokType() != ttEnd){
			stmts.push_back(ParseStatement());
			scan.Next();
		}
	}
	else
		stmts.push_back(ParseStatement());
	res = new StmtBlock(stmts);
	scan.Next();
	SmthExpexted(TokVal(), TokPos(), TokLine(), main && f ? "." : ";");
	return res;
}

StmtWhile* Parser::ParseWhile(){
	NodeExpr* expr = ParseNext();
	Statement* body = NULL;
	if (TokType() != ttDo)
		throw Error("Do expected, but " + TokVal() + " found", TokPos(), TokLine());
	body = ParseBlock(false);
	return new StmtWhile(expr, body);
}
StmtRepeat* Parser::ParseRepeat(){
	list<Statement*> stmts;
	Statement* body = NULL;
	scan.Next();
	while (TokType() != ttUntil){
		stmts.push_back(ParseStatement());
		scan.Next();
	}
	body = new StmtBlock(stmts);
	NodeExpr* expr = ParseNext();
	if (TokType() != ttSemi)
		throw Error("; expected, but " + TokVal() + " found", TokPos(), TokLine());
	return new StmtRepeat(expr, body);
}
StmtProcedure* Parser::ParseProcedure(){
	return NULL;
}
StmtIf* Parser::ParseIf(){
	return NULL;
}

StmtFor* Parser::ParseFor(){
	return NULL;
}

string Parser::CheckCurTok(string blockName, SymTable* tbl){
	if (TokType() != ttIdentifier)
		throw Error("Invalid lexem in " + blockName + " block", TokPos(), TokLine());
	string s = TokVal();
	SymTable::iterator it = FindS(s, tbl);
	if (it != tbl->end())
		throw Error("Identifier already declared", TokPos(), TokLine());	
	CheckEof();
	return s;
}

SymTable* Parser::GetArgs(string name, list<string>** arg_names){
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
				string par = CheckCurTok("function", argsTable);
				args.push_back(par);
				(*arg_names)->push_back(par);
				++args_num;
				if (TokType() == ttPoint)
					CheckEof();
				else if (TokType() == ttColon){
					CheckEof();
					type = ParseType(false);
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

void Parser::CheckProcDecl(){
	if (TokVal() != ";")
		if (TokVal() == "EOF")
			throw Error("Unexpected end of file", TokPos(), TokLine());
		else
			SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
	CheckEof();
	SmthExpexted(TokVal(), TokPos(), TokLine(), "Forward");
	CheckEof();
	SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
}

SymProc* Parser::ParseProcedure(bool newProc, bool func){
	SymProc* res = NULL;
	string ident = TokVal();
	if (newProc)
		string ident = func ? CheckCurTok("function", table()) : CheckCurTok("procedure", table());
	list<string>* arg_names = new list<string>;
	SymTable* argTable = GetArgs(ident, &arg_names);
	if (func){
		SymType* type = NULL;
		if (TokType() != ttColon)
			throw Error("Function type expected", TokPos(), TokLine());
		CheckEof();
		type = ParseType(false);
		CheckEof();
		res = new SymFunc(ident, argTable, NULL, arg_names, type);
	}
	else
		res = new SymProc(ident, argTable, NULL, arg_names);
	CheckProcDecl();
	if (newProc)
		(*table())[ident] = res;
	scan.Next();
	res->Print(os, false);
	os << "\n";
	return res;
}

SymTable* Parser::ParseVarRecordBlock(TokenType tt){ 
	list<string> idents;
	SymTable* curTable = NULL;
	curTable = (tt == ttVar) ? table() : new SymTable();
	string b_name = (tt == ttVar) ? "var" : "record";
	while(true){
		string s = CheckCurTok(b_name, curTable);
		idents.push_back(s);
		if (TokType() == ttPoint)
			CheckEof();
		else {
			if (TokType() != ttColon)
				throw Error("Invalid lexem in " + b_name + " block", TokPos(), TokLine());
			CheckEof();
			SymType* type = ParseType(false);
			CheckEof();
			if (tt == ttVar && type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, curTable);
				if (it != curTable->end())
					throw Error("Identifier already declared", TokPos(), TokLine());
				SymVar* v = (tt == ttVar) ? (SymVar*)new SymVarGlobal(*it1, type) : (SymVar*)new SymVarLocal(*it1, type);
				(*curTable)[*it1] = v;
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
						return curTable;		
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
	return curTable;
}

SymVarConst* Parser::ParseConst(bool newConst){
	SymVarConst* res = NULL;
	SymType* type = NULL;
	string curId = curIdent;
	if (IsConstType(TokType())){
		if (IsIntType(TokType())){
			type = (SymType*)table()->find("INTEGER")->second;
			res = new SymVarConstInt(curId, type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)table()->find("REAL")->second;
			res = new SymVarConstReal(curId, type, ((RealLiteral*)scan.GetToken())->GetVal());
		}
	} 
	else {
		if (TokType() != ttIdentifier)
			throw Error("Invalid lexem in constant declaration", TokPos(), TokLine());
		string s = TokVal();
		SymTable::iterator it = FindS(s, table());
		if (it == table()->end())
			throw Error("Unknown constant name", TokPos(), TokLine());	
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

void Parser::ParseTypeConstBlock(TokenType tt){
	Symbol * res = NULL;
	string b_name = (tt == ttType) ? "type" : "const";
	while (true){
		string ident = CheckCurTok(b_name, table());
		if (TokType() != ttEq)
			throw Error("Invalid lexem in " + b_name + " declaration", TokPos(), TokLine());
		CheckEof();
		if (tt == ttType){
			res = ParseType(true);
			CheckEof();
			res->SetName(ident);
			res->Print(os, true);
		}
		else{
			curIdent = ident;
			res = ParseConst(true);
			CheckEof();
			curIdent = "";
			res->Print(os, true);
		}
		os << ";\n";
		(*table())[ident] = res;
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
		scan.Next();
		if (TokType() == ttType && tt == ttType || TokType() == ttConst && tt == ttConst)
				scan.Next();
		else
			if (AnothBlock(TokVal()))
				return;	
	}
}

SymType* Parser::ParseType(bool newType){
	SymType* res = NULL;
	TokenType tt = TokType();
	string s = TokVal();
	bool isPntr =  false;
	if (TokType() == ttCaret){
		CheckEof();
		isPntr = true;
		s = TokVal();
	}
	SymTable::iterator it = FindS(s, table());
	if (it != table()->end()){
		if (isPntr)
			res = new SymTypePointer(curIdent, (SymType*)(it->second));	
		else{
			if (newType){
				if (!(it->second)->IsType())
					throw Error("Invalid lexem in type declaration", TokPos(), TokLine());
				res = new SymTypeAlias(curIdent, (SymType*)(it->second));	
			} 
			else
				res = (SymType*)(it->second);
		}
	} 
	else {
		if (isPntr)
			throw Error("Invalid pointer declaration", TokPos(), TokLine());
		switch(tt){
		case ttRecord:
			CheckEof();
			res = new SymTypeRecord(toString(id++), ParseVarRecordBlock(ttRecord));
			break;
		case ttArray:
			CheckEof();
			res = ParseArray();
			break;
		default:
			if (it == table()->end() && !newType)
				throw Error("Unknown type", TokPos(), TokLine());
		}
	}
	return res;
}

SymTypeArray* Parser::ParseArray(){
	SymTypeArray* res = NULL;
	if (TokType() != ttLeftBracket)
		throw Error("Invalid array declaration", TokPos(), TokLine());
	list<SymVarConst*> left, right;
	CheckEof();
	while (TokType() != ttRightBracket){
		SymVarConst* l = ParseConst(false);
		CheckEof();
		if (TokType() != ttDblDot)
			throw Error("Invalid array declaration", TokPos(), TokLine());
		CheckEof();
		SymVarConst* r = ParseConst(false);
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
	SymType* type = ParseType(false);
	list<SymVarConst*>::reverse_iterator itl = left.rbegin();
	list<SymVarConst*>::reverse_iterator itr = right.rbegin();
	for (; itl != left.rend() && itr != right.rend(); ++itl, ++itr)
	{
		res = new SymTypeArray(toString(id++), type, (SymVarConst*)*itl, (SymVarConst*)*itr);
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

StmtAssign* Parser::ParseAssignment(){
	NodeExpr* expr = ParseSimple(5);
	BinaryOp* res = new BinaryOp(expr->GetSymbol(), expr->GetPos(), expr->GetLine(), expr, NULL);
	if (expr->GetValue() == ":="){
		BinaryOp* res = (BinaryOp*)expr;
		NodeExpr* lExpr = expr;
		while(lExpr->IsBinaryOp())
			lExpr = ((BinaryOp*)lExpr)->GetLeft();
		if (!res->GetLeft()->LValue())
			throw Error ("Left part of assignment must be lvalue", lExpr->GetPos(), lExpr->GetLine());
		SymType* t1 = res->GetLeft()->GetType();
		SymType* t2 = res->GetRight()->GetType();
		if (!(t1 && t2 && (EqTypes(t1, t2) || IToR(t1, t2))))
			throw Error("Incompatible types in assignment", lExpr->GetPos(), lExpr->GetLine());
	}
	return new StmtAssign(res->GetLeft(), res->GetRight());
	//os << "\n";
	//expr->Print(os, 0);
}

NodeExpr* Parser::ParseSimple(int prior){
	if (prior == 1)
		return ParseFactor();
	SymType* type = NULL;
	int pos = TokPos();
	int line = TokLine();
	NodeExpr* res = ParseSimple(prior - 1);
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseSimple(prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		if (expr1->IsInt() && (res->GetType()->IsScalar() && !res->IsInt())){
			expr1 = new UnaryOp(new Symbol("ItoR"), pos, line, expr1);
			expr1->SetType((SymType*)(*table())["REAL"]);
		}
		else if (prior != 5 && (res->IsInt() && (expr1->GetType()->IsScalar() && !expr1->IsInt()))){
			res = new UnaryOp(new Symbol("ItoR"), pos, line, res);
			res->SetType((SymType*)(*table())["REAL"]);
		}
		res = new BinaryOp(new Symbol(str), pos, line, res, expr1);
		if (prior == 5)
			break;
		type = (res->IsInt()) ? (SymType*)(*table())["INTEGER"] : (SymType*)(*table())["REAL"];
		res->SetType(type);
		if (!res->GetType()->IsScalar() || !expr1->GetType()->IsScalar())
			throw Error("Invalid types in binary operation", pos, line);
		pos = TokPos();
		line = TokLine();
	}
	return res;
}

NodeExpr* Parser::ParseNext(){
	scan.Next();
	NodeExpr* expr1 = ParseSimple(4);
	if (!expr1)
		throw Error("Lexem expected, EOF found", TokPos(), TokLine());
	return expr1;
}

Const* Parser::ParseConst(){
	Const* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	SymTable::iterator it = FindS(TokVal(), table());
	scan.Next();
	CheckAccess(TokVal(), TokPos(), TokLine());
	nextScan = false;
	res = new Const((SymVarConst*)(it->second), pos, line);
	res->SetType(((SymVarConst*)it->second)->GetType());
	return res;
}

NodeExpr* Parser::ParseVariable(NodeExpr* res){
	string s;
	SymVar* var = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (!res) {
		SymTable::iterator it = FindS(TokVal(), table());
		var = (SymVar*)(it->second);
		s = TokVal();
		Symbol* symb = new Symbol(s);
		res = new NodeExpr(symb, pos, line);
	}
	else{
		s = res->GetValue();
		var = new SymVar(res->GetSymbol()->GetName(), res->GetType());
	}
	SymType* type = var->GetType();
	if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
			res = new Variable(var, pos, line);
	else
		while (true){
			if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
				break;
			if (var->IsFunc()){
				res = ParseFunc(res, &var, pos, line);
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
					res = (var->GetType()->IsArray()) ? (NodeExpr*)ParseArr(res, &var, pos, line) : (NodeExpr*)ParseRecord(res, &var, pos, line);
			}
			pos = TokPos();
			line = TokLine();
		}
	type = var->GetType();
	res->SetType(type);
	return res;
}

ArrayAccess* Parser::ParseArr(NodeExpr* res, SymVar** var, int pos, int line){
	SymType* type = (*var)->GetType();
	if (TokType() == ttLeftBracket){
		scan.Next();
		while (true){
			NodeExpr* expr = ParseSimple(4);
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

FunctionCall* Parser::ParseFunc(NodeExpr* res, SymVar** var, int pos, int line){
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
			NodeExpr* expr = ParseSimple(4);
			SymVar* curVar = ((SymVar*)(it1->second));
			if (curVar->IsParamByRef()){
				if (!(EqTypes(expr->GetType(), curVar->GetType())))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
				if (!expr->LValue())
					throw Error("Variable identifier expected", expr->GetPos(), expr->GetLine());
			}
			else{
				if (!EqTypes(expr->GetType(), curVar->GetType()) && !IToR(curVar->GetType(), expr->GetType()))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
				if (IToR(curVar->GetType(), expr->GetType())){
					expr = new UnaryOp(new Symbol("ItoR"), TokPos(), TokLine(), expr);
					expr->SetType((SymType*)(*table())["REAL"]);
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

RecordAccess* Parser::ParseRecord(NodeExpr* res, SymVar** var, int pos, int line){
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

NodeExpr* Parser::ParseFactor(){
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
		NodeExpr* expr1 = ParseFactor();
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		Symbol* symb = new Symbol(str);
		res = new UnaryOp(symb, pos, line, expr1);
		if (!expr1->GetType()->IsScalar())
			throw Error("Invalid unary operation", pos, line);
		res->SetType(expr1->GetType()); 
	}
	else if (TokType() == ttIdentifier){
		SymTable::iterator it = FindS(TokVal(), table());
		if (it == table()->end())
			throw Error("Unknown identifier", TokPos(), TokLine());
		if (it->second->IsType())
			throw Error("Invalid identifier", TokPos(), TokLine());
		if (it->second->IsConst())
			res = ParseConst();
		else if (it->second->IsVar() || it->second->IsFunc())
			res = ParseVariable(NULL);
		if (nextScan)
			scan.Next();
		nextScan = true;
	}
	else if (IsLiteral(TokType())){
		SymVarConst* symb = NULL;
		SymType* type = NULL;
		if (IsIntType(TokType())){
			type = (SymType*)(*table())["INTEGER"];
			symb = new SymVarConstInt(TokVal(),type, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else {
			type = (SymType*)(*table())["REAL"];
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
		res = ParseSimple(4);
		if (TokType() != ttRightParentheses)
			throw Error("Unclosed bracket", pos, line);
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}