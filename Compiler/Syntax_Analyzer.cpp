#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iomanip>
#include <iostream>
#include <stack>
#include "global.h"
#include "Syntax_Analyzer.h"

/***********外部变量*************/
typedef struct 
{
	int code;			//类别码
	string value;		//单词内容
	double *cosPoint;   //指向常数表的指针
    SYMBL_DEFINE *point;//指向符号表总表的指针
}Token;

Token TokenList[TokenList_range];

/**********全局变量*************/
//四元式部分
stack <string> SYN;//符号栈
stack <string> SEM;//语义栈
QUAT quat[1000];   //四元式数组
int c = 0;         //四元素数组计数器
int ti = 0;        //临时变量标号
string sAON;       //数组元素压入语义栈时需要将identifier[number|identifier]作为一个整体压

OutList symbolist;
OutList typelist;
OutList ainflist;
OutList rinflist;
OutList fvlist;
OutList pfinflist;
OutList conslist;
OutList lenlist;
OutList quartlist;
OutList lastptr;

//语法分析部分
Token *pt = TokenList;
int flag_in_Grammer = 1;

//符号表部分
int VIRTUAL_VALL_OFFSET = 3;//活动记录表存入数据的首偏移地址
//符号表系统各表实际大小，各表填入一项数据则+1
int SYMBL_I = 0; //符号表总表
int TYPEL_I = 0; //类型表
int AINFL_I = 0; //数组表
int RINFL_I = 0; //结构表 未用到
int FVL_I = 0; 	 //形参表
int PFINFL_I = 0;//函数表
int CONSL_I = 0; //常数表
int LENL_I = 0;	 //长度表

//语法分析入口
bool Syntax_Analyzer()
{
    quartlist = new OutNode;
    if(quartlist==NULL) return false;
    quartlist->next = new OutNode;
    quartlist->next->str = QStringLiteral("----四元式序列----\n");
    quartlist->next->next = NULL;
    lastptr = quartlist->next;

	//类型表初始化-前四个元素为固定的基本数据类型in re ch bo
	insert_TYPEL(in, 2, NULL, NULL);
	insert_TYPEL(re, 2, NULL, NULL);
	insert_TYPEL(ch, 2, NULL, NULL);
	insert_TYPEL(bo, 2, NULL, NULL);

	if ((*pt).code == 64)
	{//#
		pt++;
		if ((*pt).code == 21)
		{//include
			pt++;
			if ((*pt).code == 48)
			{//<
				pt++;
				if ((*pt).code == 38)
				{//stdio
					pt++;
					if ((*pt).code == 65)
					{//.
						pt++;
						if ((*pt).value == "h")
						{//h
							pt++;
							if ((*pt).code == 47)
							{//>
								pt++;
								Program();
							}
                            else{ logtxt << QStringLiteral("预编译错误ERROR IN >！\n"); flag_in_Grammer = 0; }
						}
                        else{ logtxt << QStringLiteral("预编译错误ERROR IN h！\n"); flag_in_Grammer = 0; }
					}
                    else{ logtxt << QStringLiteral("预编译错误ERROR IN .！\n"); flag_in_Grammer = 0; }
				}
                else{ logtxt << QStringLiteral("预编译错误ERROR IN stdio！\n"); flag_in_Grammer = 0; }
			}
            else{ logtxt << QStringLiteral("预编译错误ERROR IN <！\n"); flag_in_Grammer = 0; }
		}
        else{ logtxt << QStringLiteral("预编译错误ERROR IN include！\n"); flag_in_Grammer = 0; }
	}
    else{ logtxt << QStringLiteral("预编译错误ERROR IN #！\n"); flag_in_Grammer = 0; }

    if (flag_in_Grammer == 1){
        printer();//打印输出符号表系统
        return true;
    }
	else
		return false;
}

bool Program()
{
    if ((*pt).value == "int" || (*pt).value == "float" || (*pt).value == "double" || (*pt).value == "char" || (*pt).value == "void")
    {
        pt++;
        if ((*pt).code == 0)
        {//标识符
            pt++;
            Declaration();
            if(Program()) return true;
        }
        else if ((*pt).value == "main")
        {//main函数2
            pt++;
            if ((*pt).value == "(")
            {
                pt++;
                if ((*pt).value == ")")
                {
                    pt++;
                    if (!CompoundStatement()) return false;//main函数后半部分入口
                    if(Program()) return true;
                }
                else{ logtxt << "ERROR3 IN Program!\n"; flag_in_Grammer = 0; return false; }
            }
            else{ logtxt << "ERROR2 IN Program!\n"; flag_in_Grammer = 0; return false; }
        }
        else{ logtxt << "ERROR1 IN Program!\n"; flag_in_Grammer = 0; return false; }
    }
    //else 遇空
    else return false;
    return false;
}

