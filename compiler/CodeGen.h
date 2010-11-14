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
	virtual void Print(ostream& os) = 0;
};

class AsmImmInt: public AsmImm{
public:
	AsmImmInt(int v): value(v){};
	void Print(ostream& os){ os << value; }
private:
	int value;
};

class AsmImmReal: public AsmImm{
public:
	AsmImmReal(float v): value(v){};
	void Print(ostream& os){ os << value; }
private:
	double value;
};

class AsmImmString: public AsmImm{
public:
	AsmImmString(string v): value(v){};
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

class AsmData: public AsmCmd{
public:
	AsmData(cmd name, AsmMem* v, AsmImm* val): AsmCmd(name), var(v), value(val){};
	AsmData(cmd name, AsmMem* v): AsmCmd(name), var(v), value(new AsmImmInt(0)){};
	virtual bool IsDw() { return false; }
	virtual bool IsDb() { return false; }
	void Print(ostream& os) {
		var->Print(os); 
		IsDb() ? os << " db " : os << " dw "; 
		value->Print(os);
		os << "\n";
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
		os << "\n";
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
		os << "\n";
	}
private:
	AsmOp* op1, *op2;
};

class AsmProc{
public:
	AsmProc(string n, bool m, bool d): name(n), main(m), data(d) {};
	void Add(cmd cmd){ commands.push_back(new AsmCmd0(cmd)); }
	void Add(cmd cmd, AsmOp* op){ 
		if (cmd == asmDw)
			commands.push_back(new AsmDw((AsmMem*)op));
		else if (cmd == asmDb)
			commands.push_back(new AsmDb((AsmMem*)op));
		else
			commands.push_back(new AsmCmd1(cmd, op)); 
	}
	void Add(cmd cmd, AsmOp* op1, AsmOp* op2){ 
		if (cmd == asmDw)
			commands.push_back(new AsmDw((AsmMem*)op1, (AsmImm*)op2));
		else if (cmd == asmDb)
			commands.push_back(new AsmDb((AsmMem*)op1, (AsmImm*)op2));
		else
			commands.push_back(new AsmCmd2(cmd, op1, op2)); 
	}
	AsmMem* mem(string name){ return new AsmMem(name); }
	void Print(ostream& os){
		if (!data && !main)
			os << "proc " << name << "\n";
		else if (!data)
			os << name << ":\n";
		for (list<AsmCmd*>::iterator it = commands.begin(); it != commands.end(); ++it)
			(*it)->Print(os);
		if (!data && !main)
			os << name << " endp" << "\n";
		else if (!data)
			os << "end "<< name << "\n";
	}
private:
	list<AsmCmd*> commands;
	string name;
	bool main;
	bool data;
};

class AsmCode{
public:
	AsmCode(){};
	AsmProc* AddProc(string name, bool main, bool data){ 
		AsmProc* pr = new AsmProc(name, main, data);
		proc.push_back(pr);
		return pr;
	}
	void Print(ostream& os){
		os << ".686\n";
		os << "include     include\\msvcrt.inc\n";
		os << "include include\\kernel32.inc\n";
		os << "include include\\fpu.inc\n";
		os << "include include\\user32.inc\n";
		os << "includelib  lib\\msvcrt.lib\n";
		os << "includelib lib\\user32.lib\n";
		os << "includelib lib\\kernel32.lib\n";
		os << "includelib lib\\fpu.lib\n";
		for (list<AsmProc*>::iterator it = proc.begin(); it != proc.end(); ++it)
			(*it)->Print(os);
	}
	AsmProc* CurProc(){ return *(proc.rbegin());}
private:
	list<AsmProc*> proc;
};
#endif //_CODEGEN_H_
