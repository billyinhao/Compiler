#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>
#include "global.h"
#include "Lexical_Analyzer.h"

OutList tokenlist;
OutList classifylist;

CCons::CCons()
{
	int Pas_aut[8][5] = { 2, 0, 0, 0, 0,
		2, 3, 5, 8, 8,
		4, 0, 0, 0, 0,
		4, 0, 5, 8, 8,
		7, 0, 0, 6, 0,
		7, 0, 0, 0, 0,
		7, 0, 0, 8, 8,
		0, 0, 0, 0, 0 };   //状态转换矩阵
	int i, j;
	for (i = 0; i<8; i++)  //初始化自动机矩阵
		for (j = 0; j<5; j++)
			aut[i][j] = Pas_aut[i][j];
	ch = ' ';
}

void CCons::Error()
{
    logtxt << QStringLiteral("常数处理机发生错误！") << endl;
}

int CCons::map(char ch)
{
	int j;
	if (ch >= '0' && ch <= '9')
		j = 0;
	else if (ch == '.')
		j = 1;
	else if (ch == 'E' || ch == 'e')
		j = 2;
	else if (ch == '+' || ch == '-')
		j = 3;
	else
		j = 4;
	return j;
}

int CCons::find(int s, char ch)//s---当前状态；ch---当前符号
{
	int i, j;                  //行和列
	i = s - 1;                 //将s映射到行标记i
	j = map(ch);               //将ch映射到列标记j
	return aut[i][j];          //返回下一个状态值
}

void CCons::act(int s, char ch)
{
	switch (s)
	{
	case 1: n = 0; m = 0; p = 0; t = 0; e = 1; num = 0;     break;
	case 2: n = 10 * n + ch - 48;	                  break;
	case 3: t = 1;              	  	          break;
	case 4: n = 10 * n + ch - 48;    m++;           break;
	case 5: t = 1;             		          break;
	case 6: if (ch == '-') e = -1; 		          break;
	case 7: p = 10 * p + ch - 48;  		          break;
	case 8: num = n*pow(10, e*p - m);    break;//求10的次方
	}
}

double CCons::number(string value)//拼一个常数
{
	s = 1;
	act(s, ch);                   //执行q1
	while (s != 8)
	{
		ch = value[pos_in_value]; //读取当前符号到ch中
		pos_in_value++;
		s = find(s, ch);          //查状态表
		if (s == 0)
			break;
		act(s, ch);               //执行qs
	}
	if (s == 8)
	{
		pos_in_value = 0;
		return num;               //输出num
	}
	else
	{
		Error();
		return 0;				  //错误处理
	}
};

CCons ccc;//常数处理机对象

//词法分析
bool Lexical_Analyzer(string src)
{
    int state_now, state_before;    //状态
    char buf;                       //每次读入的字符
    const char *fptr=src.c_str();    //指向源文件字符串的指针

    if(!Read_List()) {
        logtxt<<QStringLiteral("程序参考文件缺失！")<<endl;
        return false;
    }

	state_now = 1;

    while ((buf = *fptr++) != '\0')
	{
		state_before = state_now;                //保存前一个状态
		state_now = state_machine(state_now, buf);//状态转换

		if (state_now == error_state)//发生错误
		{
            logtxt << QStringLiteral("当遇到 ‘") << buf << QStringLiteral("’ 时发生错误！") << endl;//遇到某字符时发生错误
			return false;
		}
		else if (state_now == blank_state)   //状态机开始即读到空格或换行
		{
			state_now = 1;
		}
		else if (state_now != end_state)     //未到终止状态
		{
			TokenList[tokenI].value += buf;
		}
		else if (state_now == end_state)     //到终止状态
		{
			TokenList[tokenI].code = state_to_code(state_before, TokenList[tokenI].value);//生成token序列码
			if (TokenList[tokenI].code == -1)
			{
                logtxt << QStringLiteral("识别到系统未定义的字符：") << TokenList[tokenI].value.c_str() << endl;
			}

			if (TokenList[tokenI].code == 3)//若遇到数字，完成常数识别器(字符串->数字)
			{
				CT_list[CT_list_I] = ccc.number(TokenList[tokenI].value);
				TokenList[tokenI].cosPoint = &CT_list[CT_list_I];
				CT_list_I++;
			}

            fptr--;

			tokenI++;
			if (tokenI >= TokenList_range)
			{
                logtxt << QStringLiteral("超出Token表范围！") << endl;//token表不够用，向用户报告
				return false;
			}
			state_now = 1;
		}
		else
		{
            logtxt << QStringLiteral("无法读到正常的状态！") << endl;//读到不正常的状态
			return false;
		}
	}
	state_before = state_now;
	TokenList[tokenI].code = state_to_code(state_before, TokenList[tokenI].value);//生成token序列码

	if (TokenList[tokenI].code == -1)
	{
        logtxt << QStringLiteral("识别到系统未定义的字符：") << TokenList[tokenI].value.c_str() << endl;
	}

	if (TokenList[tokenI].code == 3)//若遇到数字，完成常数识别器(字符串->数字)
	{
		CT_list[CT_list_I] = ccc.number(TokenList[tokenI].value);
		TokenList[tokenI].cosPoint = &CT_list[CT_list_I];
		CT_list_I++;
	}

	tokenI++;
    TokenList[tokenI].code = 111; //存入待识别符号串结束标记@
    TokenList[tokenI].value = '@';

	tokenI++;

	TokenList_printer();

	return true;
}

