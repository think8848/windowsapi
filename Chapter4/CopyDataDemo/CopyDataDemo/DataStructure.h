#pragma once

// 常量定义
#define PERSONDATA  1
#define SCOREDATA   2

// 数据结构定义
typedef struct _PersonStruct
{
    TCHAR   m_szName[32];   // 姓名
    int     m_nAge;         // 年龄
    double  m_dMoney;       // 存款
}PersonStruct, *PPersonStruct;

typedef struct _ScoreStruct
{
    double  m_dChinese;     // 语文
    double  m_dMath;        // 数学
    double  m_dEnglish;     // 英语
}ScoreStruct, *PScoreStruct;