#ifndef _COMMON_H_
#define _COMMON_H_
#include <sstream>
#include <map>
#include <string>
#include <list>
#include <algorithm>
using namespace std;
const int arrSize = 500;
static char arr[arrSize][arrSize];
static int maxLength = 0;
static int maxN = 0;
static map<string, int> priority;
class Symbol;
typedef pair<string, Symbol*> Sym;
typedef map <string, Symbol*> SymTable;
typedef list<SymTable*> SymTableStack;
typedef SymTableStack::iterator SymStIt;
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
	ttKeyWordB,
	ttAnd, //KeyWord
	ttArray,
	ttBegin,
	ttBreak,
	ttCase,
	ttConst,
	ttContinue,
	ttDiv,
	ttDo,
	ttDownto,
	ttElse,
	ttEnd,
	ttFor,
	ttFunction,
	ttForward,
	ttGoto,
	ttIf,
	ttIn,
	ttMod,
	ttNil,
	ttNot,
	ttOf,
	ttOr,
	ttProcedure,
	ttProgram,
	ttRecord,
	ttRepeat,
	ttSet,
	ttShl,
	ttShr,
	ttString,
	ttThen,
	ttTo,
	ttType,
	ttUntil,
	ttUses,
	ttVar,
	ttWhile,
	ttWith,
	ttXor,
	ttReadln,
	ttWriteln,
	ttRead,
	ttWrite,
	ttInteger,
	ttByte,
	ttShortint,
	ttWord,
	ttLongint, 
	ttInt64,
	ttBoolean,
	ttChar,
	ttSingle,
	ttReal,
	ttDouble,
	ttExtended,
	ttComp,
	ttKeyWordE,
	ttSeparatorB,
	ttDot, //Separator
	ttPoint,
	ttLeftParentheses,
	ttRightParentheses,
	ttLeftBracket,
	ttRightBracket,
	ttLeftBrace,
	ttRightrace,
	ttSemi,
	ttColon,
	ttDblDot, 
	ttSeparatorE,
	ttOperationB,
	ttPlus, //Operation 
	ttMinus,
	ttRealDiv,
	ttMul,
	ttEq,
	ttMore,
	ttLess,
	ttNotEq,
	ttAssign,
	ttMoreEq,
	ttLessEq,
	ttCaret,
	ttDog, 
	ttOperationE,
    ttLiteral,  //Other
	ttIntLit,
	ttRealLit, 
	ttStringLit, 
	ttHexLiteral,
	ttBinLiteral, 
	ttOctLiteral,
	ttEOF, 
	ttBadToken, 
	ttSpecSymb, 
};
enum cmd{
	asmMov, 
	asmAdd,
	asmSub,
	asmDiv,
	asmMul,
	asmIDiv, 
	asmIMul,
	asmAnd,
	asmOr,
	asmXor,
	asmNot,
	asmShl,
	asmShr,
	asmPush, 
	asmPop, 
	asmCmp,
	asmJmp, 
	asmDw,
	asmDb,
	asmDd,
	asmUnknown,
	asmCode, 
	asmData,
	asmInvoke,
	asmPrintf, 
	asmScanf,
	asmCall,
	asmDwtoa,
	asmDup,
	asmStdout,
	asmNeg, 
};
enum procType{
	pMain,
	pProc,
	pData,
	pCode,
};
static map<cmd, string> cmdNames;
static map<string, cmd> cmdTypes;
class Error 
{
public:
	Error(){};
	Error(string txt, int pos, int line): text(txt), errorPos(pos), errorLine(line){};
	string GetText() {return text; }
	int GetErrorPos() {return errorPos; }
	int GetErrorLine() {return errorLine; }
protected:
	string text;
	int errorPos, errorLine;
};
void CheckAccess(string s, int i, int j);
bool IsIntOperator(string val);
bool IsLogicOperator(string val);
template <typename T>
string toString(T val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}
void SmthExpexted(string tok, int pos, int line, string exp);
bool AnothBlock(string s);
int FindOpPrior(string str);
void FillMaps();
void ClearArr();
void PrintExpr(ostream& os, int n);
int PaintBranch(int i, int j, int k, int h, bool f);
int FillTreeIdentConst(int i, int j, string val);
int FillTreeOp(int i, int j, string val);
bool IsKeyWord(string tmp);
bool IsKeyWord(TokenType tt);
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