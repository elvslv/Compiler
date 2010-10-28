#include "scanner.h"
#include <iostream>
map<string, TokenType> TT;
const int TabSize = 4;

KeyWord::KeyWord(int pos, int line, string val) : Identifier(pos, line, val){
	type = TT[UpCase(val)];
}

Operation::Operation(int pos, int line, string val) : Token(pos, line, val){
	type = TT[UpCase(val)];
}

Separator::Separator(int pos, int line, string val) : Token(pos, line, val){
	type = TT[UpCase(val)];
}

bool IsKeyWord(string tmp)
{
	map<string, TokenType>::iterator it;
	transform(tmp.begin(), tmp.end(), tmp.begin(), toupper);
	it = TT.find(tmp);
	return it != TT.end() && it->second > ttKeyWordB && it->second < ttKeyWordE;
}

bool IsSeparator(TokenType tt){
	return tt > ttSeparatorB && tt < ttSeparatorE;
}

bool IsOperation(TokenType tt){
	return tt > ttOperationB && tt < ttOperationE;
}

bool IsKeyWord(TokenType tt){
	return tt > ttKeyWordB && tt < ttKeyWordE;
}

bool IsSeparator(char ch){
	string str;
	str += ch;
	return IsSeparator(TT[str]);
}

bool IsOperation(char ch){
	string str;
	str += ch;
	return IsOperation(TT[str]);
}

bool IsKeyWord(char ch){
	string str;
	str += ch;
	return IsKeyWord(TT[str]);
}

bool IsConcreteType(char ch, TokenType type)
{
	map<string, TokenType>::iterator it;
	string t = "";
	t += ch;
	it = TT.find(t);
	if (it == TT.end())
		return 0;
	return it->second == type;
}

void FillTokenTypes()
{
	TT["AND"] = ttAnd; TT["ARRAY"] = ttArray; TT["BEGIN"] = ttBegin; TT["CASE"] = ttCase;
	TT["CONST"] = ttConst; TT["DIV"] = ttDiv; TT["DO"] = ttDo; TT["DOWNTO"] = ttDownto;
	TT["ELSE"] = ttElse; TT["END"] = ttEnd; TT["FOR"] = ttFor; TT["FUNCTION"] = ttFunction;
	TT["FORWARD"] = ttForward; TT["GOTO"] = ttGoto; TT["IF"] = ttIf; TT["IN"] = ttIn;
	TT["MOD"] = ttMod; TT["NIL"] = ttNil; TT["NOT"] = ttNot; TT["OF"] = ttOf;
	TT["OR"] = ttOr; TT["PROCEDURE"] = ttProcedure; TT["PROGRAM"] = ttProgram; TT["RECORD"] = ttRecord;
	TT["REPEAT"] = ttRepeat; TT["SET"] = ttSet; TT["SHL"] = ttShl; TT["SHR"] = ttShr;
	TT["STRING"] = ttString; TT["THEN"] = ttThen; TT["TO"] = ttTo; TT["TYPE"] = ttType;
	TT["UNTIL"] = ttUntil; TT["USES"] = ttUses; TT["VAR"] = ttVar; TT["WHILE"] = ttWhile;
	TT["WITH"] = ttWith; TT["XOR"] = ttXor; TT["READLN"] = ttReadln; TT["WRITELN"] = ttWriteln;
	TT["READ"] = ttRead; TT["WRITE"] = ttWrite; TT["INTEGER"] = ttInteger; TT["BYTE"] = ttByte;
	TT["SHORTINT"] = ttShortint; TT["WORD"] = ttWord; TT["LONGINT"] = ttLongint; TT["INT64"] = ttInt64;
	TT["BOOLEAN"] = ttBoolean; TT["CHAR"] = ttChar; TT["SINGLE"] = ttSingle; TT["REAL"] = ttReal;
	TT["DOUBLE"] = ttDouble; TT["EXTENDED"] = ttExtended; TT["COMP"] = ttComp; 
	TT["."] = ttDot; TT[","] = ttPoint; TT["("] = ttLeftParentheses; TT[")"] = ttRightParentheses; TT["["] = ttLeftBracket;
	TT["]"] = ttRightBracket; TT["{"] = ttLeftBrace; TT["}"] = ttRightrace; TT[";"] = ttSemi; TT[":"] = ttColon;
	TT["+"] = ttPlus; TT["-"] = ttMinus; TT["/"] = ttRealDiv; TT["*"] = ttMul; TT["="] = ttEq;
	TT[">"] = ttMore; TT["<"] = ttLess; TT["<>"] = ttNotEq; TT[":="] = ttAssign; TT[">="] = ttMoreEq;
	TT["<="] = ttLessEq;  TT["^"] = ttCaret; TT["@"] = ttDog; TT[".."] = ttDblDot; 
}

