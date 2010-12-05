#include "Parser.h"
#include "Symbols.h"
#include "Math.h"
static int id = 0;
static int labelCnt = 0;
static int forCnt = 0;
static int formatCnt = 0;
static map<string, string> formatStrings;
list<string> lblBeginBlock;
list<string> lblEndBlock;
AsmMem* mem(string name){ return new AsmMem("G_" + name); }
AsmMem* mem(string name, int off){ return new AsmMem("G_" + name, off); }
string MakeLabel(){
	return "label" + toString(labelCnt++);
}
int Variable::Size() { return symbol->Size(); }
int Const::Size() { return symbol->Size(); }
int ArrayAccess::Size() { return type->Size(); }
int RecordAccess::Size() { return type->Size(); }
int FunctionCall::Size() { return type->Size(); }
void Parser::Generate(){
	AsmProc* curProc = Asm->AddProc(".data", pData);
	SymTable* tbl = *lowerTable;
	for (SymMap::iterator it = tbl->GetTable()->begin(); it != tbl->GetTable()->end(); ++it){
		if (it->second->IsType())
			continue;
		if (!it->second->IsProc())
			it->second->GenDef(curProc);
	}
	curProc->Add(asmDd, new AsmMem("tmp"));
	for (map<string, string>::iterator it = formatStrings.begin(); it != formatStrings.end(); ++it)
		curProc->Add(asmDb, new AsmMem(it->first), new AsmImm(it->second));
	curProc = Asm->AddProc(".code", pCode);
	for (SymMap::iterator it = tbl->GetTable()->begin(); it != tbl->GetTable()->end(); ++it){
		if (!it->second->IsProc() || it->second->IsType())
			continue;
		curProc = Asm->AddProc(it->second->GetName(), pProc);
		it->second->Generate(curProc);
	}
	curProc = Asm->AddProc("main", pMain);
	curProc->Add(asmFInit);
	program->Generate(curProc);
}
void StmtAssign::Generate(AsmProc* Asm){
	if ((!right->IsReal() || right->IsArrayAccess() || right->IsRecordAccess()) && right->Size() > 8)
	{
		Asm->Add(asmMov, ecx, new AsmImmInt(right->Size() / 4));    ///
 		Asm->Add(asmMov, esi, new AsmOffset(mem(right->GetSymbol()->GetName())));
		Asm->Add(asmMov, edi, new AsmOffset(mem(left->GetSymbol()->GetName())));
		Asm->Add(asmRepMovsd);
	}
	else {
		left->GenLValue(Asm);
		right->Generate(Asm);
		/*if (right->IsReal()){
			Asm->Add(asmSub, esp, new AsmImmInt(4));
			Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
		}*/
		Asm->Add(asmPop, eax);
		Asm->Add(asmPop, ebx);
		Asm->Add(asmMov, new AsmMemByAddr(ebx, 0), eax);  ////
	}

}
string ChangeString(string tmp, int i){
	string str = tmp.substr(i * 4, 4);
	for (int j = 0; j < str.length(); ++j)
		if (str.substr(j, 1) == "'")
			str.insert(j++, "'");
	str = "'" + str + "'";
	return str;
}
string ChangeString(string tmp){
	string str = tmp;
	for (int j = 0; j < str.length(); ++j)
		if (str.substr(j, 1) == "'")
			str.insert(j++, "'");
	return str;
}
void StmtWrite::Generate(AsmProc* Asm){
	int offset = 0;
	if (params)
		for (list<NodeExpr*>::reverse_iterator it = params->rbegin(); it != params->rend(); ++it){
			if ((*it)->IsString() || (*it)->IsConst())
				continue;
			(*it)->Generate(Asm);
			if ((*it)->IsReal()){
				Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
				Asm->Add(asmSub, esp, new AsmImmInt(4));
				Asm->Add(asmFStp, new AsmPtr("qword ptr", new AsmMemByAddr(esp, 0)));
			}
			offset += (*it)->GetSymbol()->Size();
		}
	Asm->Add(asmPush, new AsmOffset(format));
	Asm->Add(asmPrintf);
	Asm->Add(asmAdd, esp, new AsmImmInt(offset + 4));
}
void SymVar::GenDef(AsmProc* Asm){
	if (GetType()->IsScalar())
		Asm->Add(asmDd, mem(name));
	else 
		Asm->Add(asmDup, new AsmDd(mem(name), new AsmImmInt(Size() / 4)), new AsmImmInt(0));
}
void SymVarConstInt::GenDef(AsmProc* Asm){
	Asm->Add(asmDd, mem(name), new AsmImmInt(GetValue()));
}
void SymVarConstReal::GenDef(AsmProc* Asm){
	Asm->Add(asmDd, mem(name), new AsmImmReal(GetValue()));
}
void SymVarConstString::GenDef(AsmProc* Asm){
	Asm->Add(asmDd, mem(name), new AsmImmString(GetValue()));
}
void SymVarConstInt::Generate(AsmProc* Asm){
	if (UnnamedSymb(this))
		Asm->Add(asmPush, new AsmImmInt(GetValue()));
	else
		Asm->Add(asmPush, mem(GetName()));
}
void SymVarConstReal::Generate(AsmProc* Asm){
	if (UnnamedSymb(this)){
		float tmp = GetValue();
		char str_tmp[10];
		sprintf(str_tmp, "%08xh", *((int*)&tmp));
		Asm->Add(asmMov, new AsmMem("tmp"), new AsmLabelOp(str_tmp));
		Asm->Add(asmFLd, new AsmMem("tmp"));
	}
	else
		Asm->Add(asmFLd, mem(GetName()));
	Asm->Add(asmSub, esp, new AsmImmInt(4));
	Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));

}
void SymVarConstString::Generate(AsmProc* Asm){
	Asm->Add(asmPush, new AsmImmString(GetValue()));
}
void Const::Generate(AsmProc* Asm){
	((SymVarConst*)symbol)->Generate(Asm);
}
void SymVarParamByValue::Generate(AsmProc* Asm){
	if (Size() > 4 && !GetType()->IsReal()){
		Asm->Add(asmSub, esp, new AsmImmInt(Size()));
		Asm->Add(asmMov, ecx, new AsmImmInt(Size() / 4));
		Asm->Add(asmMov, esi, new AsmOffset(mem(GetName())));
		Asm->Add(asmMov, edi, esp);
		Asm->Add(asmRepMovsd);
	}
	else if (GetType()->IsInt())
		Asm->Add(asmPush, new AsmMemByAddr(ebp, new AsmImmInt(GetTableOffset() - GetOffset() + 8)));
	else{
		Asm->Add(asmFLd, new AsmMemByAddr(ebp, new AsmImmInt(GetTableOffset() - GetOffset() + 8)));
		Asm->Add(asmSub, esp, new AsmImmInt(4));
		Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
	}
}
void SymVarParamByValue::GenLValue(AsmProc* Asm){
	Asm->Add(asmMov, eax, ebp);
	Asm->Add(asmAdd, eax, new AsmImmInt(GetTableOffset() - GetOffset() + 8));
	Asm->Add(asmPush, eax);
}
void SymVarParamByRef::Generate(AsmProc* Asm){
	Asm->Add(asmMov, eax, new AsmMemByAddr(ebp, new AsmImmInt(GetTableOffset() - GetOffset() + 8)));
	Asm->Add(asmPush, new AsmMemByAddr(eax, new AsmImmInt(0))); 
}
void SymVarParamByRef::GenLValue(AsmProc* Asm){
	Asm->Add(asmPush, new AsmMemByAddr(ebp, new AsmImmInt(GetTableOffset() - GetOffset() + 8))); 
}
void SymVarLocal::Generate(AsmProc* Asm){
	Asm->Add(asmPush, new AsmMemByAddr(ebp, new AsmImmInt(-GetOffset())));
}
void SymVarLocal::GenLValue(AsmProc* Asm){
	Asm->Add(asmMov, eax, ebp);
	Asm->Add(asmAdd, eax, new AsmImmInt(- GetOffset()));
	Asm->Add(asmPush, eax);
}
void SymVarGlobal::Generate(AsmProc* Asm){
	if (Size() > 4 && !GetType()->IsReal()){
		Asm->Add(asmSub, esp, new AsmImmInt(Size()));
		Asm->Add(asmMov, ecx, new AsmImmInt(Size() / 4));
		Asm->Add(asmMov, esi, new AsmOffset(mem(GetName())));
		Asm->Add(asmMov, edi, esp);
		Asm->Add(asmRepMovsd);
	} 
	else if (GetType()->IsReal()){
		Asm->Add(asmFLd, mem(GetName())); 
		Asm->Add(asmSub, esp, new AsmImmInt(4));
		Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
	}
	else
		Asm->Add(asmPush, mem(GetName())); 
}
void SymVarGlobal::GenLValue(AsmProc* Asm){
	Asm->Add(asmPush, new AsmOffset(mem(GetName()))); 
}
void Variable::Generate(AsmProc* Asm){ 
	((SymVar*)symbol)->Generate(Asm);
}
void Variable::GenLValue(AsmProc* Asm){ 
	((SymVar*)symbol)->GenLValue(Asm);
}
void ArrayAccess::Generate(AsmProc* Asm){
	left->GenLValue(Asm);
	Asm->Add(asmPop, ebx);
	right->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmMov, ecx, new AsmImmInt(symbol->Size()));
	Asm->Add(asmMul, ecx);
	if (symbol->Size() <= 4 || IsReal()){
		if (IsReal()){
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(ebx, eax)));
			Asm->Add(asmSub, esp, new AsmImmInt(4));
			Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
		}
		else
			Asm->Add(asmPush, new AsmMemByAddr(ebx, eax));
	}
	else{
		Asm->Add(asmSub, esp, new AsmImmInt(symbol->Size()));
		Asm->Add(asmMov, ecx, new AsmImmInt(symbol->Size() / 4));
		Asm->Add(asmMov, esi, ebx);
		Asm->Add(asmMov, edi, esp);
		Asm->Add(asmRepMovsd);
	}
}
void ArrayAccess::GenLValue(AsmProc* Asm){
	((Variable*)left)->GenLValue(Asm);
	Asm->Add(asmPop, ebx);
	right->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmMov, ecx, new AsmImmInt(symbol->Size()));
	Asm->Add(asmMul, ecx);
	Asm->Add(asmAdd, eax, ebx);
	Asm->Add(asmPush, eax);
}
void RecordAccess::Generate(AsmProc* Asm){
	((Variable*)left)->GenLValue(Asm);
	Asm->Add(asmPop, ebx);
	Asm->Add(asmAdd, ebx, new AsmImmInt(((SymVar*)right->GetSymbol())->GetOffset()));
	if (((SymVar*)right->GetSymbol())->Size() <= 4 || ((SymVar*)right->GetSymbol())->GetType()->IsReal()){
		if (IsReal()){
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(ebx, 0)));
			Asm->Add(asmSub, esp, new AsmImmInt(4));
			Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
		}
		else
			Asm->Add(asmPush, new AsmMemByAddr(ebx, 0));
	}
	else{
		Asm->Add(asmSub, esp, new AsmImmInt(((SymVar*)right->GetSymbol())->Size()));
		Asm->Add(asmMov, ecx, new AsmImmInt(((SymVar*)right->GetSymbol())->Size() / 4));
		Asm->Add(asmMov, esi, ebx);
		Asm->Add(asmMov, edi, esp);
		Asm->Add(asmRepMovsd);
	}
}
void RecordAccess::GenLValue(AsmProc* Asm){
	((Variable*)left)->GenLValue(Asm);
	Asm->Add(asmPop, ebx);
	Asm->Add(asmAdd, ebx, new AsmImmInt(((SymVar*)right->GetSymbol())->GetOffset()));
	Asm->Add(asmPush, ebx);
}
void BinaryOp::Generate(AsmProc* Asm){
	string op = symbol->GetName();
	right->Generate(Asm);
	left->Generate(Asm);
	if (IsInt()){
		Asm->Add(asmPop, eax);
		Asm->Add(asmPop, ebx);
		if (op == "DIV"){
			Asm->Add(asmXor, edx, edx);
			Asm->Add(cmdTypes[op], ebx);
		}
		else if (op == "MOD"){  
			Asm->Add(asmXor, edx, edx);
			Asm->Add(asmIDiv, ebx);
			Asm->Add(asmMov, eax, edx);
		}
		else if (IsLogicOperator(op)){
			Asm->Add(asmCmp, eax, ebx);
			Asm->Add(asmMov, eax, new AsmImmInt(0));
			Asm->Add(asmMov, ecx, new AsmImmInt(1));
			if (op == "=")
				Asm->Add(asmCMovE , eax, ecx);
			else if (op == "<>")
				Asm->Add(asmCMovNE , eax, ecx);
			else if (op ==  "<")
				Asm->Add(asmCMovL , eax, ecx);
			else if (op == ">")
				Asm->Add(asmCMovG , eax, ecx);
			else if (op == "<=")
				Asm->Add(asmCMovLE , eax, ecx);
			else if (op == ">=")
				Asm->Add(asmCMovGE , eax, ecx);
		}
		else 
			Asm->Add(cmdTypes[op], eax, ebx);
		Asm->Add(asmPush, eax);
	} 
	else {
		if (IsLogicOperator(op)){
			Asm->Add(asmFLd1);
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmAdd, esp, new AsmImmInt(4));
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmAdd, esp, new AsmImmInt(4));
			Asm->Add(asmFComI, new AsmReg("st(0)"), new AsmReg("st(1)"));
			Asm->Add(asmFLdZ);
			if (op == "=")
				Asm->Add(asmFCMovE, new AsmReg("st(0)"), new AsmReg("st(2)"));
			else if (op == "<>")
				Asm->Add(asmFCMovNE, new AsmReg("st(0)"), new AsmReg("st(2)"));
			else if (op ==  "<")
				Asm->Add(asmFCMovB, new AsmReg("st(0)"), new AsmReg("st(2)"));
			else if (op == ">")
				Asm->Add(asmFCMovNBE, new AsmReg("st(0)"), new AsmReg("st(2)"));
			else if (op == "<=")
				Asm->Add(asmFCMovBE, new AsmReg("st(0)"), new AsmReg("st(2)"));
			else if (op == ">=")
				Asm->Add(asmFCMovNB, new AsmReg("st(0)"), new AsmReg("st(2)"));
		}
		else
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmAdd, esp, new AsmImmInt(4));
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmAdd, esp, new AsmImmInt(4));
			Asm->Add(cmdTypes["f" + op], new AsmReg("st(0)"), new AsmReg("st(1)"));
			Asm->Add(asmSub, esp, new AsmImmInt(4));
			Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmFInit);
	}
	
}
void UnaryOp::Generate(AsmProc* Asm){
	string op = symbol->GetName();
	child->Generate(Asm);
	if (op == "+")
		return;
	if (IsReal()){
		if (op == "ItoR" && child->IsInt()){  ////?????
			Asm->Add(asmPop, eax);
			Asm->Add(asmMov, new AsmMem("tmp"), eax);
			Asm->Add(asmFIld, new AsmMem("tmp"));
		}
		if (op == "-"){
			Asm->Add(asmFLd, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
			Asm->Add(asmAdd, esp, new AsmImmInt(4));
			Asm->Add(asmFChs);
		}
		Asm->Add(asmSub, esp, new AsmImmInt(4));
		Asm->Add(asmFStp, new AsmPtr("dword ptr", new AsmMemByAddr(esp, 0)));
		Asm->Add(asmFInit);
	}
	else{
		Asm->Add(asmPop, eax);
		Asm->Add(cmdTypes[op], eax);
		Asm->Add(asmPush, eax);
	}
}
void FunctionCall::Generate(AsmProc* Asm){
	if (IsFunction())
		Asm->Add(asmSub, esp, new AsmImmInt(symbol->Size()));
	for (list<NodeExpr*>::iterator it = args.begin(); it != args.end(); ++it)
		(*it)->Generate(Asm);
	Asm->Add(asmCall, new AsmProcName(symbol->GetName()));
}
void SymProc::Generate(AsmProc* Asm){
	Asm->Add(asmPush, ebp);
	Asm->Add(asmMov, ebp, esp);
	Asm->Add(asmSub, esp, new AsmImmInt(locals->GetOffset()));
	body->Generate(Asm);
	Asm->Add(asmMov, esp, ebp);
	Asm->Add(asmPop, ebp);
	Asm->Add(asmRet, new AsmImmInt(args->GetTable()->size()));
}
void StmtRepeat::Generate(AsmProc* Asm){
	string lblBegin = MakeLabel();
	string lblEnd = MakeLabel();
	lblBeginBlock.push_back(lblBegin);
	lblEndBlock.push_back(lblEnd);
	Asm->AddLabel(lblBegin);
	body->Generate(Asm);
	expr->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmTest, eax, eax);
	Asm->Add(asmJZ, new AsmLabelOp(lblBegin));
	Asm->AddLabel(lblEnd);
	lblBeginBlock.pop_back();
	lblEndBlock.pop_back();
}
void StmtWhile::Generate(AsmProc* Asm){
	string lblBegin = MakeLabel();
	string lblEnd = MakeLabel();
	lblBeginBlock.push_back(lblBegin);
	lblEndBlock.push_back(lblEnd);
	Asm->AddLabel(lblBegin);
	expr->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmTest, eax, eax);
	Asm->Add(asmJZ, new AsmLabelOp(lblEnd));
	body->Generate(Asm);
	Asm->Add(asmJmp, new AsmLabelOp(lblBegin));	
	Asm->AddLabel(lblEnd);
	lblBeginBlock.pop_back();
	lblEndBlock.pop_back();
}
void StmtFor::Generate(AsmProc *Asm){
	((SymVar*)var)->GenLValue(Asm);
	string lblBegin = MakeLabel();
	string lblEnd = MakeLabel();
	lblBeginBlock.push_back(lblBegin);
	lblEndBlock.push_back(lblEnd);
	initVal->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmPop, ebx);
	Asm->Add(asmMov, new AsmMemByAddr(ebx, 0), eax);
	finitVal->Generate(Asm);
	Asm->AddLabel(lblBegin);
	((SymVar*)var)->Generate(Asm);
	Asm->Add(asmPop, eax); 
	Asm->Add(asmPop, edx); 
	Asm->Add(asmCmp, eax, edx);
	Asm->Add(to ? asmJG : asmJL, new AsmLabelOp(lblEnd));
	Asm->Add(asmPush, edx);
	body->Generate(Asm);
	((SymVar*)var)->GenLValue(Asm);
	Asm->Add(asmPop, ebx);
	Asm->Add(to ? asmAdd : asmSub, new AsmPtr("dword ptr", new AsmMemByAddr(ebx, 0)), new AsmImmInt(1));
	//Asm->Add(asmPop, edx);
	Asm->Add(asmJmp, new AsmLabelOp(lblBegin));
	Asm->AddLabel(lblEnd);
	lblBeginBlock.pop_back();
	lblEndBlock.pop_back();
}
void StmtIf::Generate(AsmProc* Asm){
	string lblAfterIf = MakeLabel();
	string lblElse = MakeLabel();
	string lblBeg = MakeLabel();
	Asm->AddLabel(lblBeg);
	expr->Generate(Asm);
	Asm->Add(asmPop, eax);
	Asm->Add(asmTest, eax, eax);
	if (second)
		Asm->Add(asmJZ, new AsmLabelOp(lblElse));
	else
		Asm->Add(asmJZ, new AsmLabelOp(lblAfterIf));
	first->Generate(Asm);
	Asm->Add(asmJmp, new AsmLabelOp(lblAfterIf));
	if (second){
		Asm->AddLabel(lblElse);
		second->Generate(Asm);
	}
	Asm->AddLabel(lblAfterIf);
}
void StmtBreak::Generate(AsmProc* Asm){
	string str = *(lblEndBlock.rbegin());
	Asm->Add(asmJmp, new AsmLabelOp(str));
}
void StmtContinue::Generate(AsmProc* Asm){
	string str = *(lblBeginBlock.rbegin());
	Asm->Add(asmJmp, new AsmLabelOp(str));
}
void Parser::PrintTable(){
	SymTable* tbl = *lowerTable;
	for (SymMap::iterator it = tbl->GetTable()->begin(); it != tbl->GetTable()->end(); ++it){
		if (it->second->GetName() == "INTEGER" || it->second->GetName() == "REAL" || it->second->GetName() == "STRING")
			continue;
		it->second->Print(os, true);
		if (it->second->IsConst())
			os << ";";
		os << "\n";
	}
}
bool UnnamedType(SymType* type){ return type && type->GetName()[0] >= '0' && type->GetName()[0] <= '9'; }
bool UnnamedSymb(Symbol* symb){ return symb->GetName()[0] >= '0' && symb->GetName()[0] <= '9'; }
int StmtBlock::FillTree(int i, int j){
	int j1 = j;
	j1 = FillTreeOp(i, j, "begin");
	for (list<Statement*>::iterator it = statements.begin(); it != statements.end(); ++it)
		j1 = PaintBranch(i, j1, j1, j1 + (*it)->FillTree(i + 4, j1), true);
	j1 += FillTreeIdentConst(i, j1, "end ");
	return j1 - j;
}
int StmtIf::FillTree(int i, int j){ 
	int j1 = j;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "if"), true);
	j = PaintBranch(i, j, j, j + expr->FillTree(i + 4, j), false);
	j = FillTreeOp(i, j, "then");
	if (second){
		j = PaintBranch(i, j, j, j + first->FillTree(i + 4, j), false);
		j = FillTreeOp(i, j, "else");
		j += second->FillTree(i + 4, j);
	}
	else
		j +=  first->FillTree(i + 4, j);
	return j - j1;
}
int FillTreeWhileRepeat(int i, int j, string s1, string s2, SyntaxNode* node1, SyntaxNode* node2){
	int j1 = j;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, s1), true);
	j = PaintBranch(i, j, j, j + node1->FillTree(i + 4, j), false);
	j = FillTreeOp(i, j, s2);
	j += node2->FillTree(i + 4, j);
	return j - j1;
}
int StmtFor::FillTree(int i, int j){
	int j1 = j;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i, j, "for"), true);
	Variable* v = new Variable(var, 0, 0);
	StmtAssign* assign = new StmtAssign(v, initVal);
	j = PaintBranch(i, j, j, j + assign->FillTree(i + 4, j), false);
	j = to ? FillTreeOp(i, j, "to") : FillTreeOp(i, j, "downto");
	j = PaintBranch(i, j, j, j + finitVal->FillTree(i + 4, j), false);
	j = FillTreeOp(i, j, "do");
	j += body->FillTree(i + 4, j);
	delete v;
	delete assign;
	return j - j1;
}
int StmtProcedure::FillTree(int i, int j){
	int j1 = j;
	FunctionCall* func = new FunctionCall(proc, 0, 0, *args);
	j += func->FillTree(i, j);
	delete func;
	return j - j1;
}
void Parser::ParseMainBlock() { 
	if (TokType() == ttEOF)
		return;
	program = ParseBlock(tableStack->begin(), true);
}
void PrintArgs(ostream& os, SymTable* args){
	if (args->GetTable()->empty())
		return;
	os << "(";  
	for (SymMap::iterator it = args->GetTable()->begin(); it != args->GetTable()->end(); ++it)
		(*it->second).Print(os, false);
	os << ")";
}
void SymProc::PrintBody(ostream& os) { 
	os << "\n";
	if (!body)
		return;
	body->Print(os, 0);
}
void SymProc::Print(ostream& os, bool f) { 
	os << "procedure "<< name;
	PrintArgs(os, args);
	os << ";";
	PrintBody(os);
}
void SymFunc::Print(ostream& os, bool f) { 
	os << "function " << name;
	PrintArgs(os, args);
	os << ": ";
	type->Print(os, false);
	os << ";";
	PrintBody(os);
}
string NodeExpr::GetValue() { return symbol->GetName(); }
bool SymIsInt(Symbol* type) { return type && ((SymType*)type)->IsInt(); }
bool SymIsReal(Symbol* type) { return type && ((SymType*)type)->IsReal(); }
bool Variable::IsInt() { return symbol->GetType()->IsInt(); }
bool Variable::IsReal() { return symbol->GetType()->IsReal(); }
bool Const::IsInt() { return symbol->GetType()->IsInt(); }
bool Const::IsReal() { return symbol->GetType()->IsReal(); }
bool Const::IsString() { return ((SymVarConst*)symbol)->IsString();} 
bool BinaryOp::IsInt() {return right->IsInt() && left->IsInt() && IsIntOperator(GetValue()) || 
						 ((SymType*)right->GetType())->IsScalar() && ((SymType*)right->GetType())->IsScalar() && IsLogicOperator(GetValue()); }
