#pragma once

// ��������
#define PERSONDATA  1
#define SCOREDATA   2

// ���ݽṹ����
typedef struct _PersonStruct
{
    TCHAR   m_szName[32];   // ����
    int     m_nAge;         // ����
    double  m_dMoney;       // ���
}PersonStruct, *PPersonStruct;

typedef struct _ScoreStruct
{
    double  m_dChinese;     // ����
    double  m_dMath;        // ��ѧ
    double  m_dEnglish;     // Ӣ��
}ScoreStruct, *PScoreStruct;