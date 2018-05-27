/***********************************************************
From: Marco Qiaofeng Liu (Email: qiaofengmarco@outlook.com),
	  School of Computer Science and Engineering,
	  Southeast University, Jiulonghu Campus,
	  Nanjing, China
************************************************************/
#include "Headers.h"
#include "QLex.hpp"
int main()
{
    QLex Lex;
    Lex.LexCreate("./lex.l");
    cout << "Finished." << endl;
	system("pause");
    return 0;
}
