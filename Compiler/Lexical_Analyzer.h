#ifndef LEXICAL_ANALYZER_H
#define LEXICAL_ANALYZER_H

#include <string>
using namespace std;

#define end_state 0        //结束状态
#define error_state -1     //错误状态
#define blank_state 13     //状态机第一个字符为空格或换行或tab的状态
#define EndSignal 111      //结束标记@

#define sheet_range 100     //静态表各表的空间

/************全局变量************/
int KT_range = 0; //关键字表实际大小
int SPT_range = 0;//单字符界符表实际大小
int DPT_range = 0;//双字符界符表实际大小

/***********外部变量*************/
typedef struct
{
    int code;			//类别码
    string value;		//单词内容
    double *cosPoint;  //指向常数表的指针
    struct SYMBL_DEFINE *point;//指向符号表总表的指针
}Token;

extern Token TokenList[TokenList_range];
int tokenI = 0;               //token表下标

//词法分析动态符号表系统
Token dynamic_iT[sheet_range];//标识符
Token dynamic_cT[sheet_range];//字符
Token dynamic_sT[sheet_range];//字符串
Token dynamic_CT[sheet_range];//常数
Token dynamic_kT[sheet_range];//关键字

double CT_list[sheet_range];  //常数
int CT_list_I = 0;			  //常数表下标

//静态关键字表、界符表(双/单)
string KT[sheet_range], DPT[sheet_range], SPT[sheet_range];

/************函数声明************/
int buf_to_num(char buf);                               //字符转换成代号
int state_machine(int state, char buf);                 //状态机函数
int state_to_code(int state_before, string token_value);//生成token序列码
bool Read_List();                                       //从文件中读入KT/PT静态表
void legeng();                                          //生成动态符号表系统
void TokenList_printer();                               //输出Token序列表
void Lexical_Initialize();

/*****常数处理机类声明******/
class CCons
{
private:
    int aut[8][5];					//状态转换矩阵
    int s;							//当前状态
    int n, p, m, e, t;              //尾数值，指数值，小数位数，指数符号，类型
    double num;                     //常数
    char ch;                        //当前符号
    int pos_in_value = 0;           //读入的字符在待转换的字符串中的位置
public:
    CCons();//构造函数
    double number(string value);    //拼一个常数
private:
    void Error();
    int map(char ch);
    int find(int s, char ch);
    void act(int s, char ch);
};

#endif //LEXICAL_ANALYZER_H