//sooooooo
void Declaration()
{
	stack <name_define> nameStack;//回溯过程中保存扫描到的函数参数名
	stack <string> typeStack;//回溯过程中保存扫描到的函数参数类型
	name_define name;//临时保存函数参数名,结构体内部标记flag若不为0，则该参数为数组名，flag为数组元素个数
	string type;//临时保存函数参数类型
	
	string NAME,TYPE;
	Token *ptTemp = NULL;//中间指针
    //bool absentFlag = true;//判断是否要插入符号表时使用
	int K;
	int TYPE_LENGTH;//形参类型占字节长度

	if ((*pt).value == "(")
	{
		pt++;
		if (ParameterList()) {//进入外部函数声明
			if ((*pt).value == ")")
			{
				pt++;
				if ((*pt).value == ";")
				{
					//有外部函数声明
					ptTemp = pt;

					//扫描出参数列表
					while ((*ptTemp).code != 57)//（
					{
						if ((*ptTemp).code == 0)//标识符
						{
							name.name = (*ptTemp).value;
							name.flag = 0;
							nameStack.push(name);
						}
                        else if (((*ptTemp).code == 7) || ((*ptTemp).code == 11) || ((*ptTemp).code == 17) || ((*ptTemp).code == 20))
						{//类型关键字
							typeStack.push((*ptTemp).value);
						}
						else if ((*ptTemp).code == 60)//]
						{
							ptTemp--;//数组元素个数
							name.flag = *((*ptTemp).cosPoint);
							ptTemp--;//[
							ptTemp--;//数组名
							name.name = (*ptTemp).value;
							nameStack.push(name);
						}

						ptTemp--;//回退一个
					}

					ptTemp--;//扫描到函数名
                    ptTemp->point = &SYMBL[SYMBL_I];//修改Token序列表指针
					NAME = (*ptTemp).value;//函数名

					ptTemp--;//扫描到函数返回值类型
					TYPE = (*ptTemp).value;
					if (TYPE == "int")
					{
						K = 0;
					}
					else if (TYPE == "char")
					{
						K = 2;
					}
					else if (TYPE == "float")
					{
						K = 1;
					}
					else if (TYPE == "double")
					{
						K = 1;
					}
					else if (TYPE == "void")
					{
						K = -1;
					}

					if (!nameStack.size())//函数无形参,则赋空指针
					{
						insert_PFINFL(NAME, 3, nameStack.size(), NULL);//数据插入函数表
					}
					else
					{
						insert_PFINFL(NAME, 3, nameStack.size(), &FVL[FVL_I]);//数据插入函数表
					}

					if (K == -1)//若函数返回值为空，则指向类型表的指针赋空
					{
                        insert_SYMBL(NAME, NULL, fu, 0, &PFINFL[PFINFL_I - 1], NULL, NULL, -1);//数据插入符号表
					}
					else
					{
                        insert_SYMBL(NAME, &TYPEL[K], fu, 0, &PFINFL[PFINFL_I - 1], NULL, NULL, -1);//数据插入符号表
					}

					//插入形参(每插到形参表一次，就要插入符号表一次)
					while (!nameStack.empty())//判断栈不为空
					{
						name = nameStack.top();//取栈中的形参名
						nameStack.pop();
						NAME = name.name;

						TYPE = typeStack.top();//取栈中形参类型
						typeStack.pop();

						if (TYPE == "int")
						{
							K = 0;
							TYPE_LENGTH = sizeof(int);
						}
						else if (TYPE == "char")
						{
							K = 1;
							TYPE_LENGTH = sizeof(char);
						}
						else if (TYPE == "float")
						{
							K = 2;
							TYPE_LENGTH = sizeof(float);
						}
						else if (TYPE == "double")
						{
							K = 3;
							TYPE_LENGTH = sizeof(double);
						}

						if (name.flag == 0)//形参非数组类型
						{
							insert_FVL(NAME, &TYPEL[K], vf, TYPE_LENGTH);//数据插入形参表
                            insert_SYMBL(NAME, &TYPEL[K], vf, 3, NULL, NULL, NULL, VIRTUAL_VALL_OFFSET - TYPE_LENGTH);//数据插入符号表
						}
						else
						{//形参为数组类型
							insert_AINFL(0, name.flag - 1, &TYPEL[K], TYPE_LENGTH);//数据插入数组表
							insert_TYPEL(ar, 0, &AINFL[AINFL_I - 1], NULL);//数据插入类型表
							insert_FVL(NAME, &TYPEL[TYPEL_I - 1], vf, TYPE_LENGTH);//数据插入形参表
                            insert_SYMBL(NAME, &TYPEL[TYPEL_I - 1], vf, 3, NULL, NULL, NULL, VIRTUAL_VALL_OFFSET - TYPE_LENGTH);//数据插入符号表
						}
					}

					pt++;
				}
			}
		}
	}
	else
	{
        if (!ArrayOrNot()) return;//外部数组声明
        Assignment();//外部变量声明-有可能初始化
        if(!VariableD()) return;//多个外部变量或数组
	}
    //Program();
	return;
}

bool VariableD()
{
	if ((*pt).value == ",")
	{
		pt++;
		if ((*pt).code == 0)
		{
			pt++;
            if(!ArrayOrNot()) return false;
            Assignment();
            if (!VariableD()) return false;
		}
        else{ logtxt << "ERROR1 IN VaraibleD\n"; flag_in_Grammer = 0; return false; }
	}
	else if ((*pt).value == ";")
	{
		pt++;
	}
    else{
        logtxt << "ERROR2 IN VariableD!" << endl;
        flag_in_Grammer = 0;
    }
	//Program();
	return true;
}

bool ParameterList()
{
	if ((*pt).value == "int" || (*pt).value == "float" || (*pt).value == "double" || (*pt).value == "char")
	{//参数列表
		pt++;
		if ((*pt).code == 0)
		{
			pt++;
            if (!ArrayOrNot()) return false;
            if (!ParameterList2()) return false;
		}
        else{ logtxt << "ERROR1 IN ParameterList\n"; flag_in_Grammer = 0; return false; }
	}
	//遇空，无参数
	return true;
}