void Scanner::GC()
{
	if (!file.eof())
		file.get(curCh);
}

void Scanner::ChangePosIndex()
{
	if (curCh == '\n' && !file.eof())
	{
		++line;
		pos = 0;
	}
	else
		if (curCh == '\t')
			pos += TabSize;
		else
			++pos;
}

void Scanner::CurChPutBack(char ch)
{
	file.putback(curCh);
	curCh = ch;
}

string Scanner::Change(string str)
{
	string res = "";
	string tmp = "";
	int len = str[str.length() - 1] == '\'' ? str.length() - 1 : str.length();
	int i = str[0] == '\'' ? 1 : 0;
	char k = 0;
	int cnt = 0;
	bool f = false;
	bool opened = str[0] == '\'';
	while (i < len)
	{
		if (str[i] == '#' && !opened)
		{
			k = 0;
			int m = 0;
			++i;
			if (tmp.length() && tmp[tmp.length() - 1] == '\'')
				tmp.replace(tmp.length() - 1, 1, "");
			while(IsDigit(str[i]))
				m = m * 10 + str[i++] - '0';
			k = m;
			if (m > 256)
				tmp = tmp + "?";
			else
				tmp = tmp + k;
		}
		else
		{
			if (str[i] == '\'')
			{
				cnt = 0;
				while(i < len && str[i] == '\'')
				{
					opened = !opened;
					++cnt;
					++i;
				}
				for (int j = 0; j < cnt / 2; tmp = tmp + '\'', ++j);
			}
			else
				tmp = tmp + str[i++];
		}
	}
	return '\'' + tmp + '\'';
}

void Scanner::NewToken(TokenType tt, int p, int l, string val, char ch)
{
	if (curToken)
		delete curToken;
	switch(tt)
	{
	case ttToken:
		curToken = new Token();
		break;
	case ttIdentifier:
		curToken = new Identifier(p, l, val);
		break;
	case ttLiteral:
		curToken = new Literal(p, l, val);
		break;
	case ttIntLit:
		curToken = new IntLiteral(p, l, val);
		break;
	case ttRealLit:
		curToken = new RealLiteral(p, l, val);
		break;
	case ttStringLit:
		curToken = new StringLiteral(p, l, val);
		break;
	case ttHexLiteral:
		curToken = new HexLiteral(p, l, val);
		break;
	case ttBinLiteral:
		curToken = new BinLiteral(p, l, val);
		break;
	case ttOctLiteral:
		curToken = new OctLiteral(p, l, val);
		break;
	case ttEOF:
		curToken = new EndOfFile(p, l);
		break;
	case ttBadToken:
		curToken = new BadToken(p, l, val);
		break;
	default:
		if (tt > ttKeyWordB && tt < ttKeyWordE)
			curToken = new KeyWord(p, l, val);
		if (tt > ttOperationB && tt < ttOperationE)
			curToken = new Operation(p, l, val);
		if (tt > ttSeparatorB && tt < ttSeparatorE)
			curToken = new Separator(p, l, val);
	}
	if (ch == -52)
		ChangePosIndex();
	else
		CurChPutBack(ch);
	curPos = pos;
	curLine = line;
	state = stNewLex;
}

int Scanner::StateStNewLex()
{
	curPos = pos;
	curLine = line;
	if (file.eof())
	{
		NewToken(ttEOF, curPos, curLine, "", -52);
		return 0;
	}
	curStr = "";
	if (IsWhiteSpace(curCh))
		return 1;
	if (IsDigit(curCh))
		state = stDigConsq;
	else
		if (IsAlpha(curCh) || curCh == '_')
			state = stIdent;
		else
			switch(curCh)
			{
				case '/': state = stSlash; break;
				case '(': state = stRndBracket; break;
				case '{': state = stBrace; break;
				case '<': state = stLess; break;
				case '>': state = stMore; break;
				case ':': state = stColon; break;
				case '\'': state = stApostrophe; break;
				case '#': state = stHashSign; break;
				case '$': state = stDollar; break;
				case '%': state = stPercent; break;
				case '&': state = stAmpersand; break;
				case '.': state = stPoint; break;
				case'+': case'-': case ',':case ')': case '[': case ']': case '}': case ';': case '*': case '=': case '@': case '^':
					NewToken(TT[string(curStr + curCh)], curPos, curLine, curStr + curCh, -52);
					return 0;
				default:
				{
					NewToken(ttBadToken, curPos, curLine, "Unknown Symbol", -52);
					return 0;
				}
			}
	return 1;
}