//词法分析有关
//生成token序列码
int state_to_code(int state_before, string token_value)
{
	int token_code = -1;

	if (state_before == 2)//关键字、标识符
	{
		for (int i = 0; i < KT_range; i++)
		{
			if (KT[i].compare(token_value) == 0)
			{
				token_code = i + 4;
				return token_code;
			}
		}
		token_code = 0;
	}
	else if ((state_before == 5) || (state_before == 3) || (state_before == 20)) token_code = 3;
	else if (state_before == 8) token_code = 1;
	else if (state_before == 11) token_code = 2;
	else if ((state_before == 13) || (state_before == 15) || (state_before == 17) || (state_before == 23) || (state_before == 25) || (state_before == 27))
	{//双字符界符
		for (int i = 0; i < DPT_range; i++)
		{
			if (DPT[i].compare(token_value) == 0)
			{
				token_code = i + 40;
				return token_code;
			}
		}
	}
	else if ((state_before == 12) || (state_before == 14) || (state_before == 16) || (state_before == 18) || (state_before == 28) || (state_before == 22))
	{//单字符界符
		for (int i = 0; i < SPT_range; i++)
		{
			if (SPT[i].compare(token_value) == 0)
			{
				token_code = i + 46;
				return token_code;
			}
		}
	}
	return token_code;
}
//字符转换成代号
int buf_to_num(char buf)
{
	int num;

	if ((buf >= 'a' && buf <= 'z') || (buf >= 'A' && buf <= 'Z')) num = 1;
	else if (buf >= '0' && buf <= '9') num = 2;
	else if (buf == '.') num = 3;
	else if (buf == ' ' || buf == '\n' || buf == '\t') num = 4;//空格或换行或tab
	else if (buf == 39) num = 5;//''
	else if (buf == 34) num = 6;//""
	else if (buf == '>') num = 7;
	else if (buf == '<') num = 8;
	else if (buf == '=') num = 9;
	else if (buf == '!') num = 10;
	else if (buf == '&') num = 11;
	else if (buf == '|') num = 12;
	else num = 13;//肯定为单字符界符

	return num;
}
//状态机函数
int state_machine(int state_now, char buf)
{
	int num, state_next;

	num = buf_to_num(buf);

	switch (state_now)
	{
	case 1:
		if (num == 1) state_next = 2;//读到字母
		else if (num == 2) state_next = 3;//数字
		else if (num == 5) state_next = 6;//''
		else if (num == 6) state_next = 9;//""
		else if (num == 7) state_next = 12;//>
		else if (num == 8) state_next = 14;//<
		else if (num == 9) state_next = 16;//=
		else if (num == 10) state_next = 22;//!
		else if (num == 11) state_next = 24;//&
		else if (num == 12) state_next = 26;//|
		else if (num == 13) state_next = 18; //单字符界符
		else if (num == 4) state_next = blank_state;//开始读到空格或换行
		else if (num == 3) state_next = 28;//stdio.h中的.
		else state_next = error_state;
		break;
	case 2:
		if ((num == 1) || (num == 2) || (buf == 95)) state_next = 2;//标识符字母开头-字母数字下划线组成！
		else state_next = 0;
		break;
	case 3://数字
		if (num == 2) state_next = 3; //整数
		else if (num == 3) state_next = 4;//小数
		else if (buf == 'e') state_next = 19;//e指数
		else if ((num == 4) || (num == 7) || (num == 8) || (num == 9) || (num == 10)||(num==13)) state_next = 0;//遇到界符或空格或换行停止
		else state_next = error_state;
		break;
	case 4:
		if (num == 2) state_next = 5;
		else state_next = error_state;
		break;
	case 5:
		if (num == 2) state_next = 5;
		else if (buf == 'e') state_next = 19;//e指数
		else if ((num == 4) || (num == 7) || (num == 8) || (num == 9) || (num == 10)||(num==13)) state_next = 0;//遇到界符或空格或换行停止
		else state_next = error_state;
		break;
	case 6:
		if (num == 1) state_next = 7;
		else state_next = error_state;
		break;
	case 7:
		if (num == 5) state_next = 8;
		else state_next = -1;
		break;
	case 8:
		state_next = 0;
		break;
	case 9:
        if (num == 1 || num == 2) state_next = 10;
		else state_next = -1;
		break;
	case 10:
        if (num == 1 || num == 2) state_next = 10;
		else if (num == 6) state_next = 11;
        else state_next = -1;
		break;
	case 11:
		state_next = 0;
		break;
	case 12:
		if (num == 9) state_next = 13;//>=
		else state_next = 0;
		break;
	case 13:
		state_next = 0;
		break;
	case 14:
		if (num == 9) state_next = 15;//<=
		else state_next = 0;
		break;
	case 15:
		state_next = 0;
		break;
	case 16:
		if (num == 9) state_next = 17;//==
		else state_next = 0;
		break;
	case 17:
		state_next = 0;
		break;
	case 18://单字符界符
		state_next = 0;
		break;
	case 19:
		if (num == 2) state_next = 20;
		else if ((buf == '+') || (buf == '-')) state_next = 21;
		else state_next = -1;
		break;
	case 20:
		if (num == 2) state_next = 20;
		else state_next = 0;
		break;
	case 21:
		if (num == 2) state_next = 20;
		else state_next = -1;
		break;
	case 22:
		if (num == 9) state_next = 23;
		else state_next = 0;
		break;
	case 23:
		state_next = 0;
		break;
	case 24:
		if (num == 11) state_next = 25;
		else state_next = 0;
		break;
	case 25:
		state_next = 0;
		break;
	case 26:
		if (num == 12) state_next = 27;
		else state_next = -1;//|是错误符号
		break;
    case 27:
        state_next = 0;
        break;
	case 28:
		state_next = 0;
		break;
	default:
        logtxt << endl << QStringLiteral("遇到无法识别的字符 ‘") << buf << QStringLiteral("’！") << endl;//遇到无法识别的字符
		break;
	}
	return state_next;
}
//从文件中读入KT/PT静态表
bool Read_List()
{
	char tmp_rl;
	int i = 0;

    FILE *rl = fopen("data/KT.dat", "r");     //文件指针，用于从文件中读入字符以生成表
    if(rl==NULL) return false;

	while ((tmp_rl = fgetc(rl)) != '@')
	{
		if (tmp_rl == '\n')
		{
			i++;
			continue;
		}
		KT[i].append(1, tmp_rl);
	}

	KT_range = i + 1;//确定关键字表大小

    rl = fopen("data/DPT.dat", "r"); //读双字符界符
    if(rl==NULL) return false;
	i = 0;

	while ((tmp_rl = fgetc(rl)) != '@')
	{
		if (tmp_rl == '\n')
		{
			i++;
			continue;
		}
		DPT[i].append(1, tmp_rl);
	}

	DPT_range = i + 1;//确定双字符界符表大小

    rl = fopen("data/SPT.dat", "r"); //读单字符界符
    if(rl==NULL) return false;
	i = 0;

	while ((tmp_rl = fgetc(rl)) != '@')
	{
		if (tmp_rl == '\n')
		{
			i++;
			continue;
		}
		SPT[i].append(1, tmp_rl);
	}

	SPT_range = i + 1;//确定单字符界符表大小

	fclose(rl);
    return true;
}