bool ParameterList2()
{
	if ((*pt).value == ",")
	{
		pt++;
		if ((*pt).value == "int" || (*pt).value == "float" || (*pt).value == "double" || (*pt).value == "char")
		{
			pt++;
			if ((*pt).code == 0)
			{
				pt++;
                if (!ArrayOrNot()) return false;
                if (!ParameterList2()) return false;
			}
		}
        else{ logtxt << "ERROR IN ParameterList2"; flag_in_Grammer = 0; return false; }
	}
	//遇空
	return true;
}
//SOOOOO
bool ArrayOrNot()
{
	//符号表部分
	int UP,CLEN,LEN;
	TYPEL_DEFINE *CTP = NULL;
	string NAME;
	Token *ptTemp = NULL;//中间指针
	bool absentFlag = true;//判断是否要插入符号表时使用

	//四元式部分
	sAON = (*(pt - 1)).value;//暂存标识符

	if ((*pt).value == "[")
	{
		sAON += (*pt).value;//把[连接进去
		pt++;
		if ((*pt).code == 3)
		{
			sAON += (*pt).value;//把number|identifier连接进去
			pt++;
			if ((*pt).value == "]")
			{
				sAON += (*pt).value;//把]连接进去

				//识别到数组,插入符号表
				ptTemp = pt;
				
				UP = *((*(--ptTemp)).cosPoint) - 1;//数组上界

				NAME = (*(--(--ptTemp))).value;//数组名
                for (int k = 0; k < SYMBL_I;k++)
				{
                    if ((SYMBL[k].NAME == NAME) && (SYMBL[k].CAT == ty))
					{					
						absentFlag = false;//符号表中已经存在该数组
                        ptTemp->point = &SYMBL[k];//修改Token序列表指针
						break;
					}
				}

				if (absentFlag == true)//如果符号表中不存在当前数组则插入
				{
                    ptTemp->point = &SYMBL[SYMBL_I];//修改Token序列表指针

					//一直回溯到第一个基本数据类型时，即为数组元素数据类型
					while (((*ptTemp).code != 7) && ((*ptTemp).code != 11) && ((*ptTemp).code != 17) && ((*ptTemp).code != 20))
					{
						ptTemp--;
					}

					if ((*ptTemp).value == "int")
					{
						CTP = &TYPEL[0];//整型
						CLEN = sizeof(int);//数组元素占字节数
					}
					else if ((*ptTemp).value == "char")
					{
						CTP = &TYPEL[2];//字符型
						CLEN = sizeof(char);
					}
					else if ((*ptTemp).value == "float")
					{
						CTP = &TYPEL[1];//实型float
						CLEN = sizeof(float);
					}
					else if ((*ptTemp).value == "double")
					{
						CTP = &TYPEL[1];//实型double
						CLEN = sizeof(double);
					}

					LEN = (UP + 1) * CLEN;//数组占字节数

					insert_LENL(LEN);//数据插入长度表
					insert_AINFL(0, UP, CTP, CLEN);//数据插入数组表
					insert_TYPEL(ar, 0, &AINFL[AINFL_I - 1], NULL);//数据插入类型表
                    insert_SYMBL(NAME, &TYPEL[TYPEL_I - 1], ty, 2, NULL, NULL, &LENL[LENL_I - 1], -1);//数据插入符号表
				}
				
				pt++;//SOOOOO
			}
            else{ logtxt << "ERROR1 IN ArrayOrNot!\n"; flag_in_Grammer = 0; return false; }
		}
        else{ logtxt << "ERROR2 IN ArrayOrNot!\n"; flag_in_Grammer = 0; return false; }
	}
	return true;
}
//SOOOOO
void Assignment()
{
	string NAME;
	TYPEL_DEFINE *CTP = NULL;
	Token *ptTemp = NULL;//中间指针
	bool absentFlag = true;//判断是否要插入符号表时使用

	SEM.push(sAON);

	if ((*pt).value == "=")
	{
		ptTemp = pt;
		NAME = (*(--ptTemp)).value;//变量名
        for (int k = 0; k < SYMBL_I; k++)
		{
            if ((SYMBL[k].NAME == NAME) && (SYMBL[k].CAT == va))
			{
				absentFlag = false;//符号表中已经存在该变量
                ptTemp->point = &SYMBL[k];//修改Token序列表指针
				break;
			}
		}

		pt++;
		ARITHMETIC_TRANS_E();//算术表达式判断结束时已经读入下一个符号-分号或逗号
		ASIIN_QUATCreator();//ASIIN四元式

		//识别到有赋值的变量
		if (absentFlag == true)//如果符号表中不存在当前数组则插入
		{
            ptTemp->point = &SYMBL[SYMBL_I];//修改Token序列表指针

			//一直回溯到第一个基本数据类型时，即为当前变量的数据类型
			while (((*ptTemp).code != 7) && ((*ptTemp).code != 11) && ((*ptTemp).code != 17) && ((*ptTemp).code != 20))
			{
				ptTemp--;
			}

			if ((*ptTemp).value == "int")
			{
				CTP = &TYPEL[0];//整型
			}
			else if ((*ptTemp).value == "char")
			{
				CTP = &TYPEL[2];//字符型
			}
			else if ((*ptTemp).value == "float")
			{
				CTP = &TYPEL[1];//实型float
			}
			else if ((*ptTemp).value == "double")
			{
				CTP = &TYPEL[1];//实型double
			}
			ptTemp = pt;//识别完算术表达式后pt指向；或是，
			ptTemp--;
			if ((*ptTemp).code == 3)//如果变量有初始 常数!!!! 值,则存入常数表
			{
				insert_CONSL(*((*ptTemp).cosPoint));
				CONSL_I--;
			}
			
			insert_TYPEL(CTP->TVAL, 2, NULL, NULL);//数据插入类型表

			//变量的值是指向常数表的
            insert_SYMBL(NAME, &TYPEL[TYPEL_I - 1], va, 1, NULL, &CONSL[CONSL_I], NULL, -1);//数据插入符号表
			CONSL_I++;
		}
		
		//SOOOOO
	}
	else//识别到未赋值的变量,插入符号表
	{	
		ptTemp = pt;

		NAME = (*(--ptTemp)).value;//变量名

		if (NAME != "]")//避免一种情况，即识别完数组没有"="而到此处，产生错误判断
		{
            for (int k = 0; k < SYMBL_I; k++)
			{
                if ((SYMBL[k].NAME == NAME) && (SYMBL[k].CAT == va))
				{
					absentFlag = false;//符号表中已经存在该变量
                    ptTemp->point = &SYMBL[k];//修改Token序列表指针
					break;
				}
			}

			if (absentFlag == true)//如果符号表中不存在当前数组则插入
			{
                ptTemp->point = &SYMBL[SYMBL_I];//修改Token序列表指针

				//一直回溯到第一个基本数据类型时，即为当前变量的数据类型
				while (((*ptTemp).code != 7) && ((*ptTemp).code != 11) && ((*ptTemp).code != 17) && ((*ptTemp).code != 20))
				{
					ptTemp--;
				}

				if ((*ptTemp).value == "int")
				{
					CTP = &TYPEL[0];//整型
				}
				else if ((*ptTemp).value == "char")
				{
					CTP = &TYPEL[2];//字符型
				}
				else if ((*ptTemp).value == "float")
				{
					CTP = &TYPEL[1];//实型double float
				}
				else if ((*ptTemp).value == "double")
				{
					CTP = &TYPEL[1];//实型double float
				}

				insert_TYPEL(CTP->TVAL, 2, NULL, NULL);//数据插入类型表

				//变量的值是指向常数表的
                insert_SYMBL(NAME, &TYPEL[TYPEL_I - 1], va, 1, NULL, &CONSL[CONSL_I], NULL, -1);//数据插入符号表
			}
		}
	}

	return;
}

bool ConditionalStatement()
{
	if ((*pt).value == "(")
	{
		pt++;
        if (!LOGISTIC()) return false;
        if ((*pt).value == ")")
        {
            if_QUATCreator();//生成if的四元式
            pt++;
            if (!ConditionalStatement2()) return false;
        }
        else
        {
            logtxt << QStringLiteral("分析条件语句发生错误ERROR2。") << endl;
            flag_in_Grammer = 0;
            return false;
        }
	}
	else
	{
        logtxt << QStringLiteral("分析条件语句发生错误ERROR1。") << endl;
		flag_in_Grammer = 0;
		return false;
	}

	return true;
}

bool ConditionalStatement2()
{
	if ((*pt).value == "{")
	{
		pt++;
        if (!InclusiveDA()) return false;
        if ((*pt).value == "}")
        {
            pt++;
            ConditionalStatement3();
        }
        else { logtxt << "ERROR1 IN ConditionalStatement2\n"; flag_in_Grammer = 0; return false; }
	}
    else if ((*pt).value == ";") pt++;
    else{ logtxt << "ERROR0 IN ConditionalStatement2\n"; flag_in_Grammer = 0; return false; }
	return true;
}

