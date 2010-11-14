#ifndef _CODEGEN_H_
#define _CODEGEN_H_
#include "common.h"

class AsmCmd{
public:
	AsmCmd(cmd n): name(n){};
	AsmCmd(): name(asmUnknown){};
	virtual void Print(ostream& os) = 0;
protected:
	cmd name;
};

class AsmOp{
public:
	AsmOp(){};
	virtual void Print(ostream& os) = 0;
};

class AsmReg: public AsmOp{
public:
	AsmReg(string n):name(n){};
	void Print(ostream& os){ os << name; }
private:
	string name;
};

class AsmImm: public AsmOp{
public:
	AsmImm(){};
	virtual bool IsInt() { return false; }
	virtual bool IsReal() { return false; }
	virtual bool IsString() { return false; }
	virtual void Print(ostream& os) = 0;
};

class AsmImmInt: public AsmImm{
public:
	AsmImmInt(int v): value(v){};
	bool IsInt() { return true; }
	void Print(ostream& os){ os << value; }
private:
	int value;
};

class AsmImmReal: public AsmImm{
public:
	AsmImmReal(float v): value(v){};
	bool IsReal() { return true; }
	void Print(ostream& os){ os << value; }
private:
	double value;
};

class AsmImmString: public AsmImm{
public:
	AsmImmString(string v): value(v){};
	bool IsString() { return true; }
	void Print(ostream& os){ os << value; }
private:
	string value;
};

class AsmMem: public AsmOp{
public:
	AsmMem(string n): name(n){};
	void Print(ostream& os){ os << name; }
private:
	string name;
};

class AsmStackVal: public AsmOp{
public:
	AsmStackVal(int o): offset(o){};
	void Print(ostream& os) {os << "[ebp + " << offset << "]";}
private:
	int offset;
};

class AsmInvoke: public AsmCmd{
public:
	AsmInvoke(cmd n, AsmImm* m, AsmReg* r): AsmCmd(n), format(m), reg(r){};
	AsmInvoke(cmd n, AsmImm* m): AsmCmd(n), format(m), reg(NULL){};
	void Print(ostream& os){
		os << "invoke " << cmdNames[name] << ", addr ";
		format->Print(os);
		if (reg){
			os << ", ";
			reg->Print(os);
		}
	}
private:
	AsmImm* format;
	AsmReg* reg;
};

class AsmData: public AsmCmd{
public:
	AsmData(cmd name, AsmMem* v, AsmImm* val): AsmCmd(name), var(v), value(val){};
	AsmData(cmd name, AsmMem* v): AsmCmd(name), var(v), value(new AsmImmInt(0)){};
	virtual bool IsDw() { return false; }
	virtual bool IsDb() { return false; }
	virtual bool IsDd() { return false; }
	void Print(ostream& os) {
		var->Print(os); 
		IsDd() ? os << " dd " : IsDw() ? os << " dw " : os << " db " ; 
		value->Print(os);
	}
protected:
	AsmMem* var;
	AsmImm* value;
};

class AsmDw: public AsmData{
public:
	AsmDw(AsmMem* v, AsmImm* val): AsmData(asmDw, v, val){}; 
	AsmDw(AsmMem* v): AsmData(asmDw, v){};
	bool IsDw() { return true; }

};

class AsmDb: public AsmData{
public:
	AsmDb(AsmMem* v, AsmImm* val): AsmData(asmDb, v, val){};
	AsmDb(AsmMem* v): AsmData(asmDb, v){};
	bool IsDb() { return true; }
};

class AsmDd: public AsmData{
public:
	AsmDd(AsmMem* v, AsmImm* val): AsmData(asmDd, v, val){};
	AsmDd(AsmMem* v): AsmData(asmDd, v){};
	bool IsDd() { return true; }
};

class AsmCmd0: public AsmCmd{
public:
	AsmCmd0(cmd n): AsmCmd(n){};
	void Print(ostream& os){ 
		os << cmdNames[name] << "\n"; 
	}
}; 

class AsmCmd1: public AsmCmd{
public:
	AsmCmd1(cmd n, AsmOp* o): AsmCmd(n), op(o){};
	void Print(ostream& os) {
		os << cmdNames[name] << " ";
		op->Print(os);
	}
private:
	AsmOp* op;
};

class AsmCmd2: public AsmCmd{
public:
	AsmCmd2(cmd n, AsmOp* o1, AsmOp* o2): AsmCmd(n), op1(o1), op2(o2){};
	void Print(ostream& os) {
		os << cmdNames[name] << " ";
		op1->Print(os);
		os << ", ";
		op2->Print(os);
	}
private:
	AsmOp* op1, *op2;
};

class AsmProc{
public:
	AsmProc(string n, procType t): name(n), type(t) {};
	void Add(cmd cmd){ commands.push_back(new AsmCmd0(cmd)); }
	void Add(cmd cmd, AsmOp* op){ 
		switch(cmd){
			case asmPrintf: case asmScanf:
				commands.push_back(new AsmInvoke(cmd, (AsmImm*)op));
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
				commands.push_back(new AsmCmd1(cmd, op)); 
		}
	}
	void Add(cmd cmd, AsmOp* op1, AsmOp* op2){ 
		switch(cmd){
			case asmPrintf: case asmScanf:
				commands.push_back(new AsmInvoke(cmd, (AsmImm*)op1, (AsmReg*)op2));
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
				commands.push_back(new AsmCmd2(cmd, op1, op2)); 
		}
	}
	void Print(ostream& os){
		switch(type){
			case pMain:
				os << name << ":\n";
				break;
			case pProc:
				os << "proc " << name << "\n";
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
				os << "\tinvoke crt_scanf, addr G_writef\n";
				os << "\tinvoke ExitProcess, 0\n";
				os << "end " << name << "\n";
				break;
			case pProc:
				os << name << " endp" << "\n";
				break;
		}
	}
private:
	list<AsmCmd*> commands;
	string name;
	procType type;
};

class AsmCode{
public:
	AsmCode(){};
	AsmProc* AddProc(string name, procType type){ 
		AsmProc* pr = new AsmProc(name, type);
		proc.push_back(pr);
		return pr;
	}
	void Print(ostream& os){
		os << ".686\n";
		os << ".model flat,stdcall\n";
		os << "include     include\\msvcrt.inc\n";
		os << "include include\\kernel32.inc\n";
		os << "include include\\fpu.inc\n";
		os << "include include\\user32.inc\n";
		os << "includelib  lib\\msvcrt.lib\n";
		os << "includelib lib\\user32.lib\n";
		os << "includelib lib\\kernel32.lib\n";
		for (list<AsmProc*>::iterator it = proc.begin(); it != proc.end(); ++it)
			(*it)->Print(os);
	}
	AsmProc* CurProc(){ return *(proc.rbegin());}
private:
	list<AsmProc*> proc;
};
#endif //_CODEGEN_H_
