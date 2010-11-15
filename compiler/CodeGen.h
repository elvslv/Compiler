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
	AsmOp() { name = ""; }
	AsmOp(string n): name(n){};
	virtual string GetName() { return name; }
	virtual void Print(ostream& os) = 0;
protected:
	string name;
};

class AsmReg: public AsmOp{
public:
	AsmReg(string n):AsmOp(n){};
	void Print(ostream& os){ os << name; }
};

class AsmImm: public AsmOp{
public:
	AsmImm(){};
	AsmImm(string n): AsmOp(n){};
	virtual bool IsInt() { return false; }
	virtual bool IsReal() { return false; }
	virtual bool IsString() { return false; }
	virtual void Print(ostream& os) = 0;
};

class AsmOffset: public AsmOp{
public:
	AsmOffset(string n): AsmOp(n) {};
	AsmOffset(AsmOp* op): AsmOp(op->GetName()) {};
	void Print(ostream& os) { os << "offset " << name; } 
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
	AsmImmReal(double v): value(v){};
	bool IsReal() { return true; }
	void Print(ostream& os){ os << value; }
private:
	double value;
};

class AsmImmString: public AsmImm{
public:
	AsmImmString(string v): value(v){};
	bool IsString() { return true; }
	void Print(ostream& os){ 
		string tmp = value;
		for (int i = 0; i < tmp.length() / 2; ++i)
			swap(tmp[i], tmp[tmp.length() - 1 - i]);
		os << tmp; }
private:
	string value;
};

class AsmMem: public AsmOp{
public:
	AsmMem(string n): AsmOp(n){};
	void Print(ostream& os){ os << name; }
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
	AsmInvoke(cmd n, AsmOp* o1, AsmOp* o2): AsmCmd(n), op1(o1), op2(o2){};
	AsmInvoke(cmd n, AsmOp* o1): AsmCmd(n), op1(o1), op2(NULL){};
	void Print(ostream& os){
		os << "invoke " << cmdNames[name] << ", ";
		op1->Print(os);
		if (op2){
			os << ", ";
			op2->Print(os);
		}
	}
private:
	AsmOp* op1;
	AsmOp* op2;
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

class AsmDup: public AsmCmd{
public:
	AsmDup(AsmData* d, AsmImm* val): data(d), value(val) {};
	void Print(ostream& os) {
		data->Print(os);
		os << " dup(";
		value->Print(os);
		os << ")";
	}
private:
	AsmData* data;
	AsmImm* value;
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
	void Add(cmd command){ commands.push_back(new AsmCmd0(command)); }
	void Add(cmd command, AsmOp* op){ 
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
	void Add(cmd command, AsmOp* op1, AsmOp* op2){ 
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
	void Add(cmd command, AsmCmd* cmnd, AsmOp* op){ 
		if (command != asmDup)
			return;
		commands.push_back(new AsmDup((AsmData*)cmnd, (AsmImm*)op));
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
		os << "option casemap :none\n";
		os << "include include\\windows.inc\n";
		os << "include include\\kernel32.inc\n";
		os << "include include\\masm32.inc\n";
		os << "includelib lib\\masm32.lib\n";
		os << "includelib lib\\kernel32.lib\n";
		for (list<AsmProc*>::iterator it = proc.begin(); it != proc.end(); ++it)
			(*it)->Print(os);
	}
	AsmProc* CurProc(){ return *(proc.rbegin());}
private:
	list<AsmProc*> proc;
};
#endif //_CODEGEN_H_
