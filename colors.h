
#ifndef _COLORS_
#define _COLORS_

#include <string>

using namespace std;


/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


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