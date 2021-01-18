
#ifndef _COLORS_
#define _COLORS_

#include <string>

using namespace std;


/* FOREGROUND */
#define RST  "\001\e[0m\002"
#define KRED  "\001\e[1m\e[31m\002"
#define KGRN  "\001\e[32m\002"
#define KYEL  "\001\x1B[33m\002"
#define KBLU  "\001\e[34m\002"
#define KMAG  "\001\x1B[35m\002"
#define KCYN  "\001\x1B[36m\002"
#define KWHT  "\001\x1B[37m\002"


string green(const string &x);

string yellow(const string &x);

string blue(const string &x);

string magenta(const string &x);

string cyan(const string &x);

string white(const string &x);

string red(const string &x);

string bold(const string &x);

string underline(const string &x);

#endif    /* _COLORS_ */