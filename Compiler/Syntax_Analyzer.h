#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

#define SYMBL_range 50 //符号表
#define TYPEL_range 50 //类型表
#define AINFL_range 50 //数组表
#define RINFL_range 50 //结构表
#define FVL_range 50   //形参表
#define PFINFL_range 50//函数表
#define CONSL_range 50 //常数表 
#define LENL_range 50  //长度表
#define VALL_range 50  //活动记录表

/*********函数声明**************/
bool Program();
void Declaration();      //声明
bool VariableD();        //变量声明
bool ParameterList();    //外部声明参数表
bool ParameterList2();
bool ArrayOrNot();		 //是否为数组
bool CompoundStatement();//复合语句
void CompoundStatement2();
void Assignment();       //赋值
bool ConditionalStatement();//if
bool ConditionalStatement2();
void ConditionalStatement3();
void ConditionalStatement4();
bool LoopStatement();    //while
bool InclusiveDA();      //内部声明及调用
bool InclusiveDA2();//
void InclusivePL();      //内部调用参数表
bool InclusivePL2();//
//--------------------------
bool LOGISTIC();         //逻辑表达式
bool LOGISTIC_TRANS_STATE1();
void LOGISTIC_TRANS_STATE2();
bool LOGISTIC_TRANS_STATE3();

void ARITHMETIC_TRANS_E();//算术表达式
void ARITHMETIC_TRANS_E1();
void ARITHMETIC_TRANS_T();
void ARITHMETIC_TRANS_T1();
bool ARITHMETIC_TRANS_F();

void Syntax_Initialize();

//函数声明语句插入符号表时使用
typedef struct
{
	string name;//函数形参名
	int flag;   //0时形参不是数组，>0时形参是数组，其值为数组元素个数
}name_define;

/*****************************

********基本数据结构定义*******

******************************/
//四元式存储结构
struct QUAT
{
	string op;  //操作符
	string e1;  //元素1，次栈顶
	string e2;  //元素2，栈顶
	string temp;//临时变量
};

struct RINFL_DEFINE;
struct AINFL_DEFINE;
struct PFINFL_DEFINE;
//类型表成员-类码
typedef enum 
{
	in,//整型 
	re,//实型
	ch,//字符型
	bo,//布尔型 
	ar,//数组型
	st //结构型
}TVAL_DEFINE;

//类型表成员-指针类型
struct TPOINT_DEFINE
{
	int flag;			//标志(0时指向数组表，1时指向结构表)
	AINFL_DEFINE *AINFL;//数组表类型指针
	RINFL_DEFINE *RINFL;//结构表指针
};

//符号表总表成员-地址定义类型
struct ADDR_DEFINE
{
	int flag;			   //标志(0时指向函数表，1时指向常数表，2时指向长度表，3时OFFSET值为偏移量(此时只存相对偏移量))
	PFINFL_DEFINE *PFINFL; //指向函数表
	double *CONSL;         //常数表
	int *LENL;             //长度表 
	int OFFSET;			   //(在活动记录表中相对区头的偏移量)
};

//符号表(vn vf也用于形参表)总表成员-种类
typedef enum 
{
	fu,	//函数
	co,	//常量
	ty,	//类型
	dn,	//域名
	va,	//变量
	vn,	//换名形参
	vf	//赋值形参
}CAT_DEFINE;

/*************************

****符号表数据结构定义*****

**************************/
//类型表
typedef struct 
{
	TVAL_DEFINE TVAL;     //类码
	TPOINT_DEFINE *TPOINT;//指针(指向0数组表、1结构表之一，其他成员定义为空，2基本数据类型时直接指向空)
}TYPEL_DEFINE, TYPELlist[TYPEL_range];

//数组表
typedef struct AINFL_DEFINE
{
	int LOW;		  //数组下界
	int UP;			  //数组上界
	TYPEL_DEFINE *CTP;//成分类型指针-指向类型表
	int CLEN;		  //成分(数组元素)类型长度
}AINFL_DEFINE, AINFLlist[AINFL_range];

