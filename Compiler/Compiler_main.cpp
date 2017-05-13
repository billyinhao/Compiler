#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "global.h"

QFile file("data/log.dat");
QTextStream logtxt(&file);

int compiler_main(string src)
{
    if (file.open(QIODevice::WriteOnly|QIODevice::Text)){
        if (Lexical_Analyzer(src))//词法分析
        {
            logtxt << QStringLiteral("词法分析完成！") << endl;//词法分析完成
            legend();
            if (Syntax_Analyzer())//语法分析入口
            {
                logtxt << QStringLiteral("语法分析完成！") << endl;
                return 1;
            }
            else logtxt << QStringLiteral("语法分析错误！") << endl;
        }
        else logtxt << QStringLiteral("词法分析错误！") << endl;
        file.close();
    }
    return 0;
}

void DestroyList(OutList list){
    OutList lastptr = list;
        if(lastptr != NULL){
            OutList nextptr = lastptr->next;
            while (nextptr != NULL) {
                free(lastptr);
                lastptr = nextptr;
                nextptr = lastptr->next;
            }
            free(lastptr);
        }
}
