#include "Parser.h"
map<string, int> priority;
const int arrSize = 500;
char arr[arrSize][arrSize];
static int maxLength = 0;

int PaintBranch(int i, int j, int k, int h, bool f) 
{
	for (; k < h; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	if (f)
		for (unsigned int k = 0; k < 3; ++k)
			arr[j][i + k + 1] = '_';
	return j;
}

int FillTreeIdentConst(int i, int j, string val)
{
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int FillTreeOp(int i, int j, string val)
{
	unsigned int k = 0;
	for (; k < val.length(); ++k)
		arr[j][i + k] = val[k];
	j = PaintBranch(i, j, 0, 2, true);
	return j;
}

int FillTreeBinOp(int i, int j, string Value, Expr* left, Expr* right)
{
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

int Ident::FillTree(int i, int j)
{
	return FillTreeIdentConst(i, j, Value);
}

int Const::FillTree(int i, int j)
{
	return FillTreeIdentConst(i, j, Value);
}

int BinaryOp::FillTree(int i, int j)
{
	return FillTreeBinOp(i, j, Value, left, right);
}

int ArrayAccess::FillTree(int i, int j)
{
	return FillTreeBinOp(i, j, Value, left, right);
}

int RecordAccess::FillTree(int i, int j)
{
	return FillTreeBinOp(i, j, Value, left, right);
}

int FunctionCall::FillTree(int i, int j)
{
	unsigned int k = 0;
	int tmp = j;
	j = FillTreeOp(i, j, "()");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	j = PaintBranch(i, j, j, j + Value->FillTree(i + 4, j) - 2, false);
	int s = 0;
	int h = 0;
	for (list<Expr*>::iterator it = args.begin() ;it != args.end(); ++it)
	{
		h = (*it)->FillTree(i + 4, j + 2);
		j = PaintBranch(i, j, 0, h, true);
		s += h;
	}
	return j - tmp + 2;
}

int UnaryOp::FillTree(int i, int j)
{
	j = FillTreeOp(i, j, Value);
	int h = child->FillTree(i + 4, j);
	return h + 2;
}

void Expr::Print(ostream& os, int n)
{
	int h = FillTree(0, 0) - 1;
	for (int i = 0; i < h; ++i)
	{
		for (int j = 0; j < maxLength; ++j)
			os << arr[i][j];
		os << "\n";
	}
}

void Parser::FillMaps()
{
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	memset(arr, ' ', arrSize * arrSize);
}

int FindOpPrior(string str)
{
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}

Expr* Parser::ParseSimple(int prior)
{
	if (prior == 1)
		return ParseFactor();
	Expr* res = ParseSimple(prior - 1);
	while (FindOpPrior(TokVal()) == prior)
	{
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseSimple(prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new BinaryOp(str, res, expr1);
	}
	return res;
}

bool IsUnaryOp(string str)
{
	transform(str.begin(), str.end(), str.begin(), toupper);
	return str == "-" || str == "+" || str == "NOT";
}

bool IsLiteral(TokenType tt)
{
	return tt == ttIntLit || tt == ttRealLit || tt == ttStringLit || tt == ttHexLiteral || tt == ttBinLiteral 
		|| tt == ttOctLiteral;
}

Expr* Parser::ParseNext()
{
	scan.Next();
	Expr* expr1 = ParseSimple(4);
	if (!expr1)
		throw Error("Lexem expected, EOF found", TokPos(), TokLine());
	return expr1;
}

Expr* Parser::ParseFunctionCall(Expr* res, int pos, int line)
{
	Expr* expr1 = ParseNext();
	list<Expr*> arg;
	while (TokVal() != ")")
	{
		if (TokVal() == ",")
		{
			arg.push_back(expr1);
			expr1 = ParseNext();
		}
		else if (TokVal() != ")")
			throw Error("Unexpected lexem found", TokPos(), TokLine());
	}
	arg.push_back(expr1);
	res = new FunctionCall(res, arg);
	scan.Next();
	return res;
}

Expr* Parser::ParseArrayAccess(Expr* res, int pos, int line)
{
	Expr* expr1 = ParseNext();
	res = new ArrayAccess("[]", res, expr1);
	while (TokVal() !=  "]")
	{
		if (TokVal() == ",")
		{	
			expr1 = ParseNext();
			res = new ArrayAccess("[]", res, expr1);
		}
		else if (TokVal() != "]")
			throw Error("Unexpected lexem found", TokPos(), TokLine());		
	}
	scan.Next();
	return res;
}

Expr* Parser::ParseRecordAccess(Expr* res, int pos, int line)
{
	Expr* expr1 = ParseNext();
	if (!expr1->IsIdent())
		throw Error("Invalid record access", pos, line);
	res = new RecordAccess(".", res, expr1);
	while (TokVal() ==  ".")
	{
		expr1 = ParseNext();
		if (!expr1->IsIdent())
			throw Error("Invalid record access", pos, line);
		res = new RecordAccess(".", res, expr1);
	}
	return res;
}

Expr* Parser::CallAccess(Expr* id, int pos, int line)
{
	Expr* res = id;
	isAccess = true;
	while (TokVal() == "(" || TokVal() == "[" || TokVal() == ".")
	{
		if (TokVal() == "(")
			res = ParseFunctionCall(res, pos, line);
		else if (TokVal() == "[")
			res = ParseArrayAccess(res, pos, line);
		else if (TokVal() == ".")
		{
			isRecord = true;
			res = ParseRecordAccess(res, pos, line);
		}
	}
	isAccess = false;
	isRecord = false;
	return res;
}

Expr* Parser::ParseFactor()
{
	Expr* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (TokType() == ttEOF)
		return NULL;
	if (TokType() == ttBadToken)
		throw Error("Invalid lexem", pos, line);
	if (IsUnaryOp(TokVal()))
	{
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseFactor();
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new UnaryOp(str, expr1);
	}
	else if (TokType() == ttIdentifier)
	{
		Ident* id = new Ident(TokVal());
		scan.Next();
		if (TokVal() == "[" || TokVal() == "(" || (TokVal() == "." && !isRecord))
			res = CallAccess(id, pos, line);
		else
			res = id;
		
	}
	else if (IsLiteral(TokType()))
	{
		res = new Const(TokVal());
		scan.Next();
	}
	else if (TokVal() == "(")
	{
		scan.Next();
		if (TokVal() == ")")
			throw Error("Empty bracket sequence", pos, line);
		res = ParseSimple(4);
		if (TokVal() != ")")
			throw Error("Unclosed bracket", pos, line);
		scan.Next();
		if (res->IsIdent() && !isAccess)
			res = CallAccess(res, pos, line);
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}