int Scanner::StateStDigConsq()
{
	if (curCh == 'e' || curCh == 'E')
		state = stRealELast;   
	else
		if (curCh == '.')
			state = stRealLitPointLast;    
		else
		{
			if (file.eof() || IsSeparator(curCh) || IsOperation(curCh) || IsWhiteSpace(curCh))
			{
				NewToken(ttIntLit, curPos, curLine, curStr, prevch);
				return 0;
			}
			else
				if (!IsDigit(curCh))
				{
					NewToken(ttBadToken, curPos, curLine, "Incorrect Number", -52);
					return 0;
				}
		}
	return 1;
}

int Scanner::StateStIdent()
{
	if (file.eof() || IsWhiteSpace(curCh) || IsSeparator(curCh) || IsOperation(curCh))
	{
		if (IsKeyWord(curStr))
			NewToken(TT[UpCase(curStr)], curPos, curLine, curStr, prevch);
		else
			NewToken(ttIdentifier, curPos, curLine, curStr, prevch);
		return 0;
	}
	else
		if (!IsAlpha(curCh) && !IsDigit(curCh) && curCh != '_')
		{
			NewToken(ttBadToken, curPos, curLine, "Incorrect Identifier Literal", -52);
			return 0;
		}
	return 1;
}

int Scanner::StateStRealLitWithoutE()
{
	if (curCh == 'e' || curCh == 'E')
		state = stRealELast;
	else
	{
		if (file.eof() || IsSeparator(curCh) || IsOperation(curCh) || IsWhiteSpace(curCh))
		{
			NewToken(ttRealLit, curPos, curLine, curStr, prevch);
			return 0;
		}
		else
			if (!IsDigit(curCh))
			{
				NewToken(ttBadToken, curPos, curLine, "Incorrect Real Literal", -52);
				return 0;
			}
	}
	return 1;
}

int Scanner::StateStStrLitWithHashDigs()
{
	if (IsDigit(curCh) && state < stStrLitWithHash3Dig)
		state = State(state + 1);
	if (curCh == '\'')
		state = stApostrophe;
	else
		if (file.eof() || IsSeparator(curCh) || IsOperation(curCh) || IsWhiteSpace(curCh))
		{
			NewToken(ttStringLit, curPos, curLine, Change(curStr), prevch); 
			return 0;
		}
		else
			if (curCh == '#')
				state = stStrLitWithHash0Dig;
			else
				if (!IsDigit(curCh))
				{
					NewToken(ttBadToken, curPos, curLine, "Incorrect String Literal", -52);
					return 0;
				}
	return 1;
}

int Scanner::StateStCloseApostrophe()
{
	if (!file.eof() && curCh == '\'')
		state = stApostrophe;
		else
			if (!file.eof() && curCh == '#')
				state = stStrLitWithHash0Dig;
			else
			{
				NewToken(ttStringLit, curPos, curLine, Change(curStr), prevch);
				return 0;
			}
	return 1;
}

int Scanner::StateStRealLitPointLast()
{
	if (IsDigit(curCh))
		state = stRealLitWithoutE;
	else
		if (curCh == 'e' || curCh == 'E')
			state = stRealELast;
		else
		{
			--pos;
			curStr = curStr.erase(curStr.length() - 1, 1);
			CurChPutBack(prevch);
			NewToken(ttIntLit, curPos, curLine, curStr, '.');
			if (file.eof())
				state = stPointBeforeEOF;
			return 0;
		}
	return 1;
}

int Scanner::StateStDollarPercentAmpersand()
{
	string errs[3] = {"Incorrect Binary Literal", "Incorrect Octal Literal", "Incorrect Hex Literal"};
	char chrs[3] = {'%', '&', '$'};
	TokenType types[3] = {ttBinLiteral, ttOctLiteral, ttHexLiteral};
	typedef bool (* fptr) (char);
	fptr f_array[] = {IsBinDigit, IsOctDigit, IsHexDigit};
	int i;
	switch(state)
	{
	case stPercent:
		i = 0; break;
	case stAmpersand: 
		i = 1; break;
	case stDollar:
		i = 2; break;
	}
	if (file.eof() || IsSeparator(curCh) || IsOperation(curCh) || IsWhiteSpace(curCh))
	{
		if (curStr.length() == 1)
		{
			NewToken(ttBadToken, curPos, curLine, errs[i], -52);
			return 0;
		}
		else	
		{
			NewToken(types[i], curPos, curLine, curStr, prevch); 
			return 0;
		}
	}
	else
		if (!f_array[i](curCh))
		{
			NewToken(ttBadToken, curPos, curLine, errs[i], -52);
			return 0;
		}
	return 1;
}


