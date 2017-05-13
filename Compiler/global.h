#ifndef GLOBAL_H
#define GLOBAL_H

#include <fstream>
#include <string>
#include <sstream>
#include <QFile>
#include <QTextStream>
using namespace std;

#define TokenList_range 1000    //token表空间

typedef struct OutNode{
    QString str;
    struct OutNode *next;
}OutNode,*OutList;

extern QTextStream logtxt;

extern OutList tokenlist;
extern OutList classifylist;
extern OutList symbolist;
extern OutList typelist;
extern OutList ainflist;
extern OutList rinflist;
extern OutList fvlist;
extern OutList pfinflist;
extern OutList conslist;
extern OutList lenlist;
extern OutList quartlist;

////////////////////函数声明/////////////////////
extern int compiler_main(string);
extern bool Lexical_Analyzer(string src);//词法分析入口
extern void legend();
extern bool Syntax_Analyzer();//语法分析入口
extern void DestroyList(OutList);//销毁输出链表

using namespace std;

#endif //GLOBAL_H
