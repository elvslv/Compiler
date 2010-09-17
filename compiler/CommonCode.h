#ifndef _COMMONCODE_H_
#define _COMMONCODE_H_

#include<string>

using namespace std;

string UpperCase(string tmp)
{
	for (unsigned int i = 0; i < tmp.length(); ++i)
		if (tmp[i] >= 'a' && tmp[i] <= 'z')
			tmp[i] = tmp[i] - 'a' + 'A';
	return tmp;
}
#endif //_COMMONCODE_H_