void Scanner::Next()
{	
	try
	{
		pos = curPos;
		line = curLine;
		while (true)
		{
			prevch = curCh;
			GC();
			switch(state)
			{
			case stNewLex:
				if (!StateStNewLex())
					return;
				break;
			case stDigConsq:   
				if (!StateStDigConsq())
					return;
				break;
			case stIdent:  
				if (!StateStIdent())
					return;
				break;
			case stSlash: 
				if (curCh == '/')
					state = stCmntSlash;
				else
				{
					NewToken(TT["/"], curPos, curLine, "/", prevch);
					return;
				}
				break;
			case stRndBracket: 
				if (curCh == '*')
					state = stCmntRndBracket;
				else
				{
					NewToken(TT["("], curPos, curLine, "(", '(');
					return;
				}
				break;
			case stBrace:
				if (curCh == '}')
					state = stNewLex;
				else
					if (file.eof())
						throw Error("Unclosed Comment", curPos, curLine);
				break;
			case stLess:
				if (curCh == '=' || curCh == '>')
					NewToken(TT[string(curStr + curCh)], curPos, curLine, curStr + curCh, -52);
				else
					NewToken(TT[curStr], curPos, curLine, curStr, prevch);
				return;
			case stMore: case stColon: 
				if (curCh == '=')
					NewToken(TT[string(curStr + curCh)], curPos, curLine, curStr + curCh, -52);
				else
					NewToken(TT[curStr], curPos, curLine, curStr, prevch);
				return;
			case stRealLit: 
				if (file.eof() || IsSeparator(curCh) || IsOperation(curCh) || IsWhiteSpace(curCh))
				{
					NewToken(ttRealLit, curPos, curLine, curStr, prevch);
					return;
				}
				else
					if (!IsDigit(curCh))
						throw Error("Incorrect Real Literal", curPos, curLine);
				break;
			case stApostrophe:
				if (curCh == '\'')
					state = stClosedApostrophe;
				if (curCh == '\n' || file.eof())
					throw Error("Incorrect String Literal", curPos, curLine);
				break;
			case stHashSign:
				if (!IsDigit(curCh))
					throw Error("Incorrect String Literal", curPos, curLine);
				else
					state = stStrLitWithHash1Dig;
				break;
			case stDollar: case stPercent: case stAmpersand:
					if (!StateStDollarPercentAmpersand())
						return;
				break;
			case stRealELast:
				if (IsDigit(curCh))
					state = stRealLit;
				else
					 if(curCh == '-' || curCh == '+')
						 state = stRealLitSignLast;
					else
						throw Error("Incorrect Real Literal", curPos, curLine);
				break;
			case stRealLitWithoutE:
					if (!StateStRealLitWithoutE())
						return;
				break;
			case stCmntSlash:
				if (file.eof())
				{
					NewToken(ttEOF, pos, line, "", -52);
					return;
				}
				else
					if (curCh == '\n')
						state = stNewLex;
				break;
			case stCmntRndBracket:
				if (curCh == '*')
					state = stCmntRndPosToClose;
				else
					if (file.eof())
						throw Error("Unclosed Comment", curPos, curLine);
				break;
			case stStrLitWithHash0Dig:
				if (IsDigit(curCh))
					state = stStrLitWithHash1Dig;
				else
					throw Error("Incorrect String Literal", curPos, curLine);
				break;
			case stStrLitWithHash1Dig: case stStrLitWithHash2Dig: case stStrLitWithHash3Dig:
					if (!StateStStrLitWithHashDigs())
						return;
				break;
			case stCmntRndPosToClose:
				if (curCh == ')')
					state = stNewLex;
				break;
			case stClosedApostrophe:
					if (!StateStCloseApostrophe())
						return;
				break;
			case stRealLitSignLast:
				if (IsDigit(curCh))
					state = stRealLit;
				else
					throw Error("Incorrect Real Literal", curPos, curLine);
				break;
			case stRealLitPointLast:
					if (!StateStRealLitPointLast())
						return;
				break;
			case stPointBeforeEOF:
				NewToken(TT["."], curPos, curLine, ".", -52);
				state = stNewLex;
				return;
			case stPoint: 
				if (curCh != '.' || file.eof())
					NewToken(TT[curStr], curPos, curLine, curStr, prevch);
				else
					NewToken(TT[string(curStr + curCh)], curPos, curLine, curStr + curCh, -52);
				return;
			}
		curStr = curStr + curCh;
		ChangePosIndex();
	}
	} catch(Error lx)
	{
		NewToken(ttBadToken, lx.GetErrorPos(), lx.GetErrorLine(), lx.GetText(), -52);
	}
}
