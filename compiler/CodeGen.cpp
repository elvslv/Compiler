#include "CodeGen.h"

void FillCmdNames(){
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
	cmdNames[asmData] = ".data"; cmdNames[asmScanf] = "crt_scanf"; cmdNames[asmPrintf] = "call crt_printf";
	cmdNames[asmCall] = "call"; cmdNames[asmDwtoa] = "dwtoa"; cmdNames[asmStdout] = "StdOut";
	cmdNames[asmNeg] = "neg"; cmdNames[asmFLd] = "fld"; cmdNames[asmFStp] = "fstp"; cmdNames[asmFAdd] = "fadd";
	cmdNames[asmFSub] = "fsub"; cmdNames[asmFMul] = "fmul"; cmdNames[asmFDiv] = "fdiv"; cmdNames[asmFChs] = "fchs"; 
	cmdNames[asmRepMovsd] = "rep movsd";  cmdNames[asmPushfd] = "pushfd";	cmdNames[asmFLd1] = "fld1";
	cmdNames[asmCMovE] = "cmove"; cmdNames[asmCMovNE] = "cmovne"; cmdNames[asmCMovL] = "cmovl"; 
	cmdNames[asmCMovLE] = "cmovle"; cmdNames[asmCMovG] = "cmovg"; cmdNames[asmCMovGE] = "cmovge";
	cmdNames[asmFComI] = "fcomi"; cmdNames[asmFCMovE] = "fcmove"; cmdNames[asmFCMovNE] = "fcmovne";
	cmdNames[asmFCMovB] = "fcmovb"; cmdNames[asmFCMovNBE] = "fcmovnbe"; cmdNames[asmFCMovBE] = "fcmovbe";
	cmdNames[asmFCMovNB] = "fcmovnb"; cmdNames[asmTest] = "test"; cmdNames[asmJZ] = "jz"; cmdNames[asmJNZ] = "jnz";
	cmdNames[asmJG] = "jg"; cmdNames[asmJL] = "jl"; cmdNames[asmFInit] = "finit"; cmdNames[asmFIld] = "fild";
	cmdNames[asmFLdZ] = "fldz";
}

void AsmImmString::Print(ostream& os){ 
	string tmp = value;
	for (int i = 0; i < tmp.length() / 2; ++i)
		swap(tmp[i], tmp[tmp.length() - 1 - i]);
	os << tmp; 
}

void AsmMem::Print(ostream& os){ 
	os << name; 
	if (offset)
		offset > 0 ? os << " + " << offset : os << offset;
}

void AsmInvoke::Print(ostream& os){
	os << "invoke " << cmdNames[name] << ", ";
	op1->Print(os);
	if (op2){
		os << ", ";
		op2->Print(os);
	}
}

void AsmData::Print(ostream& os) {
	var->Print(os); 
	IsDd() ? os << " dd " : IsDw() ? os << " dw " : os << " db " ; 
	value->Print(os);
}

void AsmDup::Print(ostream& os) {
	data->Print(os);
	os << " dup(";
	value->Print(os);
	os << ")";
}

void AsmCmd2::Print(ostream& os) {
	os << cmdNames[name] << " ";
	op1->Print(os);
	os << ", ";
	op2->Print(os);
}

void AsmProc::Add(cmd command, AsmOp* op){ 
	switch(command){
		case asmDwtoa: case asmStdout:
			commands.push_back(new AsmInvoke(command, op));
			break;
		case asmDb:
			commands.push_back(new AsmDb((AsmMem*)op));
			break;
		case asmDw:
			commands.push_back(new AsmDw((AsmMem*)op));
			break;
		case asmDd:
			commands.push_back(new AsmDd((AsmMem*)op));
			break;
		default:
			commands.push_back(new AsmCmd1(command, op)); 
	}
}
void AsmProc::AddLabel(string name){
	commands.push_back(new AsmLabel(name));
}
void AsmProc::Add(cmd command, AsmOp* op1, AsmOp* op2){ 
	switch(command){
		case asmDwtoa: case asmStdout:
			commands.push_back(new AsmInvoke(command, op1, op2));
			break;
		case asmDb:
			commands.push_back(new AsmDb((AsmMem*)op1, (AsmImm*)op2));
			break;
		case asmDw:
			commands.push_back(new AsmDw((AsmMem*)op1, (AsmImm*)op2));
			break;
		case asmDd:
			commands.push_back(new AsmDd((AsmMem*)op1, (AsmImm*)op2));
			break;
		default:
			commands.push_back(new AsmCmd2(command, op1, op2)); 
	}
}
void AsmProc::Add(cmd command, AsmCmd* cmnd, AsmOp* op){ 
	if (command != asmDup)
		return;
	commands.push_back(new AsmDup((AsmData*)cmnd, (AsmImm*)op));
}
void AsmProc::Print(ostream& os){
	switch(type){
		case pMain:
			os << name << ":\n";
			break;
		case pProc:
			os << name << "proc " <<  "\n";
			break;
		case pData: case pCode:
			os << name << "\n";
	}
	for (list<AsmCmd*>::iterator it = commands.begin(); it != commands.end(); ++it){
		os << "\t";
		(*it)->Print(os);
		os << "\n";
	}
	switch(type){
		case pMain:
			os << "\tinvoke ExitProcess, 0\n";
			os << "end " << name << "\n";
			break;
		case pProc:
			os << name << " endp" << "\n";
			break;
	}
}

AsmProc* AsmCode::AddProc(string name, procType type){ 
	AsmProc* pr = new AsmProc(name, type);
	proc.push_back(pr);
	return pr;
}
void AsmCode::Print(ostream& os){
	os << ".686\n";
	os << ".model flat,stdcall\n";
	os << "option casemap :none\n";
	os << "include include\\windows.inc\n";
	os << "include include\\msvcrt.inc\n";
	os << "include include\\kernel32.inc\n";
	os << "include include\\masm32.inc\n";
	os << "include include\\fpu.inc\n";
	os << "includelib lib\\masm32.lib\n";
	os << "includelib lib\\msvcrt.lib\n";
	os << "includelib lib\\kernel32.lib\n";
	os << "includelib lib\\fpu.lib\n";
	for (list<AsmProc*>::iterator it = proc.begin(); it != proc.end(); ++it)
		(*it)->Print(os);
}