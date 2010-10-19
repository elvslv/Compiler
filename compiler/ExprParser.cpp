#include "ExprParser.h"

int FillTreeBinOp(int i, int j, string Value, Expr* left, Expr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
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

Expr* ExprParser::ExprParse(int prior){
	if (prior == 1)
		return ExprParseFactor();
	Expr* res = ExprParse(prior - 1);
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		Expr* expr1 = ExprParse(prior - 1);
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new SimpleBinaryOp(str, res, expr1);
	}
	return res;
}

Expr* ExprParser::ExprParseNext(){
	scan.Next();
	Expr* expr1 = ExprParse(4);
	if (!expr1)
		throw Error("Lexem expected, EOF found", TokPos(), TokLine());
	return expr1;
}

Expr* ExprParser::ExprParseFunctionCall(Expr* res, int pos, int line){
	Expr* expr1 = ExprParseNext();
	list<Expr*> arg;
	while (TokVal() != ")"){
		if (TokVal() == ","){
			arg.push_back(expr1);
			expr1 = ExprParseNext();
		}
		else if (TokVal() != ")")
			throw Error("Unexpected lexem found", TokPos(), TokLine());
	}
	arg.push_back(expr1);
	res = new SimpleFunctionCall(res, arg);
	scan.Next();
	return res;
}

Expr* ExprParser::ExprParseArrayAccess(Expr* res, int pos, int line){
	Expr* expr1 = ExprParseNext();
	res = new SimpleArrayAccess("[]", res, expr1);
	while (TokVal() !=  "]"){
		if (TokVal() == ","){	
			expr1 = ExprParseNext();
			res = new SimpleArrayAccess("[]", res, expr1);
		}
		else if (TokVal() != "]")
			throw Error("Unexpected lexem found", TokPos(), TokLine());		
	}
	scan.Next();
	return res;
}

Expr* ExprParser::ExprParseRecordAccess(Expr* res, int pos, int line){
	Expr* expr1 = ExprParseNext();
	if (!expr1->LValue() && !expr1->IsFunction())
		throw Error("Invalid record access", pos, line);
	res = new SimpleRecordAccess(".", res, expr1);
	while (TokVal() ==  "."){
		expr1 = ExprParseNext();
		if (!expr1->LValue() && !expr1->IsFunction())
			throw Error("Invalid record access", pos, line);
		res = new SimpleRecordAccess(".", res, expr1);
	}
	return res;
}

Expr* ExprParser::ExprCallAccess(Expr* id, int pos, int line){
	Expr* res = id;
	isAccess = true;
	while (TokVal() == "(" || TokVal() == "[" || TokVal() == "."){
		if (TokVal() == "(")
			res = ExprParseFunctionCall(res, pos, line);
		else if (TokVal() == "[")
			res = ExprParseArrayAccess(res, pos, line);
		else if (TokVal() == "."){
			isRecord = true;
			res = ExprParseRecordAccess(res, pos, line);
		}
	}
	isAccess = false;
	isRecord = false;
	return res;
}

Expr* ExprParser::ExprParseFactor(){
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
		Expr* expr1 = ExprParseFactor();
		if (!expr1)
			throw Error("Lexem expected, EOF found", TokPos(), TokLine());
		res = new SimpleUnaryOp(str, expr1);
	}
	else if (TokType() == ttIdentifier){
		SimpleIdent* id = new SimpleIdent(TokVal());
		scan.Next();
		if (TokVal() == "[" || TokVal() == "(" || (TokVal() == "." && !isRecord))
			res = ExprCallAccess(id, pos, line);
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
		res = ExprParse(4);
		if (TokVal() != ")")
			throw Error("Unclosed bracket", pos, line);
		scan.Next();
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ExprCallAccess(res, pos, line);
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}
