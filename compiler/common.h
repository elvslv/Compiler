#ifndef _COMMON_H_
#define _COMMON_H_
#include <sstream>
#include <map>
#include <string>
#include <algorithm>
using namespace std;
const int arrSize = 500;
static char arr[arrSize][arrSize];
static int maxLength = 0;
static map<string, int> priority;
enum State
{
	stNewLex, 
	stDigConsq, 
	stIdent, 
	stSlash, 
	stRndBracket,
	stBrace, 
	stLess, 
	stMore, 
	stColon,
	stRealLit,
	stApostrophe, 
	stHashSign, 
	stDollar,
	stRealELast,
	stRealLitWithoutE,
	stCmntSlash, 
	stCmntRndBracket, 
	stStrLitWithHash0Dig, 
	stStrLitWithHash1Dig, 
	stStrLitWithHash2Dig, 
	stStrLitWithHash3Dig, 
	stCmntRndPosToClose, 
	stClosedApostrophe, 
	stRealLitSignLast, 
	stRealLitPointLast, 
	stPointBeforeEOF, 
	stPercent, 
	stAmpersand, 
	stPoint, 
};
enum TokenType{
	ttToken, 
    ttIdentifier, 
	ttKeyWord,
    ttLiteral, 
	ttIntLit,
	ttRealLit, 
	ttStringLit, 
	ttHexLiteral,
	ttBinLiteral, 
	ttOctLiteral,
	ttOperation, 
	ttSeparator,
	ttEOF, 
	ttBadToken, 
	ttDelimiter, 
	ttSpecSymb, 

};
int FindOpPrior(string str);
void FillMaps();
void ClearArr();
void PrintExpr(ostream& os, int n);
int PaintBranch(int i, int j, int k, int h, bool f);
int FillTreeIdentConst(int i, int j, string val);
int FillTreeOp(int i, int j, string val);
bool IsKeyWord(string tmp);
bool IsAlpha(char ch);
bool IsDigit(char ch);
bool IsWhiteSpace(char ch);
bool IsConcreteType(char ch, TokenType type);
bool IsHexDigit(char ch);
bool IsBinDigit(char ch);
bool IsOctDigit(char ch);
class Expr;
int PaintBranch(int i, int j, int k, int h, bool f);
int FillTreeIdentConst(int i, int j, string val);
int FillTreeOp(int i, int j, string val);
int FindOpPrior(string str);
bool IsUnaryOp(string str);
bool IsLiteral(TokenType tt);
bool IsConstType(TokenType type);
bool IsIntType(TokenType type);
string UpCase(string s);


#endif //_COMMON_H_