#include "Parser.h"
static int id = 0;
map<string, int> priority;
const int arrSize = 500;
char arr[arrSize][arrSize];
static int maxLength = 0;

void Parser::ParseDecl(){
	while (TokVal() == "TYPE" || TokVal() == "VAR" || TokVal() == "CONST" || TokVal() == "PROCEDURE" || 
		TokVal() == "FUNCTION"){
		string s = TokVal();
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
	}
	/*if (TokVal() == "BEGIN"){
		ParseAssignment();
	}*/
}

string Parser::CheckCurTok(string blockName, SymTable* tbl){
	if (TokType() != ttIdentifier)
		throw Error("Invalid lexem in " + blockName + " block", TokPos(), TokLine());
	string s = TokVal();
	transform(s.begin(), s.end(), s.begin(), toupper);
	SymTable::iterator it = tbl->find(s);
	if (it != tbl->end())
		throw Error("Identifier already declarated", TokPos(), TokLine());	
	scan.Next();
	return s;
}

SymTable* Parser::GetArgs(){
	list<string> args;
	SymTable* argsTable = NULL;
	SymType* type = NULL;
	if (TokVal() == "("){
		while (TokVal() != ")"){
			scan.Next();
			string s = TokVal();
			if (s == "VAR")
				scan.Next();
			while (TokVal() != ";" && TokVal() != ")"){
				scan.Next();
				string par = CheckCurTok("function", argsTable);
				args.push_back(par);
				if (TokVal() == ",")
					scan.Next();
				else if (TokVal() == ":"){
					scan.Next();
					type = ParseType(false);
					scan.Next();
					break;
				}
				else if (TokVal() != ")" && TokVal() != ";")
					throw Error("Invalid lexem in function declaration", TokPos(), TokLine());
			}
			for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
				if (s == "VAR")
					(*argsTable)[*it] = new SymVarParamByRef(type);
				else
					(*argsTable)[*it] = new SymVarParamByValue(type);
			}
			args.empty();			
			if (TokVal() != ")")
				scan.Next();
		}
	}
	return argsTable;
}

SymProc* Parser::ParseProcedure(bool newProc){
	string ident;
	if (newProc)
		string ident = CheckCurTok("procedure", table);
	SymTable* argsTable = GetArgs();
	scan.Next();
	SymProc* res = new SymProc(argsTable, NULL);
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	scan.Next();
	if (TokVal() != "FORWARD")
		throw Error("Forward expected", TokPos(), TokLine());
	scan.Next();
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	if (newProc)
		(*table)[ident] = res;
	return res;
}

SymFunc* Parser::ParseFunction(bool newFunc){
	string ident;
	if (newFunc)
		string ident = CheckCurTok("function", table);
	SymType* type = NULL;
	SymTable* argsTable = GetArgs();
	scan.Next();
	if (TokVal() != ":")
		throw Error("Function type expected", TokPos(), TokLine());
	scan.Next();
	type = ParseType(false);
	SymFunc* res = new SymFunc(argsTable, NULL, type);
	scan.Next();
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	scan.Next();
	if (TokVal() != "FORWARD")
		throw Error("Forward expected", TokPos(), TokLine());
	scan.Next();
	if (TokVal() != ";")
		throw Error("; expected", TokPos(), TokLine());
	if (newFunc)
		(*table)[ident] = res;
	return res;
}

void Parser::ParseVarBlock(){
	list<string> idents;
	while(true){
		string s = CheckCurTok("var", table);
		idents.push_back(s);
		if (TokVal() == ",")
			scan.Next();
		else if (TokVal() == ":"){
			scan.Next();
			SymType* type = ParseType(false);
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1)
				(*table)[*it1] = type;
			scan.Next();
			if (TokVal() != ";")
				throw Error("; Expected", TokPos(), TokLine());
			scan.Next();
			if (TokVal() == "TYPE" || TokVal() == "CONST" || TokVal() == "PROCEDURE" || 
				TokVal() == "FUNCTION")
				break;
			if (TokVal() == "VAR")
				scan.Next();
		}
	}
}

SymTable* Parser::ParseRecordBlock(){
	SymTable* recTable;
	list<string> idents;
	while(TokVal() != "END"){
		string s = CheckCurTok("record", recTable);
		idents.push_back(s);
		if (TokVal() == ",")
			scan.Next();
		else if (TokVal() == ":"){
			scan.Next();
			SymType* type = ParseType(false);
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1)
				(*recTable)[*it1] = type;
			scan.Next();
			if (TokVal() != ";")
				throw Error("; Expected", TokPos(), TokLine());
			scan.Next();
		} 
		else 
			throw Error("Invalid lexem in record block", TokPos(), TokLine());
	}
	scan.Next();
	return recTable;
}

