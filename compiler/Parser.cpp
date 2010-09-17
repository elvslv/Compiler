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

int Ident::FillTree(int i, int j)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int Const::FillTree(int i, int j)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
	if (i + k > maxLength)
		maxLength = i + k;
	return 2;
}

int BinaryOp::FillTree(int i, int j)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
	++j;
	arr[j++][i] = '|';
	arr[j][i] = '|';
	for (k = 0; k < 3; ++k)
		arr[j][i + k + 1] = '_';
	int hl = left->FillTree(i + 4, j);
	for (k = 0; k < hl; ++k)
		arr[++j][i] = '|';
	arr[j][i] = '|';
	for (k = 0; k < 3; ++k)
		arr[j][i + k + 1] = '_';
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

int UnaryOp::FillTree(int i, int j)
{
	unsigned int k = 0;
	for (; k < Value.length(); ++k)
		arr[j][i + k] = Value[k];
	++j;
	arr[j++][i] = '|';
	arr[j][i] = '|';
	for (k = 0; k < 3; ++k)
		arr[j][i + k + 1] = '_';
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

Expr* Parser::ParseSimpleExpr(int prior)
{
	try
	{
		if (HasError())
			return NULL;
		if (prior == 1)
			return ParseFactor();
		Expr* res = ParseSimpleExpr(prior - 1);
		Token* t = scan.GetToken();
		while (FindOpPrior(FindExprType(t->GetValue())) == prior)
		{
			string str = t->GetValue();
			scan.Next();
			t = scan.GetToken();
			Expr* expr1 = ParseSimpleExpr(prior - 1);
			if (expr1)
				res = new BinaryOp(str, res, expr1);
			else
				throw Error("Incorrect Syntax Construction", t->GetPos(), t->GetLine());
		}
		return res;
	}
	catch(Error err)
	{
		error = err;
		return NULL;
	}
}

Expr* Parser::ParseFactor()
{
	if (HasError())
		return NULL;
	Expr* res;
	try
	{
		Token* t = scan.GetToken();
		int pos = t->GetPos();
		int line = t->GetLine();
		if (t->GetType() == ttEOF)
			return NULL;
		if (FindExprType(t->GetValue()) == etMinus || FindExprType(t->GetValue()) == etPlus || 
			FindExprType(t->GetValue()) == etNot)
		{
			string str = t->GetValue();
			scan.Next();
			res = new UnaryOp(str, ParseFactor());
		}
		else
			if (t->GetType() == ttIdentifier)
			{
				res = new Ident(t->GetValue());
				scan.Next();
			}
			else
				if (t->GetType() == ttIntLit || t->GetType() == ttRealLit || t->GetType() == ttStringLit
					|| t->GetType() == ttHexLiteral || t->GetType() == ttBinLiteral || t->GetType() == ttOctLiteral)
				{
					res = new Const(t->GetValue());
					scan.Next();
				}
				else
					if (t->GetValue() == "(")
					{
						scan.Next();
						t = scan.GetToken();
						if (t->GetValue() == ")")
						{
							scan.Next();
							res = ParseFactor();
						}
						else
						{
							res = ParseSimpleExpr(4);
							t = scan.GetToken();
							if (t->GetValue() != ")")
								throw Error("Unclosed Bracket", pos, line);
							else
								scan.Next();
						}
					}
					else
						throw Error("Incorrect Syntax Construction", pos, line);
	}
	catch (Error err)
	{
		error = err;
		return NULL;
	}
	return res;
}