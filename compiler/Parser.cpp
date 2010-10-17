#include "Parser.h"
static int id = 0;
map<string, int> priority;
const int arrSize = 500;
char arr[arrSize][arrSize];
static int maxLength = 0;

typedef list<string> ls;

int PaintBranch_(int i, int j, int k, int h, bool f){
	for (; k < h; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	if (f)
		for (unsigned int k = 0; k < 3; ++k)
			arr[j][i + k + 1] = '_';
	return j;
}

int FillTreeIdentConst_(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int FillTreeOp_(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	j = PaintBranch_(i, j, 0, 2, true);
	return j;
}

int FillTreeBinOp_(int i, int j, string Value, NodeExpr* left, NodeExpr* right){
	j = FillTreeOp_(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch_(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

bool IsIntOperator(string val){
	return (val == "+" || val == "-" || val == "*" || UpCase(val) == "MOD" || UpCase(val) == "DIV");
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
			ParseTypeBlock();
		else if (s == "VAR")
			ParseVarBlock();
		else if (s == "CONST")
			ParseConstBlock();
		else if (s == "FUNCTION")
			ParseFunction(true);
		else if (s == "PROCEDURE")
			ParseProcedure(true);
		s = TokVal();
	}
	if (TokVal() == "BEGIN"){
		scan.Next();
		ParseAssignment();
	}
	//if (TokVal() != "END")
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

SymProc* Parser::ParseProcedure(bool newProc){
	string ident = TokVal();
	if (newProc)
		string ident = CheckCurTok("procedure", table);
	ls* arg_names = new ls;
	SymTable* argTable = GetArgs(ident, &arg_names);
	SymProc* res = new SymProc(ident, argTable, NULL, arg_names);
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
	if (newProc)
		(*table)[ident] = res;
	scan.Next();
	res->Print(os, false);
	os << "\n";
	return res;
}

SymFunc* Parser::ParseFunction(bool newFunc){
	string ident = TokVal();
	if (newFunc)
		string ident = CheckCurTok("function", table);
	SymType* type = NULL;
	ls* arg_names = new ls;
	SymTable* argTable = GetArgs(ident, &arg_names);
	if (TokVal() != ":")
		throw Error("Function type expected", TokPos(), TokLine());
	CheckEof();
	type = ParseType(false);
	CheckEof();
	SymFunc* res = new SymFunc(ident, argTable, NULL, arg_names, type);
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	CheckEof();
	if (TokVal() != "FORWARD")
		throw Error("Forward expected", TokPos(), TokLine());
	CheckEof();
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	if (newFunc)
		(*table)[ident] = res;
	scan.Next();
	res->Print(os, false);
	os << "\n";
	return res;
}

void Parser::ParseVarBlock(){
	list<string> idents;
	while(true){
		string s = CheckCurTok("var", table);
		idents.push_back(s);
		if (TokVal() == ",")
			CheckEof();
		else if (TokVal() == ":"){
			CheckEof();
			SymType* type = ParseType(false);
			CheckEof();
			if (type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, table);
				if (it != table->end())
					throw Error("Identifier already declared", TokPos(), TokLine());
				SymVarGlobal* v = new SymVarGlobal(*it1, type);
				(*table)[*it1] = v;
				v->Print(os, type->GetName()[0] >= '0' && type->GetName()[0] <= '9');
				os << "\n";
			}
			idents.clear();
			if (TokVal() != ";")
				throw Error("; Expected", TokPos(), TokLine());
			scan.Next();
			if (TokVal() == "TYPE" || TokVal() == "CONST" || TokVal() == "PROCEDURE" || 
				TokVal() == "FUNCTION" || TokVal() == "BEGIN" || TokVal() == "EOF")
				return;
			if (TokVal() == "VAR")
				scan.Next();
		}
	}
	scan.Next();
}

SymTable* Parser::ParseRecordBlock(){
	SymTable* recTable = new SymTable();
	list<string> idents;
	while(TokVal() != "END"){
		string s = CheckCurTok("record", recTable);
		idents.push_back(s);
		if (TokVal() == ",")
			CheckEof();
		else if (TokVal() == ":"){
			CheckEof();
			SymType* type = ParseType(false);
			CheckEof();
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymTable::iterator it = FindS(*it1, recTable);
				if (it != recTable->end())
					throw Error("Identifier already declared", TokPos(), TokLine());
				SymVarLocal* v = new SymVarLocal(*it1, type);
				(*recTable)[*it1] = v;
			}
			idents.clear();
			if (TokVal() != ";")
				throw Error("; Expected", TokPos(), TokLine());
			CheckEof();
		} 
		else 
			throw Error("Invalid lexem in record block", TokPos(), TokLine());
	}
	return recTable;
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
			type = (SymType*)table->find("FLOAT")->second;
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

void Parser::ParseConstBlock(){
	SymVarConst * res = NULL;
	while (TokVal() != ";"){
		string ident = CheckCurTok("constants", table);
		if (TokVal() != "=")
			throw Error("Invalid lexem in const declaration", TokPos(), TokLine());
		CheckEof();
		curIdent = ident;
		res = ParseConst(true);
		(*table)[ident] = res;
		CheckEof();
		curIdent = "";
		if (TokVal() != ";")
			throw Error("; Expected", TokPos(), TokLine());
		res->Print(os, true);
		os << "; \n";
		scan.Next();
		if (TokVal() == "TYPE" || TokVal() == "VAR" || TokVal() == "PROCEDURE" || 
			TokVal() == "FUNCTION" || TokVal() == "BEGIN" || TokVal() == "EOF")
			return;
		if (TokVal() == "CONST")
			CheckEof();
	}
	scan.Next();
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
		fields = ParseRecordBlock();
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

void Parser::ParseTypeBlock(){
	while(true){
		string ident = CheckCurTok("type", table);
		if (TokVal() != "=")
			throw Error("Invalid lexem in type declaration", TokPos(), TokLine());
		CheckEof();
		SymType* type = ParseType(true);
		CheckEof();
		type->SetName(ident);
		type->Print(os, true);
		os << ";\n";
		(*table)[ident] = type;
		if (TokVal() != ";")
			throw Error("; Expected", TokPos(), TokLine());
		scan.Next();
		if (TokVal() == "TYPE" || TokVal() == "VAR" || TokVal() == "PROCEDURE" || 
			TokVal() == "FUNCTION" || TokVal() == "BEGIN" || TokVal() == "EOF")
			return;
		if (TokVal() == "CONST")
			scan.Next();
	}
	scan.Next();
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

int Variable::FillTree(int i, int j){
	return FillTreeIdentConst_(i, j, GetValue());
}

int Const::FillTree(int i, int j){
	return FillTreeIdentConst_(i, j, GetValue());
}

int BinaryOp::FillTree(int i, int j){
	return FillTreeBinOp_(i, j, GetValue(), left, right);
}

int ArrayAccess::FillTree(int i, int j){
   return FillTreeBinOp_(i, j, GetValue(), left, right);
}

int RecordAccess::FillTree(int i, int j){
	return FillTreeBinOp_(i, j, ".", left, right);
}

int FunctionCall::FillTree(int i, int j){
	unsigned int k = 0;
	int tmp = j;
	j = FillTreeOp_(i, j, "()");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst_(i + 4, j, GetValue()) - 2, false);
	int s = 0;
	int h = 0;
	for (list<NodeExpr*>::iterator it = args.begin() ;it != args.end(); ++it){
		h = (*it)->FillTree(i + 4, j + 2);
		j = PaintBranch_(i, j, 0, h, true);
		s += h;
	}
	return j - tmp + 2;
}

int UnaryOp::FillTree(int i, int j){
	j = FillTreeOp_(i, j, GetValue());
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

void Parser::ParseAssignment(){
	NodeExpr* expr = ParseSimple(5);
	if (expr->GetValue() == ":="){
		if (!((BinaryOp*)expr)->GetLeft()->LValue())
			throw Error ("Left part of assignment must be lvalue", TokPos(), TokLine());
		SymType* t1 = ((BinaryOp*)expr)->GetLeft()->GetType();
		SymType* t2 = ((BinaryOp*)expr)->GetRight()->GetType();
		if (!(t1 && t2 && EqTypes(t1, t2)))
			throw Error("Incompatible types in assignment", TokPos(), TokLine());
	}
	expr->Print(os, 0);
}

void NodeExpr::Print(ostream& os, int n){
	int h = FillTree(0, 0) - 1;
	for (int i = 0; i < h; ++i){
		for (int j = 0; j < maxLength; ++j)
			os << arr[i][j];
		os << "\n";
	}
}

NodeExpr* Parser::ParseSimple(int prior){
	if (prior == 1)
		return ParseFactor();
	SymType* type = NULL;
	NodeExpr* res = ParseSimple(prior - 1);
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseSimple(prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		Symbol* symb = new Symbol(str);
		res = new BinaryOp(symb, res, expr1);
		if (prior == 5)
			break;
		if (res->IsInt())
			type = (SymType*)(*table)["INTEGER"];
		else
			type = (SymType*)(*table)["FLOAT"];
		res->SetType(type);
		if (!res->GetType()->IsScalar() || !expr1->GetType()->IsScalar())
			throw Error("Invalid types in binary operation", TokPos(), TokLine());
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
	scan.Next();
	CheckAccess(TokVal(), TokPos(), TokLine());
	SymTable::iterator it = FindS(TokVal(), table);
	res = new Const((SymVarConst*)(it->second));
	res->SetType(((SymVarConst*)it->second)->GetType());
	return res;
}

void CheckIsInt(NodeExpr* expr, int i, int j){
	if (!expr->IsInt())
		throw Error("Integer index expected", i, j);
}

NodeExpr* Parser::ParseVariable(NodeExpr* res){
	string s;
	bool f = false;
	SymVar* var = NULL;
	if (!res) {
		SymTable::iterator it = FindS(TokVal(), table);
		var = (SymVar*)(it->second);
		s = TokVal();
		Symbol* symb = new Symbol(s);
		res = new NodeExpr(symb);
	}
	else{
		s = res->GetValue();
		var = new SymVar(res->GetSymbol()->GetName(), res->GetType());
		//res = new NodeExpr(var);
	}
	SymType* type = var->GetType();
	if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
		res = new Variable(var);
	else
		while (true){
			if (var->IsFunc()){
				res = ParseFunc(res, &var);
				f = true;
			}
			else if (var->GetType()->IsArray()){
				if (!f)
					scan.Next();
				if (TokVal() != "["){
					CheckAccess(TokVal(), TokPos(), TokLine());
					if (res->GetValue() == s)
						res = new Variable(var);
					nextScan = false;
					break;
				}
				else{
					res = ParseArr(res, &var);
					f = true;
				}
			}
			else if (var->GetType()->IsRecord()){
				if (!f)
					scan.Next();
				if (TokVal() != "."){
					CheckAccess(TokVal(), TokPos(), TokLine());
					if (res->GetValue() == s)
						res = new Variable(var);
					nextScan = false;
					break;
				}
				else{
					res = ParseRecord(res, &var);
					f = true;
				}
			} 
			else
				break;
		}
	type = var->GetType();
	res->SetType(type);
	return res;
}

ArrayAccess* Parser::ParseArr(NodeExpr* res, SymVar** var){
	SymType* type = (*var)->GetType();
	//string s = TokVal();
	//scan.Next();
	if (TokVal() == "["){
		scan.Next();
		while (TokVal() != "]"){
			NodeExpr* expr = ParseSimple(4);
			CheckIsInt(expr, TokPos(), TokLine());
			Symbol* symb = new Symbol("[]");
			res = new ArrayAccess(symb, res, expr);
			/*if (expr->IsConst()){
				if (expr->CountValue() > ((SymTypeArray*)type)->)
			}*/
			type = ((SymTypeArray*)type)->GetElemType();
			if (TokVal() == ","){
				if (!type->IsArray())
					throw Error("Invalid num of indexes", TokPos(), TokLine());
				scan.Next();
				if (TokVal() == "]")
					throw Error("Invalid array access", TokPos(), TokLine());
			}
		}
		
	}
	*var = new SymVar(toString(id++), type);
	res->SetType(type);
	return (ArrayAccess*)res;
}

FunctionCall* Parser::ParseFunc(NodeExpr* res, SymVar** var){
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
				if (!EqTypes(expr->GetType(), curVar->GetType()))
					throw Error("Incompatible types", TokPos(), TokLine());
				if (!expr->LValue())
					throw Error("Variable identifier expected", TokPos(), TokLine());
			}
			else
				if (!EqTypes(expr->GetType(), curVar->GetType()) && !(curVar->GetType()->IsFloat() && expr->GetType()->IsInt()))
					throw Error("Incompatible types", TokPos(), TokLine());
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
	if (it != arg_names->end())
		throw Error("Invalid prameters num", TokPos(), TokLine());
	Symbol* symb = new Symbol(s);
	res = new FunctionCall(symb, args);
	*var = new SymVar(toString(id++), type);
	res->SetType(type);
	return (FunctionCall*)res;
}

RecordAccess* Parser::ParseRecord(NodeExpr* res, SymVar** var){
	string s = TokVal();
	scan.Next();
	SymType* type = (*var)->GetType();
	NodeExpr* expr = NULL;
	Symbol* symb = NULL;
	if (TokVal() == "."){
		//res = new NodeExpr(s);
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
			expr = new NodeExpr(symb);
			res = new RecordAccess(*var, res, expr);
			scan.Next();
			if (TokVal() == "."){
				type = type->GetType();
				if (!type->IsRecord())
					throw Error("Invalid record access", TokPos(), TokLine());
				scan.Next();
			}
			else 
				break;
		}
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
		res = new UnaryOp(symb, expr1);
		if (!expr1->GetType()->IsScalar())
			throw Error("Invalid unary operation", TokPos(), TokLine());
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
			type = (SymType*)(*table)["FLOAT"];
		symb = new SymVarConst(TokVal(), TokVal(), type);
		res = new Const(symb);
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
		//scan.Next();
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}
void Parser::FillMaps(){
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	priority[":="] = 5;
	memset(arr, ' ', arrSize * arrSize);
}

int Parser::FindOpPrior(string str){
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}