SymVarConst* Parser::ParseConst(){
	SymVarConst* res = NULL;
	Expr* expr = ParseSimple(4);
	SymType* type = NULL;
	if (!IsConst(expr->ExprType()))
		throw Error("Invalid const declaration", TokPos(), TokLine());
	if (expr->ExprType() == ttIntLit)
		type = new SymTypeInteger();
	else if (expr->ExprType() == ttRealLit)
		type = new SymTypeFloat();
	res = new SymVarConst(type);
	return res;
}

void Parser::ParseConstBlock(){
	while (true){
		string ident = CheckCurTok("constants", table);
		if (TokVal() != "=")
			throw Error("Invalid lexem in const declaration", TokPos(), TokLine());
		scan.Next();
		(*table)[ident] = ParseConst();
		scan.Next();
		if (TokVal() != ";")
			throw Error("; Expected", TokPos(), TokLine());
		scan.Next();
		if (TokVal() == "TYPE" || TokVal() == "VAR" || TokVal() == "PROCEDURE" || 
			TokVal() == "FUNCTION")
			break;
		if (TokVal() == "CONST")
			scan.Next();
	}
}

SymType* Parser::ParseType(bool newType){
	SymType* res = NULL;
	string s = TokVal();
	transform(s.begin(), s.end(), s.begin(), toupper);
	SymTable::iterator it = table->find(s);
	if (it != table->end()){
		if (newType){
			if (!((it->second)->IsType()))
				throw Error("Invalid lexem in type declaration", TokPos(), TokLine());
			res = new SymTypeAlias((SymType*)it->second);	
		} 
		else
			res = (SymType*)it->second;
	} 
	else if (s == "RECORD"){
		SymTable* fields;
		scan.Next();
		fields = ParseRecordBlock();
		res = new SymTypeRecord(fields);
	}
	else if (s == "ARRAY"){
		scan.Next();
		res = ParseArray();
	/*} else if (s == "FUNCTION"){
		scan.Next();
		res = ParseFunction(false);
	}else if (s == "PROCEDURE"){
		scan.Next();
		res = ParseProcedure(false);*/
	} else if (it == table->end() && !newType)
		throw Error("Unknown type", TokPos(), TokLine());
	return res;
}

void Parser::ParseTypeBlock(){
	while(true){
		string ident = CheckCurTok("type", table);
		if (TokVal() != "=")
			throw Error("Invalid lexem in type declaration", TokPos(), TokLine());
		scan.Next();
		(*table)[ident] = ParseType(true);
		scan.Next();
		if (TokVal() != ";")
			throw Error("; Expected", TokPos(), TokLine());
		scan.Next();
		if (TokVal() == "TYPE" || TokVal() == "VAR" || TokVal() == "PROCEDURE" || 
			TokVal() == "FUNCTION")
			return;
		if (TokVal() == "CONST")
			scan.Next();
	}
}

SymTypeArray* Parser::ParseArray(){
	SymTypeArray* res = NULL;
	if (TokVal() != "[")
		throw Error("Invalid array declaration", TokPos(), TokLine());
	list<SymVarConst*> left, right;
	while (TokVal() != "]"){
		SymVarConst* l = ParseConst();
		scan.Next();
		if (TokVal() != "..")
			throw Error("Invalid array declaration", TokPos(), TokLine());
		scan.Next();
		SymVarConst* r = ParseConst();
		left.push_back(l);
		right.push_back(r);
		scan.Next();
		if (TokVal() == ",")
			scan.Next();
	}
	SymType* type = ParseType(false);
	list<SymVarConst*>::iterator itl = left.begin();
	list<SymVarConst*>::iterator itr = right.begin();
	for (; itl != left.end() && itr != right.end(); ++itl, ++itr)
	{
		res = new SymTypeArray(type, *itl, *itr);
		type = res;
	}
	return res;
}

