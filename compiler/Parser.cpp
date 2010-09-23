#include "Parser.h"
map<string, int> priority;
const int arrSize = 500;
char arr[arrSize][arrSize];
static int maxLength = 0;

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
	++j;
	arr[j++][i] = '|';
	arr[j][i] = '|';
	for (k = 0; k < 3; ++k)
		arr[j][i + k + 1] = '_';
	return j;
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
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	for (int k = 0; k < hl; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	for (unsigned int k = 0; k < 3; ++k)
		arr[j][i + k + 1] = '_';
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
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
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; priority["AND"] = 2; 
	priority["SHL"] = 2; priority["SHR"] = 2; 
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

Expr* Parser::ParseSimpleExpr()
{
	try
	{
		Expr* res = ParseSimple(4);
		return res;
	}
	catch(Error err)
	{
		error = err;
		return NULL;
	}
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
			throw Error("Lexem Expected, EOF Found", TokPos(), TokLine());
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

Expr* Parser::ParseFactor()
{
	Expr* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (TokType() == ttEOF)
		return NULL;
	if (TokType() == ttBadToken)
		throw Error("Invalid Lexem", pos, line);
	if (IsUnaryOp(TokVal()))
	{
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseFactor();
		if (!expr1)
			throw Error("Lexem Expected, EOF Found", TokPos(), TokLine());
		res = new UnaryOp(str, expr1);
	}
	else if (TokType() == ttIdentifier)
	{
		res = new Ident(TokVal());
		scan.Next();
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
			throw Error("Empty Bracket Sequence", pos, line);
		res = ParseSimple(4);
		if (TokVal() != ")")
			throw Error("Unclosed Bracket", pos, line);
		scan.Next();
	}
	else
		throw Error("Unexpected Lexem Found", pos, line);
	return res;
}