bool BinaryOp::IsReal() {return right->IsReal() || left->IsReal() || GetValue() == "/"; }
bool ArrayAccess::IsInt() { return SymIsInt(GetType()); }
bool ArrayAccess::IsReal() { return SymIsReal(GetType());  }
bool RecordAccess::IsInt() { return SymIsInt(GetType());  }
bool RecordAccess::IsReal() { return SymIsReal(GetType());  }
bool FunctionCall::IsInt() { return SymIsInt(GetType());  }
bool FunctionCall::IsReal() { return SymIsReal(GetType()); }
bool UnaryOp::IsInt() {return symbol->GetName() != "ItoR" && child->IsInt() && IsIntOperator(GetValue()); }
bool UnaryOp::IsReal() { return symbol->GetName() == "ItoR" || child->IsReal(); }
bool EqTypes(SymType* t1, SymType* t2){ return t1->GetSourceType() == t2->GetSourceType(); }
bool IToR(SymType* t1, SymType* t2){ return t1->GetSourceType()->GetName() == "REAL" && 
											t2->GetSourceType()->GetName() == "INTEGER"; }
void FillMaps(){
	priority["NOT"] = 1; 
	priority["*"] = 2; priority["/"] = 2; priority["DIV"] = 2; priority["MOD"] = 2; 
	priority["AND"] = 2; priority["SHL"] = 2; priority["SHR"] = 2; 
	priority["-"] = 3; priority["+"] = 3; priority["OR"] = 3; priority["XOR"] = 3;
	priority["="] = 4; priority["<>"] = 4; priority["<"] = 4;	priority["<="] = 4; priority[">"] = 4; 
	priority[">="] = 4;
	priority[":="] = 5;
	cmdNames[asmMov] = "mov"; cmdNames[asmAdd] = "add"; cmdNames[asmSub] = "sub";
	cmdNames[asmDiv] = "div"; cmdNames[asmMul] = "mul"; cmdNames[asmIDiv] = "idiv"; 
	cmdNames[asmAnd] = "and"; cmdNames[asmOr] = "or"; cmdNames[asmXor] = "xor"; 
	cmdNames[asmNot] = "not"; cmdNames[asmShl] = "shl"; cmdNames[asmShr] = "shr"; 
	cmdTypes["+"] = asmAdd; cmdTypes["-"] = asmSub; cmdTypes["DIV"] = asmIDiv; cmdTypes["*"] = asmIMul;
	cmdTypes["AND"] = asmAnd; cmdTypes["OR"] = asmOr; cmdTypes["XOR"] = asmXor; 
	cmdTypes["NOT"] = asmNot; cmdTypes["SHL"] = asmShl; cmdTypes["SHR"] = asmShr; 
	cmdTypes["f+"] = asmFAdd; cmdTypes["f-"] = asmFSub; cmdTypes["f/"] = asmFDiv; cmdTypes["f*"] = asmFMul;
	cmdNames[asmIMul] = "imul"; cmdNames[asmPush] = "push"; cmdNames[asmPop] = "pop"; 
	cmdNames[asmCmp] = "cmp"; cmdNames[asmJmp] = "jmp"; cmdNames[asmDw] = "dw"; cmdNames[asmDd] = "dd"; 
	cmdNames[asmDb] = "db"; cmdNames[asmUnknown] = "?";cmdNames[asmCode] = ".code";
	cmdNames[asmData] = ".data"; cmdNames[asmScanf] = "crt_scanf"; cmdNames[asmPrintf] = "crt_printf";
	cmdNames[asmCall] = "call"; cmdNames[asmDwtoa] = "dwtoa"; cmdNames[asmStdout] = "StdOut";
	cmdNames[asmNeg] = "neg"; cmdNames[asmFLd] = "fld"; cmdNames[asmFStp] = "fstp"; cmdNames[asmFAdd] = "fadd";
	cmdNames[asmFSub] = "fsub"; cmdNames[asmFMul] = "fmul"; cmdNames[asmFDiv] = "fdiv"; cmdNames[asmFChs] = "fchs"; 
	cmdNames[asmRepMovsd] = "rep movsd";  cmdNames[asmPushfd] = "pushfd";	cmdNames[asmFLd1] = "fld1";
	cmdNames[asmCMovE] = "cmove"; cmdNames[asmCMovNE] = "cmovne"; cmdNames[asmCMovL] = "cmovl"; 
	cmdNames[asmCMovLE] = "cmovle"; cmdNames[asmCMovG] = "cmovg"; cmdNames[asmCMovGE] = "cmovge";
	cmdNames[asmFComI] = "fcomi"; cmdNames[asmFCMovE] = "fcmove"; cmdNames[asmFCMovNE] = "fcmovne";
	cmdNames[asmFCMovB] = "fcmovb"; cmdNames[asmFCMovNBE] = "fcmovnbe"; cmdNames[asmFCMovBE] = "fcmovbe";
	cmdNames[asmFCMovNB] = "fcmovnb"; cmdNames[asmTest] = "test";cmdNames[asmJZ] = "jz"; cmdNames[asmJNZ] = "jnz";
	cmdNames[asmJG] = "jg"; cmdNames[asmJL] = "jl"; cmdNames[asmFInit] = "finit"; cmdNames[asmFIld] = "fild";
	cmdNames[asmFLdZ] = "fldz";
}
int FindOpPrior(string str){
	map<string, int>::iterator it;
	transform(str.begin(), str.end(), str.begin(), toupper);
	it = priority.find(str);
	return (it != priority.end()) ? it->second : 0;
}
Parser::Parser(Scanner& sc, ostream& o): scan(sc), os(o){
	SymTable* table = new SymTable();
	tableStack = new SymTableStack(); 
	FillMaps();
	scan.Next();
	isRecord = false;
	isAccess = false;
	nextScan = true;
	idFound = true;
	SymTypeInteger* i = new SymTypeInteger("INTEGER");
	SymTypeReal* f = new SymTypeReal("REAL");
	SymType* s = new SymTypeString("STRING");
	table->insert("INTEGER", i);
	table->insert("REAL", f);
	table->insert("STRING", s);
	tableStack->push_back(table);
	lowerTable = tableStack->begin();
	curIdent = "";
	Asm = new AsmCode();
}
void Parser::TryError(bool f, string mes) {
	if (f)
		throw Error(mes, TokPos(), TokLine());
}
void Parser::TryError(bool f, string mes, int pos, int line) {
	if (f)
		throw Error(mes, pos, line);
}
void CheckIsInt(NodeExpr* expr, int i, int j){
	if (!expr->IsInt())
		throw Error("Integer index expected", i, j);
}
void Parser::CheckEof(){
	scan.Next();
	TryError(TokType() == ttEOF, "Unexpected end of file");
}
void Parser::ParseDecl(SymStIt curTable, bool main){
	TokenType tt = TokType();
	while (tt == ttType || tt == ttVar || tt == ttConst || tt == ttProcedure || tt == ttFunction){
		scan.Next();
		switch(tt){
		case ttType:
			ParseTypeConstBlock(curTable, ttType);
			break;
		case ttVar:
			ParseVarRecordBlock(curTable, ttVar);
			break;
		case ttConst:
			ParseTypeConstBlock(curTable, ttConst);
			break;
		case ttProcedure: case ttFunction:
			TryError(!main, "Embedded procedures are not allowed");
			tt == ttProcedure ? ParseProcedure(curTable, true, false) : ParseProcedure(curTable, true, true);
			break;
		}
		tt = TokType();
	}
}
SymMap::iterator Parser::FindS(string s, SymStIt curTable, SymStIt end){
	SymStIt curIt = curTable;
	SymMap::iterator it;
	idFound = true;
	bool f = false;
	while(true) {
		it = (**curIt).GetTable()->find(UpCase(s));
		if (it != (*curIt)->GetTable()->end()){
			f = true;
			break;
		}
		else if (curIt != end)
				--curIt;
		else
			break;
	};
	if (!f)
		idFound = false;
	return it;
}
SymMap::iterator Parser::FindS(string s, SymTable* curTable){	return curTable->GetTable()->find(UpCase(s)); }
Statement* Parser::ParseStatement(SymStIt curTable){
	Statement* res = NULL;
	TryError(TokType() != ttIdentifier && !IsKeyWord(TokType()), "Unexpected lexem found");
	if (IsKeyWord(TokType())){
		switch(TokType()){
			case ttIf:
				res = ParseIf(curTable);
				break;
			case ttWhile:
				res = ParseWhile(curTable);
				break;
			case ttRepeat:
				res = ParseRepeat(curTable);
				break;
			case ttFor:
				res = ParseFor(curTable);
				break;
			case ttBreak:
				res = new StmtBreak();
				scan.Next();
				break;
			case ttContinue:
				res = new StmtContinue();
				scan.Next();
				break;
			case ttBegin:
				res = ParseBlock(curTable, false);
				break;
			case ttWrite: case ttWriteln:
				res = ParseWrite(curTable, TokType() == ttWriteln);
				break;
			default:
				throw Error("Unexpected keyword found", TokPos(), TokLine());				
		}
	} 
	else{
		SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		TryError(!idFound, "Unknown identifier");
		TryError(!(it->second->IsProc() || it->second->IsFunc() || it->second->IsVar()), "Invalid lexem");
		res = it->second->IsVar() ? (Statement*)ParseAssignment(curTable) : (Statement*)ParseProcedure(curTable);
	}
	return res;
}
StmtBlock* Parser::ParseBlock(SymStIt curTable, bool main){
	list<Statement*> stmts;
	StmtBlock* res = NULL;
	if (TokType() == ttBegin){
		scan.Next();
		while (TokType() != ttEnd){
			stmts.push_back(ParseStatement(curTable));
			if (TokType() != ttSemi){
				TryError(TokType() != ttEnd, "; expected, but " + TokVal() + " found");
				break;
			}
			else
				scan.Next();
		}
		scan.Next();
	}
	else
		stmts.push_back(ParseStatement(curTable));
	TryError(main && TokType() != ttDot, ". expected, but " + TokVal() + " found");
	res = new StmtBlock(stmts);
	return res;
}
StmtWhile* Parser::ParseWhile(SymStIt curTable){
	NodeExpr* expr = ParseNext(curTable);
	TryError(!expr->IsInt(), "Condition must be integer");
	Statement* body = NULL;
	TryError(TokType() != ttDo, "Do expected, but " + TokVal() + " found");
	scan.Next();
	body = ParseBlock(curTable, false);
	return new StmtWhile(expr, body);
}
StmtRepeat* Parser::ParseRepeat(SymStIt curTable){
	list<Statement*> stmts;
	StmtBlock* body = NULL;
	scan.Next();
	while (TokType() != ttUntil){
		bool b = TokType() == ttBegin;
		b ? stmts.push_back(ParseBlock(curTable, false)) : stmts.push_back(ParseStatement(curTable));
		if (TokType() != ttSemi){
			string str = b ? "Until" : ";";
			TryError(TokType() != ttUntil, str + " expected, but " + TokVal() + " found");
			if (!b) 
				break;
		}
		else
			scan.Next();
	}
	body = new StmtBlock(stmts);
	NodeExpr* expr = ParseNext(curTable);
	TryError(!expr->IsInt(), "Condition must be integer");
	return new StmtRepeat(expr, body);
}
StmtWrite* Parser::ParseWrite(SymStIt curTable, bool w){
	scan.Next();
	string format = "";
	string formatName = "format" + toString(formatCnt++);
	if (TokType() != ttLeftParentheses){
		format = "\'\', 0";
		formatStrings[formatName] = format;
		return new StmtWrite(w, formatName);
	}
	map<string, string>::iterator it;
	list<NodeExpr*>* params = new list<NodeExpr*>();
	while (TokType() != ttRightParentheses){
		NodeExpr* expr = ParseNext(curTable);
		if (TokType() != ttPoint)
			TryError(TokType() != ttRightParentheses, ") expected, but " + TokVal() + " found");
		params->push_back(expr);
		if (expr->IsString() || expr->IsConst()){
			string tmp = ((SymVarConst*)expr->GetSymbol())->ToString();
			if (expr->IsString()){
				tmp = tmp.substr(1, tmp.length() - 2);
				tmp = ChangeString(tmp);
			}
			format += tmp;
		}
		else if (expr->IsInt())
			format += "%d";
		else if (expr->IsReal())
			format += "%f";
	}
	format = "'" + format + "'";
	if (w)
		format += ", 10";
	format += ", 0";
	formatStrings[formatName] = format;
	scan.Next();
	return new StmtWrite(w, params, formatName);
}
StmtProcedure* Parser::ParseProcedure(SymStIt curTable){
	NodeExpr* expr =  ParseSimple(curTable, 5);
	if (!expr->IsFunction()){
		TryError(expr->IsBinaryOp() && expr->GetSymbol()->GetName() == ":=", "Left part of assignment must be lvalue", expr->GetPos(), expr->GetLine());
		throw Error("Invalid expression", TokPos(), TokLine());
	}
	SymProc* proc = (SymProc*)expr->GetSymbol();
	return new StmtProcedure(proc, ((FunctionCall*)expr)->GetArgs());
}
StmtIf* Parser::ParseIf(SymStIt curTable){
	scan.Next();
	NodeExpr* expr = ParseSimple(curTable, 5);
	TryError(!expr->IsInt(), "Condition must be integer");
	TryError(TokType() != ttThen, "Then expected, but " + TokVal() + " found");
	scan.Next();
	Statement* first = ParseBlock(curTable, false);
	Statement* second = NULL;
	if (TokType() == ttElse){
		scan.Next();
		second = ParseBlock(curTable, false);
	}
	return new StmtIf(expr, first, second);
}
StmtFor* Parser::ParseFor(SymStIt curTable){
	bool to = true;
	scan.Next();
	TryError(TokType() != ttIdentifier, "Identifier expected");
	SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	TryError(!idFound, "Unknown identifier");
	idFound = true;
	Symbol* symb = it->second;
	TryError(!symb->IsVar() || symb->IsConst() || symb->GetType()->IsArray() || symb->GetType()->IsRecord(), 
				"Illegal counter variable");
	TryError(!symb->GetType()->IsInt(), "Loop variable must be integer");
	scan.Next();
	TryError(TokType() != ttAssign, "Invalid identifier initialization");
	NodeExpr* init = ParseNext(curTable);
	TryError(!init->IsInt(), "Init expression must be integer");
	TokType() == ttDownto ? to = false : TryError(TokType() != ttTo,"To expected, but " + TokVal() + " found");
	NodeExpr* finit = ParseNext(curTable);
	TryError(!finit->IsInt(), "Final expression must be integer");
	TryError(TokType() != ttDo, "Do expected, but " + TokVal() + " found");
	scan.Next();
	StmtBlock* body = ParseBlock(curTable, false);
	return new StmtFor((SymVar*)it->second, init, finit, body, to);
}
StmtAssign* Parser::ParseAssignment(SymStIt curTable){
	NodeExpr* expr = ParseSimple(curTable, 5);
	BinaryOp* res = new BinaryOp(expr->GetSymbol(), expr->GetPos(), expr->GetLine(), expr, NULL);
	TryError(expr->GetValue() != ":=", "Illegal expression");
	res = (BinaryOp*)expr;
	NodeExpr* lExpr = expr;
	while(lExpr->IsBinaryOp())
		lExpr = ((BinaryOp*)lExpr)->GetLeft();
	TryError(!res->GetLeft()->LValue(), "Left part of assignment must be lvalue", lExpr->GetPos(), lExpr->GetLine());
	Symbol* t1 = res->GetLeft()->GetType();
	Symbol* t2 = res->GetRight()->GetType();
	TryError(!(t1 && t2 && (EqTypes((SymType*)t1, (SymType*)t2) || IToR((SymType*)t1,(SymType*) t2))), 
				"Incompatible types in assignment", lExpr->GetPos(), lExpr->GetLine());
	return new StmtAssign(res->GetLeft(), res->GetRight());
}
string Parser::CheckCurTok(SymStIt curTable, string blockName, SymTable* tbl){
	TryError(TokType() != ttIdentifier, "Invalid lexem in " + blockName + " block");
	string s = TokVal();
	SymMap::iterator it = FindS(TokVal(), tbl);
	TryError(it != tbl->GetTable()->end(), "Identifier already declared");
	idFound = true;
	CheckEof();
	return s;
}
SymTable* Parser::GetArgs(SymStIt curTable, string name, list<string>** arg_names){
	list<string> args;
	SymTable* argsTable = new SymTable();
	SymType* type = NULL;
	int args_num = 0;
	if (TokType() == ttLeftParentheses){
		while (TokType() != ttRightParentheses){
			CheckEof();
			TokenType tt = TokType();
			if (tt == ttVar)
				CheckEof();
			while (TokType() != ttSemi && TokType() != ttRightParentheses){
				string par = CheckCurTok(curTable, "function", argsTable);
				args.push_back(par);
				(*arg_names)->push_back(par);
				++args_num;
				if (TokType() == ttPoint)
					CheckEof();
				else if (TokType() == ttColon){
					CheckEof();
					type = (SymType*)ParseType(curTable, false);
					TryError(UnnamedType(type), "Invalid type");
					CheckEof();
					break;
				}
				else
					TryError(TokType() != ttRightParentheses && TokType() != ttSemi, "Invalid lexem in function declaration");
			}
			int offset = 0;
			for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
				SymMap::iterator it1 = FindS(*it, argsTable);
				TryError(it1 != argsTable->GetTable()->end() || *it == name, "Identifier already declared");
				argsTable->insert(*it, (tt == ttVar) ? (SymVarParam*)new SymVarParamByRef(*it, type, offset, argsTable) 
									: (SymVarParam*)new SymVarParamByValue(*it, type, offset, argsTable));
				offset += it1->second->Size();
			}
			args.clear();			
		}
		scan.Next();
	}
	return argsTable;
}
Symbol* Parser::ParseProcedure(SymStIt curTable, bool newProc, bool func){
	SymProc* res = NULL;
	int offset = 0;
	string ident = TokVal();
	if (newProc)
		string ident = func ? CheckCurTok(curTable, "function", *curTable) : 
							CheckCurTok(curTable, "procedure", *curTable);
	list<string>* arg_names = new list<string>;
	SymTable* argTable = GetArgs(curTable, ident, &arg_names);
	SymTable* locals = new SymTable();
	SymType* type = NULL;
	if (func){
		TryError(TokType() != ttColon, "Function type expected");
		CheckEof();
		type = (SymType*)ParseType(curTable, false);
		CheckEof();
	}
	tableStack->push_back(argTable);
	tableStack->push_back(locals);
	if (TokType() != ttSemi)
		TryError(TokType() == ttEOF, "Unexpected end of file");
	TryError(TokType() != ttSemi, "; expected end of file, but " + TokVal() + " found");
	CheckEof();
	StmtBlock* body = NULL;
	res = func ? (SymProc*)new SymFunc(ident, argTable, locals, arg_names, body, type) : new SymProc(ident, argTable, locals, arg_names, body);
	if (func){
		SymVarLocal* var = new SymVarLocal("RESULT", type, locals->GetOffset(), locals);
		locals->insert("RESULT", var);
	}
	if (newProc)
		(*curTable)->insert(ident, res);
	if (TokType() != ttForward){
		SymStIt it;
		for (SymStIt i = tableStack->begin(); i != tableStack->end(); ++i)
			it = i;
		ParseDecl(it, false);
		body = ParseBlock(it, false);
		res->SetBody(body);
	}else {
		CheckEof();
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
	}
	tableStack->pop_back();
	tableStack->pop_back();
	scan.Next();
	return res;
}
SymTable* Parser::ParseVarRecordBlock(SymStIt curTable, TokenType tt){ 
	list<string> idents;
	SymTable* curTbl = NULL;
	curTbl = (tt == ttVar) ? *curTable : new SymTable();
	string b_name = (tt == ttVar) ? "var" : "record";
	while(true){
		string s = CheckCurTok(curTable, b_name, curTbl);
		idents.push_back(s);
		if (TokType() == ttPoint)
			CheckEof();
		else {
			TryError(TokType() != ttColon, "Invalid lexem in " + b_name + " block");
			CheckEof();
			SymType* type = (SymType*)ParseType(curTable, false);
			CheckEof();
			if (tt == ttVar && type->GetName() == "")
				type->SetName(toString(id++));
			for (list<string>::iterator it1 = idents.begin(); it1 != idents.end(); ++it1){
				SymMap::iterator it = FindS(*it1, curTbl);
				TryError(it != curTbl->GetTable()->end(), "Identifier already declared");
				SymVar* v = (tt == ttVar) ? (SymVar*)new SymVarGlobal(*it1, type, curTbl->GetOffset(), curTbl) : (SymVar*)new SymVarLocal(*it1, type, curTbl->GetOffset(), curTbl);
				curTbl->insert(*it1, v);
			}
			idents.clear();
			SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
			if (tt == ttVar){
				scan.Next();
				if (TokType() == ttVar)
					scan.Next();
				else
					if (AnothBlock(TokVal()))
						return curTbl;		
			}
			else{
				CheckEof();
				if (TokType() == ttEnd)
					break;
			}
		}
			
	}
	if (tt == ttVar)
		scan.Next();
	return curTbl;
}
Symbol* Parser::ParseConst(SymStIt curTable, bool newConst){
	SymVarConst* res = NULL;
	SymType* type = NULL;
	string curId = curIdent;
	if (IsConstType(TokType())){
		if (IsIntType(TokType())){
			type = (SymType*)(*lowerTable)->GetTable()->find("INTEGER")->second;
			res = new SymVarConstInt(curId, type, (*lowerTable)->GetOffset(), (*lowerTable), ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)(*lowerTable)->GetTable()->find("REAL")->second;
			res = new SymVarConstReal(curId, type, (*lowerTable)->GetOffset(), *lowerTable, ((RealLiteral*)scan.GetToken())->GetVal());
		}
	} 
	else {
		TryError(TokType() != ttIdentifier, "Invalid lexem in constant declaration");
		string s = TokVal();
		SymMap::iterator it = FindS(s, curTable, tableStack->begin());
		TryError(!idFound, "Unknown constant name");
		idFound = true;
		TryError(!it->second->IsConst(), "Invalid constant name");
		type = ((SymVarConst*)(it->second))->GetType();
		if (!newConst)
			curId = s;
		res = type->IsInt() ? (SymVarConst*)new SymVarConstInt(curId, type, (*lowerTable)->GetOffset(), (*lowerTable), ((SymVarConstInt*)(it->second))->GetValue()):
							  (SymVarConst*)new SymVarConstReal(curId, type, (*lowerTable)->GetOffset(), (*lowerTable), ((SymVarConstReal*)(it->second))->GetValue());
	}
	return res;
}
void Parser::ParseTypeConstBlock(SymStIt curTable, TokenType tt){
	Symbol * res = NULL;
	string b_name = (tt == ttType) ? "type" : "const";
	while (true){
		string ident = CheckCurTok(curTable, b_name, *curTable);
		TryError(TokType() != ttEq, "Invalid lexem in " + b_name + " declaration");
		CheckEof();
		if (tt == ttType){
			res = ParseType(curTable, true);
			CheckEof();
			res->SetName(ident);
		}
		else{
			curIdent = ident;
			res = ParseConst(curTable, true);
			CheckEof();
			curIdent = "";
		}
		(*curTable)->insert(ident, res);
		SmthExpexted(TokVal(), TokPos(), TokLine(), ";");
		scan.Next();
		if (TokType() == ttType && tt == ttType || TokType() == ttConst && tt == ttConst)
				scan.Next();
		else
			if (AnothBlock(TokVal()))
				return;	
	}
}
Symbol* Parser::ParseType(SymStIt curTable, bool newType){
	SymType* res = NULL;
	TokenType tt = TokType();
	string s = TokVal();
	bool isPntr =  false;
	if (TokType() == ttCaret){
		CheckEof();
		isPntr = true;
		s = TokVal();
	}
	SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	if (idFound){
		if (isPntr)
			res = new SymTypePointer(curIdent, (SymType*)it->second);	
		else{
			if (newType){
				TryError(!it->second->IsType(), "Invalid lexem in type declaration");
				res = new SymTypeAlias(curIdent, (SymType*)it->second);	
			} 
			else
				res = (SymType*)it->second;
		}
	} 
	else {
		TryError(isPntr, "Invalid pointer declaration");
		switch(tt){
		case ttRecord:
			CheckEof();
			res = new SymTypeRecord(toString(id++), ParseVarRecordBlock(curTable, ttRecord));
			break;
		case ttArray:
			CheckEof();
			res = (SymType*)ParseArray(curTable);
			break;
		default:
			TryError(it == (*curTable)->GetTable()->end() && !newType, "Unknown type");
		}
	}
	return res;
}
Symbol* Parser::ParseArray(SymStIt curTable){
	Symbol* res = NULL;
	TryError(TokType() != ttLeftBracket, "Invalid array declaration");
	list<Symbol*> left, right;
	CheckEof();
	while (TokType() != ttRightBracket){
		Symbol* l = ParseConst(curTable, false);
		CheckEof();
		TryError(TokType() != ttDblDot, "Invalid array declaration");
		CheckEof();
		Symbol* r = ParseConst(curTable, false);
		CheckEof();
		TryError(!(l->GetType()->IsInt() && r->GetType()->IsInt()), "Invalid array bounds");
		TryError(((SymVarConstInt*)l)->GetValue() > ((SymVarConstInt*)r)->GetValue(), "Upper bound less than lower bound");
		left.push_back(l);
		right.push_back(r);
		if (TokType() == ttPoint)
			CheckEof();
	}
	scan.Next();
	TryError(TokType() != ttOf, "Invalid array declaration");
	CheckEof();
	Symbol* type = ParseType(curTable, false);
	list<Symbol*>::reverse_iterator itl = left.rbegin();
	list<Symbol*>::reverse_iterator itr = right.rbegin();
	for (; itl != left.rend() && itr != right.rend(); ++itl, ++itr)
	{
		res = new SymTypeArray(toString(id++), (SymType*)type, (SymVarConstInt*)*itl, (SymVarConstInt*)*itr);
		type = res;
	}
	return res;
}
int StmtWrite::FillTree(int i, int j){
	unsigned int k = 0;
	int tmp = j;int j1 = j;
	j += writeln ? FillTreeIdentConst(i, j, "writeln") : FillTreeIdentConst(i, j, "write");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	
	for (list<NodeExpr*>::iterator it = params->begin(); it != params->end(); ++it){
		PaintBranch(i, j1, 0,  j - j1, true);
		j1 = j;
		j += (*it)->FillTree(i + 4, j);
	}	
	return j - tmp;
}
int FunctionCall::FillTree(int i, int j){
	unsigned int k = 0;
	int tmp = j;
	j = FillTreeOp(i, j, "()");
	if (i + 4 > maxLength)
		maxLength = i + 4;
	j = PaintBranch(i, j, j, j + FillTreeIdentConst(i + 4, j, GetValue()) - 2, false);
	int j1 = j;
	for (list<NodeExpr*>::iterator it = args.begin() ;it != args.end(); ++it){
		PaintBranch(i, j1, 0,  j - j1 + 2, true);
		j1 = j;
		j += (*it)->FillTree(i + 4, j + 2);
	}
	return j - tmp + 2;
}
int UnaryOp::FillTree(int i, int j){
	j = FillTreeOp(i, j, GetValue());
	int h = child->FillTree(i + 4, j);
	return h + 2;
}
int FillTreeBinOp(int i, int j, string Value, NodeExpr* left, NodeExpr* right){
	j = FillTreeOp(i, j, Value);
	int hl = left->FillTree(i + 4, j);
	j = PaintBranch(i, j, 0, hl, true);
	int hr = right->FillTree(i + 4, j);
	return hl + hr + 2;
}