//结构表
typedef struct RINFL_DEFINE
{
	string ID;		 //结构的域名-结构体成员名
	int OFF;		 //区距
	TYPEL_DEFINE *TP;//域成分类型指针-指向类型表
}RINFL_DEFINE, RINFLlist[RINFL_range];

//形参表
typedef struct 
{
	string NAME;						//形参名
	TYPEL_DEFINE *TYPE;					//形参数据类型-指向类型表
	CAT_DEFINE FV_TRANS_TYPE;		    //形参传递类型(只能为枚举值vn vf)
	int OFFSET;						    //在活动记录表中相对区头的偏移量,不适用时值为-1
}FVL_DEFINE, FVLlist[FVL_range];

//函数表
typedef struct PFINFL_DEFINE
{
	//int LEVEL;	//层次号-静态层次嵌套号
	string NAME;    //函数名
	int OFF;		//区距-相对区头位置
	int FN;			//函数参数个数
	FVL_DEFINE *FVL;//指针-指向形参表
	//ENTRY;		//入口地址？
}PFINFL_DEFINE, PFINFLlist[PFINFL_range];

//符号表总表
typedef struct 
{
	string NAME;		//名字
	TYPEL_DEFINE *TYPE; //类型-指向类型表
	CAT_DEFINE CAT;		//种类
	ADDR_DEFINE *ADDR;	//地址(指向函数表、常数表、长度表之一，其他成员值为空)
}SYMBL_DEFINE, SYMBLlist[SYMBL_range];

/*************************

********符号表系统********

**************************/
SYMBLlist SYMBL;   //符号表总表
TYPELlist TYPEL;   //类型表
AINFLlist AINFL;   //数组表
RINFLlist RINFL;   //结构表
FVLlist FVL;	   //形参表
PFINFLlist PFINFL; //函数表
double CONSL[CONSL_range];		   //常数表
int LENL[LENL_range];			   //长度表
string VALL[VALL_range];		   //类活动记录表，用于存放变量值

//符号表系统元素插入函数
//符号表系统输出
void printer();
//符号总表输出、内存初始化、插入
void SYMBL_printer();
bool SYMBL_initialize();
bool insert_SYMBL(string NAME, TYPEL_DEFINE *TYPE, CAT_DEFINE CAT, int flag, PFINFL_DEFINE *PFINFL, double *CONSL, int *LENL, int OFFSET);
//类型表输出、内存初始化、插入
void TYPEL_printer();
bool TYPEL_initialize();
bool insert_TYPEL(TVAL_DEFINE TVAL, int flag, AINFL_DEFINE *AINFL, RINFL_DEFINE *RINFL);
//数组表输出、内存初始化、插入
void AINFL_printer();
bool AINFL_initialize();
bool insert_AINFL(int LOW, int UP, TYPEL_DEFINE *CTP, int CLEN);
//结构表输出、内存初始化、插入
void RINFL_printer();
bool RINFL_initialize();
bool insert_RINFL(string ID, int OFF, TYPEL_DEFINE *TP);
//形参表输出、内存初始化、插入
void FVL_printer();
bool FVL_initialize();
bool insert_FVL(string NAME, TYPEL_DEFINE *TYPE, CAT_DEFINE FV_TRANS_TYPE, int TYPE_LENGTH);
//函数表输出、内存初始化、插入
void PFINFL_printer();
bool PFINFL_initialize();
bool insert_PFINFL(string NAME, int OFF, int FN, FVL_DEFINE *FVL);
//常数表输出、插入
void CONSL_printer();
bool insert_CONSL(double CONS);
//长度表输出、插入
void LENL_printer();
bool insert_LENL(int LEN);

/**********四元式相关函数************/
void QUATCreator();
string NEWT();
void if_QUATCreator();
void PrintQuat();
void else_QUATCreator();
void IE_QUATCreator();
void while_QUATCreator();
void DO_QUATCreator();
void WE_QUATCreator();
void ASIIN_QUATCreator();
void SIN_QUATCreator();
void To_function();

#endif //SYNTAX_ANALYZER_H