//生成动态符号表系统
void legend()
{
	int m, I = 0, c = 0, s = 0, C = 0, k = 0;

	for (int i = 0; TokenList[i].code != 111; i++)
	{
		if (TokenList[i].code == 0)//标识符
		{
			if (I == 0)
			{
				dynamic_iT[I].value = TokenList[i].value;
				dynamic_iT[I].code = 0;
				I++;
			}
			else
			{
				m = 0;
				for (int j = 0; j < I; j++)
				{
					if (TokenList[i].value.compare(dynamic_iT[j].value) != 0)
					{
						m++;
					}
				}
				if (m == I)
				{
					dynamic_iT[I].value = TokenList[i].value;
					dynamic_iT[I].code = I;
					I++;
				}
			}
		}

		if (TokenList[i].code == 1)//字符
		{
			if (c == 0)
			{
				dynamic_cT[c].value = TokenList[i].value;
				dynamic_cT[c].code = 0;
				c++;
			}
			else
			{
				m = 0;
				for (int j = 0; j < c; j++)
				{
					if (TokenList[i].value.compare(dynamic_cT[j].value) != 0)
					{
						m++;
					}
				}
				if (m == c)
				{
					dynamic_cT[c].value = TokenList[i].value;
					dynamic_cT[c].code = c;
					c++;
				}
			}
		}
		if (TokenList[i].code == 2)//字符串
		{
			if (s == 0)
			{
				dynamic_sT[s].value = TokenList[i].value;
				dynamic_sT[s].code = s;
				s++;
			}
			else
			{
				m = 0;
				for (int j = 0; j < s; j++)
				{
					if (TokenList[i].value.compare(dynamic_sT[j].value) != 0)
					{
						m++;
					}
				}
				if (m == s)
				{
					dynamic_sT[s].value = TokenList[i].value;
					dynamic_sT[s].code = s;
					s++;
				}
			}
		}
		if (TokenList[i].code == 3)//常数
		{
			if (C == 0)
			{
				dynamic_CT[C].value = TokenList[i].value;
				dynamic_CT[C].code = 0; C++;
			}
			else
			{
				m = 0;
				for (int j = 0; j < C; j++)
				{
					if (TokenList[i].value.compare(dynamic_CT[j].value) != 0)
					{
						m++;
					}
				}
				if (m == C)
				{
					dynamic_CT[C].value = TokenList[i].value;
					dynamic_CT[C].code = C;
					C++;
				}
			}
		}
		if (TokenList[i].code >= 4 && TokenList[i].code <= 39)//关键字
		{
			if (k == 0)
			{
				dynamic_kT[k].value = TokenList[i].value;
				dynamic_kT[k].code = 0;
				k++;
			}
			else
			{
				m = 0;
				for (int j = 0; j < k; j++)
				{
					if (TokenList[i].value.compare(dynamic_kT[j].value) != 0)
					{
						m++;
					}
				}
				if (m == k)
				{
					dynamic_kT[k].value = TokenList[i].value;
					dynamic_kT[k].code = k; k++;
				}
			}
		}
	}
    classifylist = new OutNode;
    if(classifylist==NULL) return;
    classifylist->next = NULL;
    OutList lastptr = classifylist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----标识符----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	for (int i = 0; i < I; i++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << dynamic_iT[i].value << '<' << dynamic_iT[i].code << '>';
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}
    newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("\n----字符----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	for (int i = 0; i < c; i++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << dynamic_cT[i].value << '<' << dynamic_cT[i].code << '>';
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}
    newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("\n----字符串----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	for (int i = 0; i < s; i++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << dynamic_sT[i].value << '<' << dynamic_sT[i].code << '>';
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}
    newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("\n----常数----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	for (int i = 0; i < C; i++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << dynamic_CT[i].value << '<' << dynamic_CT[i].code << '>';
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}
    newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("\n----关键字----\n");
    lastptr->next = newptr;
    lastptr = lastptr->next;
	for (int i = 0; i < k; i++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << dynamic_kT[i].value << '<' << dynamic_kT[i].code << '>';
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}

//Token序列码输出
void TokenList_printer()
{
    tokenlist = new OutNode;
    if(tokenlist==NULL) return;
    tokenlist->next = NULL;
    OutList lastptr = tokenlist;
    OutList newptr = new OutNode;
    newptr->next = NULL;
    newptr->str = QStringLiteral("----Token序列表----\n\n符号串  序列号");
    lastptr->next = newptr;
    lastptr = lastptr->next;

    for (int k = 0; k < tokenI - 1; k++)
	{
        stringstream stream;
        newptr = new OutNode;
        newptr->next = NULL;
        stream << TokenList[k].value << "   " << "<" << TokenList[k].code << ">";
        newptr->str = QString::fromStdString(stream.str());
        lastptr->next = newptr;
        lastptr = lastptr->next;
	}

	return;
}