NodeExpr* Parser::ParseSimple(SymStIt curTable, int prior){
	if (prior == 1)
		return ParseFactor(curTable);
	SymType* type = NULL;
	int pos = TokPos();
	int line = TokLine();
	NodeExpr* res = ParseSimple(curTable, prior - 1);
	string s = TokVal();
	while (FindOpPrior(TokVal()) == prior){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseSimple(curTable, prior - 1);
		TryError(!expr1, "Lexem expected, EOF found");
		TryError(!res->GetType(), "Variable must be typed", pos, line);
		TryError(!expr1->GetType(), "Variable must be typed");
		if (expr1->IsInt() && (((SymType*)res->GetType())->IsScalar() && !res->IsInt())){
			expr1 = new UnaryOp(new Symbol("ItoR"), pos, line, expr1);
			expr1->SetType((SymType*)((*lowerTable)->GetTable()->find("REAL"))->second);
		}
		else if (prior != 5 && (res->IsInt() && (((SymType*)expr1->GetType())->IsScalar() && !expr1->IsInt()))){
			res = new UnaryOp(new Symbol("ItoR"), pos, line, res);
			res->SetType((SymType*)((*lowerTable)->GetTable()->find("REAL"))->second);
		}
		res = new BinaryOp(new Symbol(str), pos, line, res, expr1);
		if (prior == 5)
			break;
		type = (res->IsInt()) ? (SymType*)((*lowerTable)->GetTable()->find("INTEGER"))->second : (SymType*)((*lowerTable)->GetTable()->find("REAL"))->second;
		res->SetType(type);
		TryError(!((SymType*)res->GetType())->IsScalar() || !((SymType*)expr1->GetType())->IsScalar(), 
				"Invalid types in binary operation", pos, line);
		pos = TokPos();
		line = TokLine();
	}
	return res;
}
NodeExpr* Parser::ParseNext(SymStIt curTable){
	scan.Next();
	NodeExpr* expr1 = ParseSimple(curTable, 4);
	TryError(!expr1, "Lexem expected, EOF found");
	return expr1;
}
Const* Parser::ParseConst(SymStIt curTable){
	Const* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
	scan.Next();
	CheckAccess(TokVal(), TokPos(), TokLine());
	nextScan = false;
	res = new Const((SymVarConst*)(it->second), pos, line);
	res->SetType(((SymVarConst*)it->second)->GetType());
	return res;
}
NodeExpr* Parser::ParseVariable(SymStIt curTable, NodeExpr* res){
	string s;
	Symbol* var = NULL;
	int pos = TokPos();
	int line = TokLine();
	if (!res) {
		SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		var = (SymVar*)(it->second);
		s = TokVal();
		//Symbol* symb = new Symbol(s);
		res = new Variable(var, pos, line);
	}
	else{
		s = res->GetValue();
		var = new SymVar(res->GetSymbol()->GetName(), (SymType*)res->GetType(), (*curTable)->GetOffset(), *curTable);  //????
	}
	TryError(!var->GetType(), "Variable must be typed", pos, line);
	if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
			res = new Variable(var, pos, line);
	else
		while (true){
			if (!var->GetType()->IsArray() && !var->GetType()->IsRecord() && !var->IsFunc())
				break;
			if (var->IsFunc()){
				res = ParseFunc(curTable, res, &var, pos, line);
				nextScan = false;
			}
			else{
				if (nextScan)
					scan.Next();
				if (var->GetType()->IsArray() && TokType() != ttLeftBracket || var->GetType()->IsRecord() && TokType() != ttDot){
					CheckAccess(TokVal(), TokPos(), TokLine());
					if (res->GetValue() == s)
						res = new Variable(var, pos, line);
					nextScan = false;
					break;
				}
				else
					res = (var->GetType()->IsArray()) ? (NodeExpr*)ParseArr(curTable, res, &var, pos, line) : (NodeExpr*)ParseRecord(curTable, res, &var, pos, line);
			}
			pos = TokPos();
			line = TokLine();
		}
	res->SetType(var->GetType());
	return res;
}
ArrayAccess* Parser::ParseArr(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	SymType* type = (*var)->GetType();
	//if (res->)
	if (TokType() == ttLeftBracket){
		scan.Next();
		while (true){
			NodeExpr* expr = ParseSimple(curTable, 4);
			CheckIsInt(expr, expr->GetPos(), expr->GetLine());
			Symbol* symb = new Symbol("[]");
			res = new ArrayAccess(symb, pos, line, res, expr);
			if (expr->IsConst()){
				int val = ((SymVarConstInt*)expr->GetSymbol())->GetValue();
				int top = ((SymVarConstInt*)((SymTypeArray*)type)->GetTop())->GetValue();
				int bottom = ((SymVarConstInt*)((SymTypeArray*)type)->GetBottom())->GetValue();
				TryError(val > top || val < bottom, "Range check error", expr->GetPos(), expr->GetLine());
			}
			type = ((SymTypeArray*)type)->GetElemType();
			res->SetType(type);
			if(TokType() == ttRightBracket){
				if (!nextScan)
					scan.Next();
				break;
			}
			if (TokType() == ttPoint){
				TryError(!type->IsArray(), "Invalid num of indexes");
				scan.Next();
				TryError(TokType() == ttRightBracket, "Invalid array access");
			}
		}
		pos = TokPos();
		line = TokLine();
	}
	*var = new SymVar(toString(id++), type, (*curTable)->GetOffset(), *curTable);
	res->SetType(type);
	return (ArrayAccess*)res;
}
FunctionCall* Parser::ParseFunc(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	SymType* type = ((SymFunc*)*var)->GetType();
	string s = TokVal();
	scan.Next();
	list<string>* arg_names = ((SymFunc*)*var)->GetArgNames();
	SymTable* arg_table = ((SymFunc*)*var)->GetArgsTable();
	list<NodeExpr*>args;
	list<string>::iterator it = arg_names->begin();
	SymMap::iterator it1;
	if (TokType() == ttLeftParentheses){
		scan.Next();
		while (TokType() != ttRightParentheses){
			it1 = FindS(*it, arg_table);
			NodeExpr* expr = ParseSimple(curTable, 4);
			SymVar* curVar = ((SymVar*)(it1->second));
			if (curVar->IsParamByRef()){
				TryError(!(EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType())), 
					"Incompatible types", expr->GetPos(), expr->GetLine());
				TryError(!expr->LValue(), "Variable identifier expected", expr->GetPos(), expr->GetLine());
			}
			else{
				TryError(!EqTypes((SymType*)expr->GetType(), (SymType*)curVar->GetType()) && 
					!IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType()), "Incompatible types", 
					expr->GetPos(), expr->GetLine());
				if (IToR((SymType*)curVar->GetType(), (SymType*)expr->GetType())){
					expr = new UnaryOp(new Symbol("ItoR"), TokPos(), TokLine(), expr);
					expr->SetType((SymType*)((*lowerTable)->GetTable()->find("REAL"))->second);
				}
			}
			args.push_back(expr);
			++it;
			if (TokType() == ttPoint){
				TryError(it == arg_names->end(), "Invalid parameters num");
				scan.Next();
				TryError(TokType() == ttRightParentheses, "Invalid function call");
			}
		}
		scan.Next();
	}
	else
		nextScan = false;
	TryError(it != arg_names->end(), "Invalid parameters num");
	if (!type)
		nextScan = false;
	res = new FunctionCall(*var, pos, line, args);
	if (type)
		*var = new SymVar(toString(id++), type, (*curTable)->GetOffset(), *curTable);
	res->SetType(type);
	return (FunctionCall*)res;
}
RecordAccess* Parser::ParseRecord(SymStIt curTable, NodeExpr* res, Symbol** var, int pos, int line){
	string s = TokVal();
	SymType* type = (*var)->GetType();
	NodeExpr* expr = NULL;
	Symbol* symb = NULL;
	if (TokType() == ttDot){
		scan.Next();
		while (true){
			TryError(TokType() != ttIdentifier, "Invalid record access");
			SymTable* recTable = ((SymTypeRecord*)(*var)->GetType())->GetFileds();
			SymMap::iterator it = FindS(TokVal(), recTable);
			TryError(it == recTable->GetTable()->end(), "Unknown identifier in record access");
			*var = (SymVar*)(it->second);
			symb = new Symbol(TokVal());
			expr = new Variable(*var, pos, line);
			res = new RecordAccess(*var, pos, line, res, expr);
			res->SetType(type);
			scan.Next();
			if (TokType() == ttDot){
				type = (*var)->GetType();
				TryError(!type->IsRecord(), "Invalid record access", pos, line);
				scan.Next();
			}
			else {
				nextScan = false;
				break;
			}
		}
		pos = TokPos();
		line = TokLine();
	}
	nextScan = false; 
	res->SetType(type);
	return (RecordAccess*)res;
}
NodeExpr* Parser::ParseFactor(SymStIt curTable){
	NodeExpr* res = NULL;
	int pos = TokPos();
	int line = TokLine();
	string s = TokVal();
	if (TokType() == ttEOF)
		return NULL;
	TryError(TokType() == ttBadToken, "Invalid lexem", pos, line);
	if (IsUnaryOp(TokVal())){
		string str = TokVal();
		scan.Next();
		NodeExpr* expr1 = ParseFactor(curTable);
		TryError(!expr1, "Lexem expected, EOF found");
		Symbol* symb = new Symbol(str);
		res = new UnaryOp(symb, pos, line, expr1);
		TryError(!((SymType*)expr1->GetType())->IsScalar(), "Invalid unary operation", pos, line);
		res->SetType(expr1->GetType()); 
	}
	else if (TokType() == ttIdentifier){
		SymMap::iterator it = FindS(TokVal(), curTable, tableStack->begin());
		TryError(!idFound, "Unknown identifier");
		TryError(it->second->IsType(), "Invalid identifier");
		if (it->second->IsConst())
			res = ParseConst(curTable);
		else if (it->second->IsVar() || it->second->IsFunc())
			res = ParseVariable(curTable, NULL);
		else if (it->second->IsProc())
			res = ParseFunc(curTable, new NodeExpr(new Symbol(TokVal()), pos, line), &(it->second), pos, line);
		if (nextScan)
			scan.Next();
		nextScan = true;
	}
	else if (IsLiteral(TokType())){
		SymVarConst* symb = NULL;
		SymType* type = NULL;
		if (IsIntType(TokType())){
			type = (SymType*)((*lowerTable)->GetTable()->find("INTEGER"))->second;
			symb = new SymVarConstInt(toString(id++), type, (*curTable)->GetOffset(), *curTable, ((IntLiteral*)scan.GetToken())->GetVal());
		}
		else if (TokType() == ttRealLit){
			type = (SymType*)((*lowerTable)->GetTable()->find("REAL"))->second;
			symb = new SymVarConstReal(toString(id++), type, (*curTable)->GetOffset(), *curTable, ((RealLiteral*)scan.GetToken())->GetVal());
		}
		else{
			type = (SymType*)((*lowerTable)->GetTable()->find("STRING"))->second;
			symb = new SymVarConstString(toString(id++), type, (*curTable)->GetOffset(), *curTable, ((StringLiteral*)scan.GetToken())->GetVal());
		}
		res = new Const(symb, pos, line);
		res->SetType(type);
		scan.Next();
	}
	else if (TokType() == ttLeftParentheses){
		scan.Next();
		TryError(TokType() == ttRightParentheses, "Empty bracket sequence", pos, line);
		res = ParseSimple(curTable, 5);
		TryError(TokType() != ttRightParentheses, "Unclosed bracket", pos, line);
		if (res->IsFunction() || res->LValue() && !isAccess)
			res = ParseVariable(curTable, res);
		scan.Next();
	}
	else
		throw Error("Unexpected lexem found", pos, line);
	return res;
}