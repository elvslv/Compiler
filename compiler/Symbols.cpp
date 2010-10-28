#include "Symbols.h"
void SymVar::Print(ostream& os, bool f) { 
	os << "var "<< name << " : "; 
	type->Print(os, f); 
	os << ";";
}

void PrintArgs(ostream& os, SymTable* args){
	if (!args->empty()){	
		os << "(";  
		int i = 0;
		for (SymTable::iterator it = args->begin(); it != args->end(); ++it)
			(*it->second).Print(os, false);
		os << ")";
	}
}

void SymProc::Print(ostream& os, bool f) { 
	os << "procedure "<< name;
	PrintArgs(os, args);
	os << ";";
}

void SymFunc::Print(ostream& os, bool f) { 
	os << "function " << name;
	PrintArgs(os, args);
	os << ": ";
	type->Print(os, false);
	os << ";";
}

void SymTypeRecord::Print(ostream& os, bool f) {
	if (!f)
		os << name;
	else{
		if (name != "" && (name[0] < '0' || name[0] > '9'))
			os << "type " << name << " = ";
		os << "record \n";
		for (SymTable::iterator it = fields->begin(); it != fields->end(); ++it){
			os << "    ";
			string s = (*it->second).GetType()->GetName();
			(*it->second).Print(os, (*it->second).IsVar() && s[0] >= '0' && s[0] <= '9');
			os << "\n";
		}
		os << "end";
	}
}

void SymTypeAlias::Print(ostream& os, bool f) { 
	if (!f)
		os << name;
	else{
		if (name != "" && !(name[0] >= '0' && name[0] <= '9'))
			os << "type "<< name << " = ";
		refType->Print(os, false);
	}
}

void SymTypePointer::Print(ostream& os, bool f) { 
	if (!f)
		os << name;
	else{
		if (name != "" && !(name[0] >= '0' && name[0] <= '9') && f)
			os << "type "<< name << " = ";
		os << "^";  
		refType->Print(os, false);
	}
}

void SymVarConst::Print(ostream& os, bool f) { 
	if (!f && !(name == "" || (name[0] >= '0' && name[0] <= '9')))
		os << "const " << name;
	else{
		if (name == "" || (name[0] >= '0' && name[0] <= '9')){
			os << "const ";
			PrintVal(os);
		}
		else {
			os << "const " << name << " = ";
			PrintVal(os);
		}
	}
}

void SymTypeArray::Print(ostream& os, bool f) {
	if (!f)
		os << name;
	else{
		if (name != "" && (name[0] < '0' || name[0] > '9'))
			os << "type " << name << " = ";
		os << "array [";
		bottom->Print(os, false);
		os << "..";
		top->Print(os, false);
		os << "] of ";
		elemType->Print(os, elemType->GetName()[0] >= '0' && elemType->GetName()[0] <= '9');
	}
}

void SymVarParamByRef::Print(ostream& os, bool f) { 
	os << "var (by ref) "<< name << " : "; 
	type->Print(os, false); 
	os << ";";
}