void ConditionalStatement3()
{
	if ((*pt).value == "else")
	{
		else_QUATCreator();//生成else四元式
		pt++;
		ConditionalStatement4();
		IE_QUATCreator();//IE四元式
	}
	//遇空
	return;
}

void ConditionalStatement4()
{
	if ((*pt).value == "{")
	{
		pt++;
		if (InclusiveDA()) {
			if ((*pt).value == "}")
			{
				pt++;
			}
		}
	}
	return;
}

bool LoopStatement()
{
	if ((*pt).value == "(")
	{
		pt++;
        if (!LOGISTIC()) return false;
        if ((*pt).value == ")")
        {
            DO_QUATCreator();//do四元式
            pt++;
            if ((*pt).value == "{")
            {
                pt++;
                if (InclusiveDA()) {
                    if ((*pt).value == "}")
                    {
                        WE_QUATCreator();//we四元式
                        pt++;
                    }
                else { logtxt << "ERROR4 IN LoopStatement\n"; flag_in_Grammer = 0; return false; }
                }
            }
            else { logtxt << "ERROR3 IN LoopStatement\n"; flag_in_Grammer = 0; return false; }
        }
        else { logtxt << "ERROR2 IN LoopStatement\n"; flag_in_Grammer = 0; return false; }
	}
    else{ logtxt << "ERROR1 IN LoopStatement\n"; flag_in_Grammer = 0; return false; }
	return true;
}

bool CompoundStatement()
{
	if ((*pt).value == "{")
	{
		pt++;
		CompoundStatement2();
		if ((*pt).value == "}")
			pt++;
        else{ logtxt << QStringLiteral("缺少右大括号‘}’\n"); flag_in_Grammer = 0; return false; }

	}
	return true;
}

void CompoundStatement2()
{
	if ((*pt).value == "if")
	{
		pt++;
		if (ConditionalStatement()) {
			CompoundStatement2();
		}
    }
	else if ((*pt).value == "while")
	{
		while_QUATCreator();//while四元式
		pt++;
        if (!LoopStatement()) return;
        CompoundStatement2();
	}
	else if ((*pt).value == "return")
	{
		pt++;
		if ((*pt).value == ";")
			pt++;
	}
	else
	{
        if (!InclusiveDA()) return;
        CompoundStatement2();
	}
}

bool InclusiveDA()
{
	if ((*pt).value == "int" || (*pt).value == "float" || (*pt).value == "double" || (*pt).value == "char")
	{
		pt++;
		if (!InclusiveDA()) return false;
	}
	else if ((*pt).code == 0)
	{
		pt++;
		if (!InclusiveDA2()) return false;
	}
	//遇空
	return true;
}
//soooooooo
bool InclusiveDA2()
{
	Token *ptTemp = NULL;//中间指针
	string NAME;//临时保存函数名

	if ((*pt).value == "(")
	{
		SEM.push((*(pt - 1)).value);//将要调用的函数名压入语义栈
		pt++;
		InclusivePL();
		if ((*pt).value == ")")
		{
			pt++;
			if ((*pt).value == ";")
			{
				To_function();

				//识别到函数调用,只是判断符号表中是否存在当前函数,若不存在则报错
				ptTemp = pt;

				//扫描出所调用的函数名
				while ((*ptTemp).code != 57)//（
				{
					ptTemp--;//回退一个
				}
				ptTemp--;//回退一个
				NAME = (*ptTemp).value;//查出扫描到的函数名

                for (int k = 0; k < SYMBL_I; k++)
				{
                    if ((SYMBL[k].NAME == NAME) && (SYMBL[k].CAT == fu))
					{
						break;
					}
                    if (k == (SYMBL_I - 1))
					{
                        logtxt << QStringLiteral("调用的函数") << NAME.c_str() << QStringLiteral("未声明！")<< endl;
						return false;
					}
				}

				pt++;
				InclusiveDA();
			}
            else{ logtxt << "ERROR2 IN InclusiveDA2\n"; flag_in_Grammer = 0; return false; }
		}
        else{ logtxt << "ERROR1 IN InclusiveDA2\n"; flag_in_Grammer = 0; return false; }
	}
	else
	{
        if (!ArrayOrNot()) return false;
        Assignment();
        if (!VariableD()) return false;
        if (!InclusiveDA()) return false;
	}
	return true;
}

void InclusivePL()
{
	if ((*pt).code == 0)
	{
		pt++;
		if (ArrayOrNot()) {
			InclusivePL2();
		}
	}
	//遇空
	return;
}

