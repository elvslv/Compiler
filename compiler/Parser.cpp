#include "Parser.h"

map<string, ExprType> ET;
map<ExprType, int> priority;
char arr[500][500];
static int maxLength = 0;

string UpperCase_(string tmp)
{
	for (unsigned int i = 0; i < tmp.length(); ++i)
		if (tmp[i] >= 'a' && tmp[i] <= 'z')
			tmp[i] = tmp[i] - 'a' + 'A';
	return tmp;
}

int FillTreeIdentConst(int i, int j, string val)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int FillTreeOp(int i, int j, string val)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
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
	for (unsigned int k = 0; k < hl; ++k)
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
	for (unsigned int i = 0; i < h; ++i)
	{
		for (unsigned int j = 0; j < maxLength; ++j)
			os << arr[i][j];
		os << "\n";
	}
}

void Parser::FillMaps()
{
	ET["-"] = etMinus; ET["+"] = etPlus; ET["*"] = etMul; ET["/"] = etDiv; ET["DIV"] = etIntDiv; ET["MOD"] = etMod;
	ET["NOT"] = etNot; ET["AND"] = etAnd; ET["OR"] = etOr; ET["XOR"] = etXor; ET["SHL"] = etShl; ET["SHR"] = etShr; 
	ET["="] = etEqual; ET["<>"] = etNotEq; ET["<"] = etLess; ET["<="] = etLessOrEq; ET[">"] = etMore; ET[">="] = etMoreOrEq;
	priority[etNot] = 1; 
	priority[etMul] = 2; priority[etDiv] = 2; priority[etIntDiv] = 2; priority[etMod] = 2; priority[etAnd] = 2; 
	priority[etShl] = 2; priority[etShr] = 2; 
	priority[etMinus] = 3; priority[etPlus] = 3; priority[etOr] = 3; priority[etXor] = 3;
	priority[etEqual] = 4; priority[etNotEq] = 4; priority[etLess] = 4;	priority[etLessOrEq] = 4; priority[etMore] = 4; 
	priority[etMoreOrEq] = 4;
	memset(arr, ' ', 500*500);
}

ExprType FindExprType(string val)
{
	map<string, ExprType>::iterator it;
	it = ET.find(UpperCase_(val));
	if (it != ET.end())
		return it->second;
	else
		return etExpr;
}

int FindOpPrior(ExprType t)
{
	map<ExprType, int>::iterator it;
	it = priority.find(t);
	if (it != priority.end())
		return it->second;
	else
		return 10;
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
	while (FindOpPrior(FindExprType(TokVal())) == prior)
	{
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ParseSimple(prior - 1);
		if (expr1)
			res = new BinaryOp(str, res, expr1);
		else
			throw Error("Lexem Expected, EOF Found", TokPos(), TokLine());
	}
	return res;
}

bool IsUnaryOp(string str)
{
	return FindExprType(str) == etMinus || FindExprType(str) == etPlus || FindExprType(str) == etNot;
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
		if (expr1)
			res = new UnaryOp(str, expr1);
		else
			throw Error("Lexem Expected, EOF Found", TokPos(), TokLine());
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
		else
		{
			res = ParseSimple(4);
			if (TokVal() != ")")
				throw Error("Unclosed Bracket", pos, line);
			else
				scan.Next();
		}
	}
	else
		throw Error("Unexpected Lexem Found", pos, line);
	return res;
}