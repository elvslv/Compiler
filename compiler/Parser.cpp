#include "Parser.h"
static int id = 0;
typedef list<string> ls;

bool IsIntOperator(string val) { return (val == "+" || val == "-" || val == "*" || UpCase(val) == "MOD" 
										|| UpCase(val) == "DIV"); }

int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

template <typename T>
std::string toString(T val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

void Parser::CheckEof(){
	scan.Next();
	if (TokVal() == "EOF")
		throw Error("Unexpected end of file", TokPos(), TokLine());
}

void Parser::ParseDecl(){
	string s = TokVal();
	while (s == "TYPE" || s == "VAR" || s == "CONST" || s == "PROCEDURE" || s == "FUNCTION"){
		scan.Next();
		if (s == "TYPE")
			ParseTypeConstBlock("type");
		else if (s == "VAR")
			ParseVarRecordBlock("var");
		else if (s == "CONST")
			ParseTypeConstBlock("const");
		else if (s == "FUNCTION")
			ParseProcedure(true, true);
		else if (s == "PROCEDURE")
			ParseProcedure(true, false);
		s = TokVal();
	}
	if (TokVal() == "BEGIN"){
		scan.Next();
		ParseAssignment();
	}
	//if (TokVal() != "END")
}

bool AnothBlock(string s){
	if (s == "TYPE" || s == "CONST" || s == "PROCEDURE" || s == "FUNCTION" || s == "BEGIN" || s == "EOF" || s == "VAR")
		return true;
}

SymTable::iterator FindS(string s, SymTable* tbl){
	transform(s.begin(), s.end(), s.begin(), toupper);
	SymTable::iterator it = tbl->find(s);
	return it;
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

SymTable* Parser::GetArgs(string name, ls** arg_names){
	list<string> args;
	SymTable* argsTable = new SymTable();
	SymType* type = NULL;
	int args_num = 0;
	int index = 0;
	if (TokVal() == "("){
		while (TokVal() != ")"){
			CheckEof();
			string s = TokVal();
			if (s == "VAR")
				CheckEof();
			while (TokVal() != ";" && TokVal() != ")"){
				string par = CheckCurTok("function", argsTable);
				args.push_back(par);
				(*arg_names)->push_back(par);
				++args_num;
				if (TokVal() == ",")
					CheckEof();
				else if (TokVal() == ":"){
					CheckEof();
					type = ParseType(false);
					if (type->GetName()[0] >= '0' && type->GetName()[0] <= '9')
						throw Error("Invalid type", TokPos(), TokLine());
					CheckEof();
					break;
				}
				else if (TokVal() != ")" && TokVal() != ";")
					throw Error("Invalid lexem in function declaration", TokPos(), TokLine());
			}
			for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
				SymTable::iterator it1 = FindS(*it, argsTable);
				if (it1 != argsTable->end() || *it == name)
					throw Error("Identifier already declared", TokPos(), TokLine());
				if (s == "VAR")
					(*argsTable)[*it] = new SymVarParamByRef(*it, type);
				else
					(*argsTable)[*it] = new SymVarParamByValue(*it, type);
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
			throw Error("; expected", TokPos(), TokLine());
	CheckEof();
	if (TokVal() != "FORWARD")
		throw Error("Forward expected", TokPos(), TokLine());
	CheckEof();
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
}

SymProc* Parser::ParseProcedure(bool newProc, bool func){
	SymProc* res = NULL;
	string ident = TokVal();
	if (newProc)
		if (func)
			string ident = CheckCurTok("function", table);
		else 
			string ident = CheckCurTok("procedure", table);
	ls* arg_names = new ls;
	SymTable* argTable = GetArgs(ident, &arg_names);
	if (func){
		SymType* type = NULL;
		if (TokVal() != ":")
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
		(*table)[ident] = res;
	scan.Next();
	res->Print(os, false);
	os << "\n";
	return res;
}

SymTable* Parser::ParseVarRecordBlock(string b_name){ 
	list<string> idents;
	SymTable* curTable = NULL;
	if (b_name == "var")
		curTable =  table;
	else 
		curTable = new SymTable();
	while(true){
		string s = CheckCurTok(b_name, curTable);
		idents.push_back(s);
		if (TokVal() == ",")
			CheckEof();
		else if (TokVal() == ":"){
			CheckEof();
			SymType* type = ParseType(false);
			CheckEof();
			if (b_name == "var" && type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, curTable);
				if (it != curTable->end())
					throw Error("Identifier already declared", TokPos(), TokLine());
				SymVar* v = NULL;
				if (b_name == "var")
					v = new SymVarGlobal(*it1, type);
				else 
					v = new SymVarLocal(*it1, type);
				(*curTable)[*it1] = v;
				if (b_name == "var"){
					v->Print(os, type->GetName()[0] >= '0' && type->GetName()[0] <= '9');
					os << "\n";
				}
			}
			idents.clear();
			if (TokVal() != ";")
				throw Error("; Expected", TokPos(), TokLine());
			if (b_name == "var"){
				scan.Next();
				if (TokVal() == "VAR")
					scan.Next();
				else
					if (AnothBlock(TokVal()))
						return curTable;		
			}
			else{
				CheckEof();
				if (TokVal() == "END")
					break;
			}
		}
		else 
			throw Error("Invalid lexem in " + b_name + " block", TokPos(), TokLine());
	}
	if (b_name == "var")
		scan.Next();
	return curTable;
}

SymVarConst* Parser::ParseConst(bool newConst){
	SymVarConst* res = NULL;
	SymType* type = NULL;
	string curId = curIdent;
	string val = TokVal();
	if (IsConstType(TokType())){
		if (IsIntType(TokType()))
			type = (SymType*)table->find("INTEGER")->second;
		else if (TokType() == ttRealLit)
			type = (SymType*)table->find("REAL")->second;
	} 
	else {
		if (TokType() != ttIdentifier)
			throw Error("Invalid lexem in constant declaration", TokPos(), TokLine());
		string s = TokVal();
		SymTable::iterator it = FindS(s, table);
		if (it == table->end())
			throw Error("Unknown constant name", TokPos(), TokLine());	
		if (!it->second->IsConst())
			throw Error("Invalid constant name", TokPos(), TokLine());
		val = ((SymVarConst*)(it->second))->GetValue();
		type = ((SymVarConst*)(it->second))->GetType();
		if (!newConst)
			curId = s;
	}
	res = new SymVarConst(curId, val, type);
	return res;
}

void Parser::ParseTypeConstBlock(string b_name){
	Symbol * res = NULL;
	while (true){
		string ident = CheckCurTok(b_name, table);
		if (TokVal() != "=")
			throw Error("Invalid lexem in " + b_name + " declaration", TokPos(), TokLine());
		CheckEof();
		if (b_name == "type"){
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
		(*table)[ident] = res;
		if (TokVal() != ";")
			throw Error("; Expected", TokPos(), TokLine());
		scan.Next();
		if (TokVal() == "TYPE" && b_name == "type" || TokVal() == "CONST" && b_name == "const")
				scan.Next();
		else
			if (AnothBlock(TokVal()))
				return;	
	}
}

SymType* Parser::ParseType(bool newType){
	SymType* res = NULL;
	string s = TokVal();
	bool isPntr =  false;
	if (s == "^"){
		CheckEof();
		isPntr = true;
		s = TokVal();
	}
	SymTable::iterator it = FindS(s, table);
	if (it != table->end()){
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
		if (s == "RECORD"){
		SymTable* fields ;
		CheckEof();
		fields = ParseVarRecordBlock("record");
		res = new SymTypeRecord(toString(id++), fields);
		}
		else if (s == "ARRAY"){
			CheckEof();
			res = ParseArray();
		} 
		else if (it == table->end() && !newType)
			throw Error("Unknown type", TokPos(), TokLine());
	}
	return res;
}

SymTypeArray* Parser::ParseArray(){
	SymTypeArray* res = NULL;
	if (TokVal() != "[")
		throw Error("Invalid array declaration", TokPos(), TokLine());
	list<SymVarConst*> left, right;
	CheckEof();
	while (TokVal() != "]"){
		SymVarConst* l = ParseConst(false);
		CheckEof();
		if (TokVal() != "..")
			throw Error("Invalid array declaration", TokPos(), TokLine());
		CheckEof();
		SymVarConst* r = ParseConst(false);
		CheckEof();
		if (!(l->GetType()->IsInt() && r->GetType()->IsInt()))
			throw Error("Invalid array bounds", TokPos(), TokLine());
		if (atoi(l->GetValue().c_str()) > atoi(r->GetValue().c_str()))
			throw Error("Upper bound less than lower bound", TokPos(), TokLine());
		left.push_back(l);
		right.push_back(r);
		if (TokVal() == ",")
			CheckEof();
	}
	scan.Next();
	if (TokVal() != "OF")
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
		j = PaintBranch(i, j, 0, h, true);
		s += h;
	}
	return j - tmp + 2;
}

int UnaryOp::FillTree(int i, int j){
	j = FillTreeOp(i, j, GetValue());
	int h = child->FillTree(i + 4, j);
	return h + 2;
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

void Parser::ParseAssignment(){
	NodeExpr* expr = ParseSimple(5);
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
	os << "\n";
	expr->Print(os, 0);
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
		Symbol* symb = new Symbol(str);
		res = new BinaryOp(symb, pos, line, res, expr1);
		if (prior == 5)
			break;
		if (res->IsInt())
			type = (SymType*)(*table)["INTEGER"];
		else
			type = (SymType*)(*table)["REAL"];
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

void CheckAccess(string s, int i, int j){
	if (s == "[")
		throw Error("Invalid array name", i, j);
	if (s == "(")
		throw Error("Invalid function name", i, j);
	if (s== ".")
		throw Error("Invalid record name", i, j);
}

Const* Parser::ParseConst(){
	Const* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	SymTable::iterator it = FindS(TokVal(), table);
	scan.Next();
	CheckAccess(TokVal(), TokPos(), TokLine());
	nextScan = false;
	res = new Const((SymVarConst*)(it->second), pos, line);
	res->SetType(((SymVarConst*)it->second)->GetType());
	return res;
}

void CheckIsInt(NodeExpr* expr, int i, int j){
	if (!expr->IsInt())
		throw Error("Integer index expected", i, j);
}

NodeExpr* Parser::ParseVariable(NodeExpr* res){
	string s;
	SymVar* var = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (!res) {
		SymTable::iterator it = FindS(TokVal(), table);
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
			if (var->IsFunc()){
				res = ParseFunc(res, &var, pos, line);
				nextScan = false;
			}
			else if (var->GetType()->IsArray() || var->GetType()->IsRecord()){
				if (nextScan)
					scan.Next();
				if (var->GetType()->IsArray() && TokVal() != "[" || var->GetType()->IsRecord() && TokVal() != "."){
					CheckAccess(TokVal(), TokPos(), TokLine());
					if (res->GetValue() == s)
						res = new Variable(var, pos, line);
					nextScan = false;
					break;
				}
				else{
					if (var->GetType()->IsArray())
						res = ParseArr(res, &var, pos, line);
					else
						res = ParseRecord(res, &var, pos, line);
				}
			}
			else
				break;
			pos = TokPos();
			line = TokLine();
		}
	type = var->GetType();
	res->SetType(type);
	return res;
}

ArrayAccess* Parser::ParseArr(NodeExpr* res, SymVar** var, int pos, int line){
	SymType* type = (*var)->GetType();
	if (TokVal() == "["){
		scan.Next();
		while (true){
			NodeExpr* expr = ParseSimple(4);
			CheckIsInt(expr, expr->GetPos(), expr->GetLine());
			Symbol* symb = new Symbol("[]");
			res = new ArrayAccess(symb, pos, line, res, expr);
			if (expr->IsConst()){
				string val = ((SymVarConst*)expr->GetSymbol())->GetValue();
				string top = ((SymVarConst*)((SymTypeArray*)type)->GetTop())->GetValue();
				string bottom = ((SymVarConst*)((SymTypeArray*)type)->GetBottom())->GetValue();
				if (atof(val.c_str()) > atof(top.c_str()) || atof(val.c_str()) < atof(bottom.c_str()))
					throw Error("Range check error", expr->GetPos(), expr->GetLine());
			}
			type = ((SymTypeArray*)type)->GetElemType();
			if(TokVal() == "]"){
				if (!nextScan)
					scan.Next();
				break;
			}
			if (TokVal() == ","){
				if (!type->IsArray())
					throw Error("Invalid num of indexes", TokPos(), TokLine());
				scan.Next();
				if (TokVal() == "]")
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
	if (TokVal() == "("){
		scan.Next();
		while (TokVal() != ")"){
			it1 = FindS(*it, arg_table);
			NodeExpr* expr = ParseSimple(4);
			SymVar* curVar = ((SymVar*)(it1->second));
			if (curVar->IsParamByRef()){
				if (!(EqTypes(expr->GetType(), curVar->GetType())))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
				if (!expr->LValue())
					throw Error("Variable identifier expected", expr->GetPos(), expr->GetLine());
			}
			else
				if (!EqTypes(expr->GetType(), curVar->GetType()) && !IToR(curVar->GetType(), expr->GetType()))
					throw Error("Incompatible types", expr->GetPos(), expr->GetLine());
			args.push_back(expr);
			++it;
			if (TokVal() == ","){
				if (it == arg_names->end())
					throw Error("Invalid parameters num", TokPos(), TokLine());
				scan.Next();
				if (TokVal() == ")")
					throw Error("Invalid function call", TokPos(), TokLine());
			}
		}
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
	if (TokVal() == "."){
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
			scan.Next();
			if (TokVal() == "."){
				type = type->GetType();
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
		SymTable::iterator it = FindS(TokVal(), table);
		if (it == table->end())
			throw Error("Unknown identifier", TokPos(), TokLine());
		if (it->second->IsType() || it->second->IsProc())
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
		if (TokType() == ttIntLit)
			type = (SymType*)(*table)["INTEGER"];
		else
			type = (SymType*)(*table)["REAL"];
		symb = new SymVarConst(TokVal(), TokVal(), type);
		res = new Const(symb, pos, line);
		res->SetType(type);
		scan.Next();
	}
	else if (TokVal() == "("){
		scan.Next();
		if (TokVal() == ")")
			throw Error("Empty bracket sequence", pos, line);
		res = ParseSimple(4);
		if (TokVal() != ")")
			throw Error("Unclosed bracket", pos, line);
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}