#include "scanner.h"
#include <iostream>

const int TabSize = 4;

map<string, TokenType> TT;

bool IsAlpha(char ch) {	return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z';}
bool IsDigit(char ch) {	return ch >= '0' && ch <= '9';}
bool IsWhiteSpace(char ch) {	return ch >= 0 && ch <= 32;}
bool IsConcreteType(char ch, TokenType type)
{
	map<string, TokenType>::iterator it;
	string t = "";
	t += ch;
	it = TT.find(t);
	return it != TT.end() && it->second == type;
}

bool IsHexDigit(char ch){ return IsDigit(ch) || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f';}
bool IsBinDigit(char ch){ return ch == '1' || ch == '0';}
bool IsOctDigit(char ch){ return ch >= '0' && ch <= '7';}

bool IsKeyWord(string tmp)
{
	map<string, TokenType>::iterator it;
	transform(tmp.begin(), tmp.end(), tmp.begin(), toupper);
	it = TT.find(tmp);
	return it != TT.end() && it->second == ttKeyWord;
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
	case ttKeyWord:
		curToken = new KeyWord(p, l, val);
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
	case ttOperation:
		curToken = new Operation(p, l, val);
		break;
	case ttSeparator:
		curToken = new Separator(p, l, val);
		break;
	case ttEOF:
		curToken = new EndOfFile(p, l);
		break;
	case ttBadToken:
		curToken = new BadToken(p, l, val);
		break;
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
					if (IsConcreteType(curCh, ttSeparator))
						NewToken(ttSeparator, curPos, curLine, curStr + curCh, -52);
					else
						NewToken(ttOperation, curPos, curLine, curStr + curCh, -52);
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
			if (file.eof() || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation) || IsWhiteSpace(curCh))
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
	if (file.eof() || IsWhiteSpace(curCh) || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation))
	{
		if (IsKeyWord(curStr))
			NewToken(ttKeyWord, curPos, curLine, curStr, prevch);
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
		if (file.eof() || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation) || IsWhiteSpace(curCh))
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
		if (file.eof() || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation) || IsWhiteSpace(curCh))
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
	if (file.eof() || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation) || IsWhiteSpace(curCh))
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
					NewToken(ttOperation, curPos, curLine, "/", prevch);
					return;
				}
				break;
			case stRndBracket: 
				if (curCh == '*')
					state = stCmntRndBracket;
				else
				{
					NewToken(ttSeparator, curPos, curLine, "(", '(');
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
					NewToken(ttOperation, curPos, curLine, curStr + curCh, -52);
				else
					NewToken(ttOperation, curPos, curLine, curStr, prevch);
				return;
			case stMore: case stColon: 
				if (curCh == '=')
					NewToken(ttOperation, curPos, curLine, curStr + curCh, -52);
				else
					if (state == stMore)
						NewToken(ttOperation, curPos, curLine, curStr, prevch);
					else 
						NewToken(ttSeparator, curPos, curLine, curStr, prevch);
				return;
			case stRealLit: 
				if (file.eof() || IsConcreteType(curCh, ttSeparator) || IsConcreteType(curCh, ttOperation) || IsWhiteSpace(curCh))
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
				NewToken(ttSeparator, curPos, curLine, ".", -52);
				state = stNewLex;
				return;
			case stPoint: 
				if (curCh != '.' || file.eof())
					NewToken(ttSeparator, curPos, curLine, curStr, prevch);
				else
					NewToken(ttSeparator, curPos, curLine, curStr + curCh, -52);
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

void Scanner::FillTokenTypes()
{
	TT["AND"] = ttKeyWord; TT["ARRAY"] = ttKeyWord; TT["BEGIN"] = ttKeyWord; TT["CASE"] = ttKeyWord;
	TT["CONST"] = ttKeyWord; TT["DIV"] = ttKeyWord; TT["DO"] = ttKeyWord; TT["DOWNTO"] = ttKeyWord;
	TT["ELSE"] = ttKeyWord; TT["END"] = ttKeyWord; TT["FOR"] = ttKeyWord; TT["FUNCTION"] = ttKeyWord;
	TT["FORWARD"] = ttKeyWord; TT["GOTO"] = ttKeyWord; TT["IF"] = ttKeyWord; TT["IN"] = ttKeyWord;
	TT["MOD"] = ttKeyWord; TT["NIL"] = ttKeyWord; TT["NOT"] = ttKeyWord; TT["OF"] = ttKeyWord;
	TT["OR"] = ttKeyWord; TT["PROCEDURE"] = ttKeyWord; TT["PROGRAM"] = ttKeyWord; TT["RECORD"] = ttKeyWord;
	TT["REPEAT"] = ttKeyWord; TT["SET"] = ttKeyWord; TT["SHL"] = ttKeyWord; TT["SHR"] = ttKeyWord;
	TT["STRING"] = ttKeyWord; TT["THEN"] = ttKeyWord; TT["TO"] = ttKeyWord; TT["TYPE"] = ttKeyWord;
	TT["UNTIL"] = ttKeyWord; TT["USES"] = ttKeyWord; TT["VAR"] = ttKeyWord; TT["WHILE"] = ttKeyWord;
	TT["WITH"] = ttKeyWord; TT["XOR"] = ttKeyWord; TT["READLN"] = ttKeyWord; TT["WRITELN"] = ttKeyWord;
	TT["READ"] = ttKeyWord; TT["WRITE"] = ttKeyWord; TT["INTEGER"] = ttKeyWord; TT["BYTE"] = ttKeyWord;
	TT["SHORTINT"] = ttKeyWord; TT["WORD"] = ttKeyWord; TT["LONGINT"] = ttKeyWord; TT["INT64"] = ttKeyWord;
	TT["BOOLEAN"] = ttKeyWord; TT["CHAR"] = ttKeyWord; TT["SINGLE"] = ttKeyWord; TT["REAL"] = ttKeyWord;
	TT["DOUBLE"] = ttKeyWord; TT["EXTENDED"] = ttKeyWord; TT["COMP"] = ttKeyWord; 
	TT["."] = ttSeparator; TT[","] = ttSeparator; TT["("] = ttSeparator; TT[")"] = ttSeparator; TT["["] = ttSeparator;
	TT["]"] = ttSeparator; TT["{"] = ttSeparator; TT["}"] = ttSeparator; TT[";"] = ttSeparator; TT[":"] = ttSeparator;
	TT["+"] = ttOperation; TT["-"] = ttOperation; TT["/"] = ttOperation; TT["*"] = ttOperation; TT["="] = ttOperation;
	TT[">"] = ttOperation; TT["<"] = ttOperation; TT["<>"] = ttOperation; TT[":="] = ttOperation; TT[">="] = ttOperation;
	TT["<="] = ttOperation;  TT["^"] = ttOperation; TT["@"] = ttOperation; TT[".."] = ttSeparator; 
}