bool InclusivePL2()
{
	if ((*pt).value == ",")
	{
		pt++;
		if ((*pt).code == 0)
		{
			pt++;
            if (!ArrayOrNot()) return false;
            if (!InclusivePL2()) return false;
		}
        else { logtxt << "ERROR1 IN InclusivePL2\n"; flag_in_Grammer = 0; return false; }
	}
	//遇空
	return true;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//逻辑表达式
bool LOGISTIC()
{
	if ((*pt).code == 57)//(
	{
		pt++;
        if (!LOGISTIC()) return false;
        if ((*pt).code == 58)//)
        {
            pt++;
            if ((((*pt).code >= 40) && ((*pt).code <= 45)) || ((*pt).code == 47) || ((*pt).code == 48))//双目运算符BIN_OPERATOR
            {
                SYN.push((*pt).value);//压入符号栈
                pt++;
                if ((*pt).code == 57)//(
                {
                    pt++;
                    if (!LOGISTIC()) return false;
                    if ((*pt).code == 58)//)
                    {
                        QUATCreator();//逻辑表达式双目运算四元式，相当于算数运算，所以用同一个函数
                        pt++;
                    }
                    else
                    {
                        logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR4。") << endl; flag_in_Grammer = 0;
                        return false;
                    }
                }
                else
                {
                    logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR3。") << endl; flag_in_Grammer = 0;
                    return false;
                }
            }
            else
            {
                logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR2。") << endl; flag_in_Grammer = 0;
                return false;
            }
        }
        else
        {
            logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR1。") << endl; flag_in_Grammer = 0;
            return false;
        }
    }
	else
	{
		if ((*pt).code == 61)//单目运算符SIN_OPERATOR
		{
			SYN.push((*pt).value);//压符号栈
			pt++;
			if (LOGISTIC_TRANS_STATE1()) {
				SIN_QUATCreator();//单目运算四元式
			}
		}
		else
		{
			if ((*pt).code == 0)//标识符IDENTIFIER
			{
				SEM.push((*pt).value);//压入语义栈
				pt++;
				LOGISTIC_TRANS_STATE2();
			}
			else
			{
				if ((*pt).code == 3)//数字NUMBER
				{
					SEM.push((*pt).value);//压入语义栈
					pt++;
					LOGISTIC_TRANS_STATE2();
				}
				else
				{
                    logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR5。") << endl; flag_in_Grammer = 0;
					return false;
				}
			}
		}
	}
	return true;
}


bool LOGISTIC_TRANS_STATE1()
{
	if ((*pt).code != 0)//标识符IDENTIFIER
	{
		if ((*pt).code != 3)//数字NUMBER
		{
			if ((*pt).code == 57)//(
			{
				pt++;
                if (!LOGISTIC()) return false;
                if ((*pt).code != 58)//)
                {
                    logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR7(TRANS_STATE1)。") << endl; flag_in_Grammer = 0;
                    return false;
                }
			}
			else
			{
                logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR6(TRANS_STATE1)。") << endl; flag_in_Grammer = 0;
				return false;
			}
		}
	}
	SEM.push((*pt).value);//压入语义栈
	pt++;
	return true;
}

void LOGISTIC_TRANS_STATE2()
{
	if ((((*pt).code >= 40) && ((*pt).code <= 45)) || ((*pt).code == 47) || ((*pt).code == 48))//双目运算符BIN_OPERATOR
	{
		SYN.push((*pt).value);//压入符号栈
		pt++;
		if (LOGISTIC_TRANS_STATE3()) {
			QUATCreator();
		}
	}

	return;
}

bool LOGISTIC_TRANS_STATE3()
{
	if ((*pt).code != 0)//标识符IDNTIFIER
	{
		if ((*pt).code != 3)//数字NUMBER
		{
            logtxt << QStringLiteral("分析逻辑表达式发生错误ERROR8(TRANS_STATE3)。") << endl; flag_in_Grammer = 0;
			return false;
		}
	}
	SEM.push((*pt).value);//压入语义栈
	pt++;
	return true;
}
//算术表达式
void ARITHMETIC_TRANS_E()
{
	ARITHMETIC_TRANS_T();
	ARITHMETIC_TRANS_E1();

	return;
}

void ARITHMETIC_TRANS_T()
{
	if (ARITHMETIC_TRANS_F()) {
		ARITHMETIC_TRANS_T1();
	}

	return;
}

void ARITHMETIC_TRANS_E1()
{
	if (((*pt).code == 49) || ((*pt).code == 50))
	{
		SYN.push((*pt).value);//压入符号栈
		pt++;
		ARITHMETIC_TRANS_T();
		QUATCreator();//生成四元式
		ARITHMETIC_TRANS_E1();
	}

	return;
}

void ARITHMETIC_TRANS_T1()
{
	if (((*pt).code == 51) || ((*pt).code == 52))
	{
		SYN.push((*pt).value);
		pt++;
		if (ARITHMETIC_TRANS_F()) {
			QUATCreator();
			ARITHMETIC_TRANS_T1();
		}
	}

	return;
}

bool ARITHMETIC_TRANS_F()
{
	if (((*pt).code == 3) || ((*pt).code == 0))//读到常数或变量（此处放宽为标识符）
	{
		SEM.push((*pt).value);//压入语义栈
		pt++;
	}
	else
	{
		if ((*pt).code == 57)//(
		{
			pt++;
			ARITHMETIC_TRANS_E();
			if ((*pt).code == 58)//)
			{
				pt++;
			}
			else
			{
                logtxt << QStringLiteral("分析算术表达式发生错误ERROR2(ARITHMETIC_TRANS_F)") << endl;
				return false;
			}
		}
		else
		{
            logtxt << QStringLiteral("分析算术表达式发生错误ERROR2(ARITHMETIC_TRANS_F)") << endl;
			return false;
		}
	}

	return true;
}

//符号表系统生成函数
//符号总表插入
bool insert_SYMBL(string NAME, TYPEL_DEFINE *TYPE, CAT_DEFINE CAT, int flag, PFINFL_DEFINE *PFINFL, double *CONSL, int *LENL, int OFFSET)
{
    if (!SYMBL_initialize())
	{
        logtxt << QStringLiteral("符号表初始化内存失败！") << endl;
		return false;
	}
	
    if (SYMBL_I < SYMBL_range)
	{
        SYMBL[SYMBL_I].NAME = NAME;
        SYMBL[SYMBL_I].TYPE = TYPE;
        SYMBL[SYMBL_I].CAT = CAT;

		if (flag == 0)//指向函数表
		{
            SYMBL[SYMBL_I].ADDR->flag = 0;
            SYMBL[SYMBL_I].ADDR->PFINFL = PFINFL;
            SYMBL[SYMBL_I].ADDR->CONSL = NULL;
            SYMBL[SYMBL_I].ADDR->LENL = NULL;
            SYMBL[SYMBL_I].ADDR->OFFSET = -1;
		}
		else if (flag == 1)//指向常数表
		{
            SYMBL[SYMBL_I].ADDR->flag = 1;
            SYMBL[SYMBL_I].ADDR->PFINFL = NULL;
            SYMBL[SYMBL_I].ADDR->CONSL = CONSL;
            SYMBL[SYMBL_I].ADDR->LENL = NULL;
            SYMBL[SYMBL_I].ADDR->OFFSET = -1;
		}
		else if (flag == 2)//指向长度表
		{
            SYMBL[SYMBL_I].ADDR->flag = 2;
            SYMBL[SYMBL_I].ADDR->PFINFL = NULL;
            SYMBL[SYMBL_I].ADDR->CONSL = NULL;
            SYMBL[SYMBL_I].ADDR->LENL = LENL;
            SYMBL[SYMBL_I].ADDR->OFFSET = -1;
		}
		else if (flag == 3)//存入在活动记录表中的偏移量
		{
            SYMBL[SYMBL_I].ADDR->flag = 3;
            SYMBL[SYMBL_I].ADDR->PFINFL = NULL;
            SYMBL[SYMBL_I].ADDR->CONSL = NULL;
            SYMBL[SYMBL_I].ADDR->LENL = NULL;
            SYMBL[SYMBL_I].ADDR->OFFSET = OFFSET;
		}
		else
		{
            logtxt << QStringLiteral("flag值有误！") << endl;
		}

        SYMBL_I++;
	}

    if (SYMBL_I >= SYMBL_range)
	{
        logtxt << QStringLiteral("符号总表表空间使用完！") << endl;
		return false;
	}

	return true;
}
//类型表插入
bool insert_TYPEL(TVAL_DEFINE TVAL, int flag, AINFL_DEFINE *AINFL, RINFL_DEFINE *RINFL)
{
	if (!TYPEL_initialize())
	{
        logtxt << QStringLiteral("类型表初始化内存失败！") << endl;
		return false;
	}

	if (TYPEL_I<TYPEL_range)
	{
		TYPEL[TYPEL_I].TVAL = TVAL;//整型
		if (flag == 0)
		{
			TYPEL[TYPEL_I].TPOINT->flag = 0;
			TYPEL[TYPEL_I].TPOINT->AINFL = AINFL;
			TYPEL[TYPEL_I].TPOINT->RINFL = NULL;
		}
		else if (flag == 1)
		{
			TYPEL[TYPEL_I].TPOINT->flag = 1;
			TYPEL[TYPEL_I].TPOINT->AINFL = NULL;
			TYPEL[TYPEL_I].TPOINT->RINFL = RINFL;
		}
		else if (flag == 2)
		{
			TYPEL[TYPEL_I].TPOINT->AINFL = NULL;
			TYPEL[TYPEL_I].TPOINT->RINFL = NULL;
			(*(TYPEL[TYPEL_I].TPOINT)).flag = 2;
		}
		else
		{
            logtxt << QStringLiteral("类型表flag值有误！") << endl;
		}


		TYPEL_I++;
	}

	if (TYPEL_I >= TYPEL_range)
	{
        logtxt << QStringLiteral("类型表空间使用完！") << endl;
		return false;
	}

	return true;
}
//数组表插入
bool insert_AINFL(int LOW, int UP, TYPEL_DEFINE *CTP, int CLEN)
{
	if (!AINFL_initialize())
	{
        logtxt << QStringLiteral("数组表初始化内存失败！") << endl;
		return false;
	}

	if (AINFL_I<AINFL_range)
	{
		AINFL[AINFL_I].LOW = LOW;
		AINFL[AINFL_I].UP = UP;
		AINFL[AINFL_I].CTP = CTP;
		AINFL[AINFL_I].CLEN = CLEN;

		AINFL_I++;
	}

	if (AINFL_I >= AINFL_range)
	{
        logtxt << QStringLiteral("数组表空间使用完！") << endl;
		return false;
	}

	return true;
}
//结构表插入
bool insert_RINFL(string ID, int OFF, TYPEL_DEFINE *TP)
{
	if (!RINFL_initialize())
	{
        logtxt << QStringLiteral("结构表初始化内存失败！") << endl;
		return false;
	}

	if (RINFL_I<RINFL_range)
	{
		RINFL[RINFL_I].ID = ID;
		RINFL[RINFL_I].OFF = OFF;
		RINFL[RINFL_I].TP = TP;
		RINFL_I++;
	}

	if (RINFL_I >= RINFL_range)
	{
        logtxt << QStringLiteral("结构表空间使用完！") << endl;
		return false;
	}

	return true;
}
//形参表插入
bool insert_FVL(string NAME, TYPEL_DEFINE *TYPE, CAT_DEFINE FV_TRANS_TYPE, int TYPE_LENGTH)
{
	if (!FVL_initialize())
	{
        logtxt << QStringLiteral("形参表初始化内存失败！") << endl;
		return false;
	}

	if (FVL_I<FVL_range)
	{
		FVL[FVL_I].NAME = NAME;
		FVL[FVL_I].TYPE = TYPE;
		FVL[FVL_I].FV_TRANS_TYPE = FV_TRANS_TYPE;
		FVL[FVL_I].OFFSET = VIRTUAL_VALL_OFFSET;
		VIRTUAL_VALL_OFFSET += TYPE_LENGTH;

		FVL_I++;
	}

	if (FVL_I >= FVL_range)
	{
        logtxt << QStringLiteral("形参表空间使用完！") << endl;
		return false;
	}

	return true;
}
//函数表插入
bool insert_PFINFL(string NAME, int OFF, int FN, FVL_DEFINE *FVL)
{
	if (!PFINFL_initialize())
	{
        logtxt << QStringLiteral("函数表初始化内存失败！") << endl;
		return false;
	}

	if (PFINFL_I<PFINFL_range)
	{
		//PFINFL[PFINFL_I].LEVEL = LEVEL;
		PFINFL[PFINFL_I].NAME = NAME;
		PFINFL[PFINFL_I].OFF = OFF;
		PFINFL[PFINFL_I].FN = FN;
		PFINFL[PFINFL_I].FVL = FVL;
		//PFINFL[PFINFL_I].ENTRY = ENTRY;//入口地址

		PFINFL_I++;
	}

	if (PFINFL_I >= PFINFL_range)
	{
        logtxt << QStringLiteral("函数表空间使用完！") << endl;
		return false;
	}

	return true;
}
//常数表插入
bool insert_CONSL(double CONS)
{
	if (CONSL_I<CONSL_range)
	{
		CONSL[CONSL_I] = CONS;

		CONSL_I++;
	}

	if (CONSL_I >= CONSL_range)
	{
        logtxt << QStringLiteral("常数表空间用完！") << endl;
		return false;
	}

	return true;
}
//长度表插入
bool insert_LENL(int LEN)
{
	if (LENL_I<LENL_range)
	{
		LENL[LENL_I] = LEN;

		LENL_I++;
	}

	if (LENL_I >= LENL_range)
	{
        logtxt << QStringLiteral("长度表空间用完！") << endl;
		return false;
	}

	return true;
}
//符号总表内存初始化
bool SYMBL_initialize()
{
    SYMBL[SYMBL_I].TYPE = (TYPEL_DEFINE *)malloc(sizeof(TYPEL_DEFINE));
    SYMBL[SYMBL_I].ADDR = (ADDR_DEFINE *)malloc(sizeof(ADDR_DEFINE));

    SYMBL[SYMBL_I].TYPE->TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));
    SYMBL[SYMBL_I].ADDR->PFINFL = (PFINFL_DEFINE *)malloc(sizeof(PFINFL_DEFINE));
    SYMBL[SYMBL_I].ADDR->CONSL = (double *)malloc(sizeof(double));
    SYMBL[SYMBL_I].ADDR->LENL = (int *)malloc(sizeof(int));

	return true;
}
//类型表内存初始化
bool TYPEL_initialize()
{
	TYPEL[TYPEL_I].TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));
	TYPEL[TYPEL_I].TPOINT->flag = (int)malloc(sizeof(int));
	TYPEL[TYPEL_I].TPOINT->RINFL = (RINFL_DEFINE *)malloc(sizeof(RINFL_DEFINE));
	TYPEL[TYPEL_I].TPOINT->AINFL = (AINFL_DEFINE *)malloc(sizeof(AINFL_DEFINE));

	return true;
}
//数组表内存初始化
bool AINFL_initialize()
{
	AINFL[AINFL_I].CTP = (TYPEL_DEFINE *)malloc(sizeof(TYPEL_DEFINE));
	AINFL[AINFL_I].CTP->TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));

	return true;
}
//结构表内存初始化
bool RINFL_initialize()
{
	RINFL[RINFL_I].TP = (TYPEL_DEFINE *)malloc(sizeof(TYPEL_DEFINE));
	RINFL[RINFL_I].TP->TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));

	return true;
}
//形参表内存初始化
bool FVL_initialize()
{
	FVL[FVL_I].TYPE = (TYPEL_DEFINE *)malloc(sizeof(TYPEL_DEFINE));
	FVL[FVL_I].TYPE->TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));

	return true;
}
//函数表内存初始化
bool PFINFL_initialize()
{
	PFINFL[PFINFL_I].FVL = (FVL_DEFINE *)malloc(sizeof(FVL_DEFINE));
	PFINFL[PFINFL_I].FVL->TYPE = (TYPEL_DEFINE *)malloc(sizeof(TYPEL_DEFINE));
	PFINFL[PFINFL_I].FVL->TYPE->TPOINT = (TPOINT_DEFINE *)malloc(sizeof(TPOINT_DEFINE));

	return true;
}
//符号表系统输出
void printer()
{
    SYMBL_printer();
	TYPEL_printer();
	AINFL_printer();
	RINFL_printer();
	FVL_printer();
	PFINFL_printer();
	CONSL_printer();
	LENL_printer();

	return;
}
//符号总表输出
void SYMBL_printer()
{
    symbolist = new OutNode;
    if(symbolist==NULL) return;
    symbolist->next = NULL;
    OutList lastptr = symbolist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----符号总表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;

    if (SYMBL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("符号  类型类码  种类  地址指向表");
        lastptr->next = newptr;
        lastptr = lastptr->next;
        for (int k = 0; k < SYMBL_I; k++)
		{
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            newptr->str = QString::fromStdString(SYMBL[k].NAME + "     ");

			//类码输出
            if (SYMBL[k].TYPE->TVAL == 0)
			{
                newptr->str += "in";
			}
            else if (SYMBL[k].TYPE->TVAL == 1)
			{
                newptr->str += "re";
			}
            else if (SYMBL[k].TYPE->TVAL == 2)
			{
                newptr->str += "ch";
			}
            else if (SYMBL[k].TYPE->TVAL == 3)
			{
                newptr->str += "bo";
			}
            else if (SYMBL[k].TYPE->TVAL == 4)
			{
                newptr->str += "ar";
			}
            else if (SYMBL[k].TYPE->TVAL == 5)
			{
                newptr->str += "st";
			}

            newptr->str += "     ";

			//种类输出
            if (SYMBL[k].CAT == 0)
			{
                newptr->str += "fu";
			}
            else if (SYMBL[k].CAT == 1)
			{
                newptr->str += "co";
			}
            else if (SYMBL[k].CAT == 2)
			{
                newptr->str += "ty";
			}
            else if (SYMBL[k].CAT == 3)
			{
                newptr->str += "dn";
			}
            else if (SYMBL[k].CAT == 4)
			{
                newptr->str += "va";
			}
            else if (SYMBL[k].CAT == 5)
			{
                newptr->str += "vn";
			}
            else if (SYMBL[k].CAT == 6)
			{
                newptr->str += "vf";
			}

            newptr->str += "       ";

            if (SYMBL[k].ADDR->flag == 0)
			{
                newptr->str += QStringLiteral("函数表");
			}
            else if (SYMBL[k].ADDR->flag == 1)
			{
                newptr->str += QStringLiteral("常数表");
			}
            else if (SYMBL[k].ADDR->flag == 2)
			{
                newptr->str += QStringLiteral("长度表");
			}
            else if (SYMBL[k].ADDR->flag == 3)
			{
                stream << SYMBL[k].ADDR->OFFSET;
                newptr->str += QStringLiteral("偏移量");
                newptr->str += QString::fromStdString(stream.str());
			}
			else
			{
                logtxt << QStringLiteral("地址指向表标志flag错误！") << endl;
			}
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}
//类型表输出
void TYPEL_printer()
{
    typelist = new OutNode;
    if(typelist==NULL) return;
    typelist->next = NULL;
    OutList lastptr = typelist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----类型表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (TYPEL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("类码  类型名称");
        lastptr->next = newptr;
        lastptr = lastptr->next;
		for (int k = 0; k < TYPEL_I; k++)
		{
            newptr = new OutNode;
            newptr->next = NULL;
			if (TYPEL[k].TVAL == 0)
			{
                newptr->str = "in    ";
			}
			else if (TYPEL[k].TVAL == 1)
			{
                newptr->str = "re    ";
			}
			else if (TYPEL[k].TVAL == 2)
			{
                newptr->str = "ch    ";
			}
			else if (TYPEL[k].TVAL == 3)
			{
                newptr->str = "bo    ";
			}
			else if (TYPEL[k].TVAL == 4)
			{
                newptr->str = "ar    ";
			}
			else if (TYPEL[k].TVAL == 5)
			{
                newptr->str = "st    ";
			}

			if (TYPEL[k].TPOINT->flag == 0)
			{
                newptr->str += QStringLiteral("数组表");
			}
			else if (TYPEL[k].TPOINT->flag == 1)
			{
                newptr->str += QStringLiteral("结构表");
			}
			else if (TYPEL[k].TPOINT->flag == 2)
			{
                newptr->str += QStringLiteral("基本数据类型");
			}
			else 
			{
                logtxt << QStringLiteral("类型标记flag错误！") << endl;
			}
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}
//数组表输出
void AINFL_printer()
{
    ainflist = new OutNode;
    if(ainflist==NULL) return;
    ainflist->next = NULL;
    OutList lastptr = ainflist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----数组表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (AINFL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("下界  上界  元素长度  元素类型类码");
        lastptr->next = newptr;
        lastptr = lastptr->next;
		for (int k = 0; k < AINFL_I; k++)
        {
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            stream << AINFL[k].LOW << "       " << AINFL[k].UP << "       " << AINFL[k].CLEN << "       " ;
            newptr->str = QString::fromStdString(stream.str());
			if (AINFL[k].CTP->TVAL == 0)
			{
                newptr->str += "in";
			}
			else if (AINFL[k].CTP->TVAL == 1)
			{
                newptr->str += "re";
			}
			else if (AINFL[k].CTP->TVAL == 2)
			{
                newptr->str += "ch";
			}
			else if (AINFL[k].CTP->TVAL == 3)
			{
                newptr->str += "bo";
			}
			else if (AINFL[k].CTP->TVAL == 4)
			{
                newptr->str += "ar";
			}
			else if (AINFL[k].CTP->TVAL == 5)
			{
                newptr->str += "st";
            }
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}
//结构表输出
void RINFL_printer()
{
    rinflist = new OutNode;
    if(rinflist==NULL) return;
    rinflist->next = NULL;
    OutList lastptr = rinflist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----结构表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (RINFL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("域名  区距  域成分类型类码");
        lastptr->next = newptr;
        lastptr = lastptr->next;
		for (int k = 0; k < RINFL_I; k++)
        {
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            stream << RINFL[k].ID << "  " << RINFL[k].OFF << "  " ;
            newptr->str = QString::fromStdString(stream.str());
			if (RINFL[k].TP->TVAL == 0)
			{
                newptr->str += "in";
			}
			else if (RINFL[k].TP->TVAL == 1)
			{
                newptr->str += "re";
			}
			else if (RINFL[k].TP->TVAL == 2)
			{
                newptr->str += "ch";
			}
			else if (RINFL[k].TP->TVAL == 3)
			{
                newptr->str += "bo";
			}
			else if (RINFL[k].TP->TVAL == 4)
			{
                newptr->str += "ar";
			}
			else if (RINFL[k].TP->TVAL == 5)
			{
                newptr->str += "st";
			}
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}

//形参表输出
void FVL_printer()
{
    fvlist = new OutNode;
    if(fvlist==NULL) return;
    fvlist->next = NULL;
    OutList lastptr = fvlist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----形参表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (FVL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("形参名    类型类码");
        lastptr->next = newptr;
        lastptr = lastptr->next;
		for (int k = 0; k < FVL_I; k++)
        {
            OutList newptr = new OutNode;
            newptr->next = NULL;
            newptr->str = QString::fromStdString(FVL[k].NAME + "          ");
			if (FVL[k].TYPE->TVAL == 0)
			{
                newptr->str += "in";
			}
			else if (FVL[k].TYPE->TVAL == 1)
			{
                newptr->str += "re";
			}
			else if (FVL[k].TYPE->TVAL == 2)
			{
                newptr->str += "ch";
			}
			else if (FVL[k].TYPE->TVAL == 3)
			{
                newptr->str += "bo";
			}
			else if (FVL[k].TYPE->TVAL == 4)
			{
                newptr->str += "ar";
			}
			else if (FVL[k].TYPE->TVAL == 5)
			{
                newptr->str += "st";
			}
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}

//函数表输出
void PFINFL_printer()
{
    pfinflist = new OutNode;
    if(pfinflist==NULL) return;
    pfinflist->next = NULL;
    OutList lastptr = pfinflist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----函数表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (PFINFL_I != 0)
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("函数名  参数个数  相对区头偏移量");
        lastptr->next = newptr;
        lastptr = lastptr->next;
		for (int k = 0; k < PFINFL_I; k++)
		{
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            stream << PFINFL[k].NAME << "     " << PFINFL[k].FN << "     " << PFINFL[k].OFF;
            newptr->str = QString::fromStdString(stream.str());
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}
//常数表输出
void CONSL_printer()
{
    conslist = new OutNode;
    if(conslist==NULL) return;
    conslist->next = NULL;
    OutList lastptr = conslist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----常数表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (CONSL_I != 0)
	{
		for (int k = 0; k < CONSL_I; k++)
		{
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            stream << CONSL[k] << "  ";
            newptr->str = QString::fromStdString(stream.str());
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}
	return;
}

//长度表输出
void LENL_printer()
{
    lenlist = new OutNode;
    if(lenlist==NULL) return;
    lenlist->next = NULL;
    OutList lastptr = lenlist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----长度表----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	if (LENL_I != 0)
	{
		for (int k = 0; k < LENL_I; k++)
		{
            stringstream stream;
            newptr = new OutNode;
            newptr->next = NULL;
            stream << LENL[k] << "  ";
            newptr->str = QString::fromStdString(stream.str());
            lastptr->next = newptr;
            lastptr = lastptr->next;
		}
	}
	else
	{
        newptr = new OutNode;
        newptr->next = NULL;
        newptr->str = QStringLiteral("为空！");
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}

/********************四元式生成相关函数********************************/
//基本双目运算符四元式生成
void QUATCreator()
{
	string temp = NEWT();
	quat[c].op = SYN.top();
	quat[c].e2 = SEM.top();//栈顶
	SEM.pop();
	quat[c].e1 = SEM.top();//次栈顶
	SEM.pop();
	SYN.pop();
	quat[c].temp = temp;
	SEM.push(temp);
	PrintQuat();
	c++;
}
/********if语句增加*********/
void if_QUATCreator()
{
	quat[c].op = "if";
	quat[c].e1 = SEM.top();
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
	SEM.pop();
}

void else_QUATCreator()
{
	quat[c].op = "el";
	quat[c].e1 = " ";
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
}

void IE_QUATCreator()
{
	quat[c].op = "ie";
	quat[c].e1 = " ";
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
}
/****************************/

/******while语句增加**********/
void while_QUATCreator()
{
	quat[c].op = "wh";
	quat[c].e1 = " ";
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
}

void DO_QUATCreator()
{
	quat[c].op = "do";
	quat[c].e1 = SEM.top();
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
	SEM.pop();
}

void WE_QUATCreator()
{
	quat[c].op = "we";
	quat[c].e1 = " ";
	quat[c].e2 = " ";
	quat[c].temp = " ";
	PrintQuat();
	c++;
}

/******赋值语句增加**********/
void ASIIN_QUATCreator()
{
	quat[c].op = "=";
	quat[c].e1 = SEM.top();//栈顶
	SEM.pop();
	quat[c].e2 = " ";
	quat[c].temp = SEM.top();//次栈顶
	SEM.pop();
	PrintQuat();
	c++;
}

/******逻辑表达式***********/
void SIN_QUATCreator()
{
	string temp = NEWT();
	quat[c].op = SYN.top();
	SEM.pop();
	quat[c].e1 = SEM.top();//次栈顶
	quat[c].e2 = " ";
	quat[c].temp = temp;
	SEM.pop();
	SYN.pop();
	SEM.push(quat[c].temp);
	PrintQuat();
	c++;
}

/******函数调用***********/

void To_function()
{
	quat[c].op = "To";
	quat[c].e1 = " ";
	quat[c].e1 = " ";
	quat[c].temp = SEM.top();
	SEM.pop();
	PrintQuat();
	c++;
}

string NEWT()
{
	char t[3];
	_itoa(ti, t, 10);//将ti转化为10进制字符串
	ti++;//临时变量标号+1
	string tx = "t" + string(t);
	return tx;
}

void PrintQuat()
{
    stringstream stream;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    stream << "<" << quat[c].op << "," << setw(2) << quat[c].e1 << "," << setw(2) << quat[c].e2 << "," << quat[c].temp << ">" << endl;
    newptr->str = QString::fromStdString(stream.str());
    lastptr->next = newptr;
    lastptr = lastptr->next;
}