int PaintBranch(int i, int j, int k, int h, bool f){
	for (; k < h; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	if (f)
		for (unsigned int k = 0; k < 3; ++k)
			arr[j][i + k + 1] = '_';
	return j;
}

int FillTreeIdentConst(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int FillTreeOp(int i, int j, string val){
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	j = PaintBranch(i, j, 0, 2, true);
	return j;
}

int FillTreeBinOp(int i, int j, string Value, Expr* left, Expr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

int SimpleIdent::FillTree(int i, int j){
	return FillTreeIdentConst(i, j, Value);
}

int SimpleConst::FillTree(int i, int j){
	return FillTreeIdentConst(i, j, Value);
}

int SimpleBinaryOp::FillTree(int i, int j){
	return FillTreeBinOp(i, j, Value, left, right);
}

int SimpleArrayAccess::FillTree(int i, int j){
	return FillTreeBinOp(i, j, Value, left, right);
}

int SimpleRecordAccess::FillTree(int i, int j){
	return FillTreeBinOp(i, j, Value, left, right);
}

int SimpleFunctionCall::FillTree(int i, int j){
	unsigned int k = 0;
	int tmp = j;
	j = FillTreeOp(i, j, "()");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	j = PaintBranch(i, j, j, j + Value->FillTree(i + 4, j) - 2, false);
	int s = 0;
	int h = 0;
	for (list<Expr*>::iterator it = args.begin() ;it != args.end(); ++it){
		h = (*it)->FillTree(i + 4, j + 2);
		j = PaintBranch(i, j, 0, h, true);
		s += h;
	}
	return j - tmp + 2;
}

int SimpleUnaryOp::FillTree(int i, int j){
	j = FillTreeOp(i, j, Value);
	int h = child->FillTree(i + 4, j);
	return h + 2;
}

void Expr::Print(ostream& os, int n){
	int h = FillTree(0, 0) - 1;
	for (int i = 0; i < h; ++i){
		for (int j = 0; j < maxLength; ++j)
			os << arr[i][j];
		os << "\n";
	}
}

void Parser::FillMaps(){
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	memset(arr, ' ', arrSize * arrSize);
}

int FindOpPrior(string str){
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}

Expr* Parser::ParseSimple(int prior){
	if (prior == 1)
		return ParseFactor();
	Expr* res = ParseSimple(prior - 1);
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseSimple(prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new SimpleBinaryOp(str, res, expr1);
	}
	return res;
}

bool IsUnaryOp(string str){
	transform(str.begin(), str.end(), str.begin(), toupper);
	return str == "-" || str == "+" || str == "NOT";
}

bool IsLiteral(TokenType tt){
	return tt == ttIntLit || tt == ttRealLit || tt == ttStringLit || tt == ttHexLiteral || tt == ttBinLiteral 
		|| tt == ttOctLiteral;
}

Expr* Parser::ParseNext(){
	scan.Next();
	Expr* expr1 = ParseSimple(4);
	if (!expr1)
		throw Error("Lexem expected, EOF found", TokPos(), TokLine());
	return expr1;
}

Expr* Parser::ParseFunctionCall(Expr* res, int pos, int line){
	Expr* expr1 = ParseNext();
	list<Expr*> arg;
	while (TokVal() != ")"){
		if (TokVal() == ","){
			arg.push_back(expr1);
			expr1 = ParseNext();
		}
		else if (TokVal() != ")")
			throw Error("Unexpected lexem found", TokPos(), TokLine());
	}
	arg.push_back(expr1);
	res = new SimpleFunctionCall(res, arg);
	scan.Next();
	return res;
}

Expr* Parser::ParseArrayAccess(Expr* res, int pos, int line){
	Expr* expr1 = ParseNext();
	res = new SimpleArrayAccess("[]", res, expr1);
	while (TokVal() !=  "]"){
		if (TokVal() == ","){	
			expr1 = ParseNext();
			res = new SimpleArrayAccess("[]", res, expr1);
		}
		else if (TokVal() != "]")
			throw Error("Unexpected lexem found", TokPos(), TokLine());		
	}
	scan.Next();
	return res;
}

Expr* Parser::ParseRecordAccess(Expr* res, int pos, int line){
	Expr* expr1 = ParseNext();
	if (!expr1->LValue() && !expr1->IsFunction())
		throw Error("Invalid record access", pos, line);
	res = new SimpleRecordAccess(".", res, expr1);
	while (TokVal() ==  "."){
		expr1 = ParseNext();
		if (!expr1->LValue() && !expr1->IsFunction())
			throw Error("Invalid record access", pos, line);
		res = new SimpleRecordAccess(".", res, expr1);
	}
	return res;
}

Expr* Parser::CallAccess(Expr* id, int pos, int line){
	Expr* res = id;
	isAccess = true;
	while (TokVal() == "(" || TokVal() == "[" || TokVal() == "."){
		if (TokVal() == "(")
			res = ParseFunctionCall(res, pos, line);
		else if (TokVal() == "[")
			res = ParseArrayAccess(res, pos, line);
		else if (TokVal() == "."){
			isRecord = true;
			res = ParseRecordAccess(res, pos, line);
		}
	}
	isAccess = false;
	isRecord = false;
	return res;
}

Expr* Parser::ParseFactor(){
	Expr* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (TokType() == ttEOF)
		return NULL;
	if (TokType() == ttBadToken)
		throw Error("Invalid lexem", pos, line);
	if (IsUnaryOp(TokVal())){
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseFactor();
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new SimpleUnaryOp(str, expr1);
	}
	else if (TokType() == ttIdentifier){
		SimpleIdent* id = new SimpleIdent(TokVal());
		scan.Next();
		if (TokVal() == "[" || TokVal() == "(" || (TokVal() == "." && !isRecord))
			res = CallAccess(id, pos, line);
		else
			res = id;
		
	}
	else if (IsLiteral(TokType())){
		res = new SimpleConst(TokVal());
		scan.Next();
	}
	else if (TokVal() == "("){
		scan.Next();
		if (TokVal() == ")")
			throw Error("Empty bracket sequence", pos, line);
		res = ParseSimple(4);
		if (TokVal() != ")")
			throw Error("Unclosed bracket", pos, line);
		scan.Next();
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = CallAccess(res, pos, line);
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}