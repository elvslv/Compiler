#include "Parser.h"
#include <iostream>
#include <fstream>
int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "Pascal Compiler. Vasilyeva Elena, FENU, 236 group. 2010\n";
		cout << "\nUsage:\n";
		cout << "compiler filename [options]\n";
		cout << "-lex Print tokens info\n";
		cout << "-syn Print syntax tree\n";
	}
	else
	{
		ifstream is;
		is.open(argv[1]);
		ofstream os;
		string tmp = argv[1];
		tmp += ".out";
		os.open(tmp.c_str());
		if (is.good() && os.good())
		{
			Scanner scanner(is);
			bool flex = false;
			bool fsyn = false;
			for (int i = 2; i < argc; ++i)
			{
				if (strcmp(argv[i], "-lex") == 0)
					flex = true;
				if (strcmp(argv[i], "-syn") == 0)
					fsyn = true;
			}
			if (flex)
			{
				while(scanner.GetToken()->GetType() != ttBadToken && scanner.GetToken()->GetType() != ttEOF)
				{
					scanner.Next();
					Token* tkn = scanner.GetToken();
					os << tkn->GetValue() << " " << tkn->GetTypeName() <<  " " << tkn->GetPos() << " " << tkn->GetLine() << "\n";
				}
			}
			if (fsyn)
			{
				Parser parser(scanner);
				Expr* expr = parser.ParseSimpleExpr();
				if (parser.HasError())
					parser.PrintError(os);
				else 
				{
					
					if (scanner.GetToken()->GetType() != ttEOF)
						os << scanner.GetToken()->GetPos()<< " " << scanner.GetToken()->GetLine() << " "
						<< "Incorrect Syntax Construction";
					else
						if (expr)
							expr->Print(os, 0);
				}
					
			}
			is.close();
			os.close();
		}
		else
			cout << "Wrong Input or Output File";
	}
	return 0;
}