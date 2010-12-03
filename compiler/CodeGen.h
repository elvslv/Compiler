#ifndef _CODEGEN_H_
#define _CODEGEN_H_
#include "common.h"

void FillCmdNames();

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
	void Print(ostream& os);
private:
	string value;
};

class AsmMem: public AsmOp{
public:
	AsmMem(string n): AsmOp(n), offset(0){};
	AsmMem(string n, int off): AsmOp(n), offset(off){};
	void Print(ostream& os);
	AsmMem* operator+(int off){	return new AsmMem(name, off + offset); }
private:
	int offset;
};

class AsmProcName: public AsmOp{
public:
	AsmProcName(string n): AsmOp(n), offset(0){};
	AsmProcName(string n, int off): AsmOp(n), offset(off){};
	void Print(ostream& os) { os << name; } ;
private:
	int offset;
};

class AsmMemByAddr: public AsmOp{
public:
	AsmMemByAddr(AsmOp* m, int offs): mem(m), offset(new AsmImmInt(offs)){};
	AsmMemByAddr(AsmOp* m, AsmOp* off): mem(m), offset(off){};
	void Print(ostream& os) {
		os << "["; 
		mem->Print(os);
		os  << " + ";
		offset->Print(os); 
		os << "]";}
private:
	AsmOp* mem;
	AsmOp* offset;
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
	void Print(ostream& os);
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
	void Print(ostream& os);
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
	void Print(ostream& os);
private:
	AsmData* data;
	AsmImm* value;
};

class AsmCmd0: public AsmCmd{
public:
	AsmCmd0(cmd n): AsmCmd(n){};
	void Print(ostream& os){ os << cmdNames[name] << "\n"; }
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
	void Print(ostream& os);
private:
	AsmOp* op1, *op2;
};

class AsmLabel: public AsmCmd0{
public:
	AsmLabel(string n): AsmCmd0(asmLbl), name(n){};
	void Print(ostream& os) { os << name << ":"; }
private:
	string name;
};

class AsmLabelOp: public AsmOp{
public:
	AsmLabelOp(string name): AsmOp(name){};
	void Print(ostream& os){ os << name; }
};

class AsmProc{
public:
	AsmProc(string n, procType t): name(n), type(t) {};
	void Add(cmd command){ commands.push_back(new AsmCmd0(command)); }
	void Add(cmd command, AsmOp* op);
	void Add(cmd command, AsmOp* op1, AsmOp* op2);
	void Add(cmd command, AsmCmd* cmnd, AsmOp* op);
	void AddLabel(string name);
	void Print(ostream& os);
private:
	list<AsmCmd*> commands;
	string name;
	procType type;
};

class AsmCode{
public:
	AsmCode(){ FillCmdNames(); };
	AsmProc* AddProc(string name, procType type);
	void Print(ostream& os);
	AsmProc* CurProc(){ return *(proc.rbegin());}
private:
	list<AsmProc*> proc;
};
#endif //_CODEGEN_H_
