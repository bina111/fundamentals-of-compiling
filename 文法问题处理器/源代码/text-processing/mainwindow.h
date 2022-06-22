#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QChar>
#include<QVector>
#include<QMap>
namespace Ui {
class MainWindow;
}

struct rightNode{     //右部规则指针
    rightNode(QChar ch){
        rightCh = ch;
        next = NULL;
    }
    QChar rightCh;    //右部规则符号
    rightNode *next;  //指向下一个右部规则符号指针
};

struct textNode{
    textNode(QChar ch):leftCh(ch){
        next = NULL;
        right = NULL;
    }
    QChar leftCh;       //规则左部符号
    textNode *next;     //下一规则指针
    rightNode *right;    //右部规则指针
};

//文法存储结构
struct text_rule{
    text_rule(){
        textG = NULL;
    }
    QVector<QChar> non_terminalSet;         //存放非终结符号的集合
    QVector<QChar> terminalSet;             //存放终结符号的集合
    textNode *textG;                        //文法规则的开始结点
    QChar start;                            //文法开始符号
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QVector<QChar> upperCase;    //存放26个大写字母，将存在的已被使用的非终结符号从该表中删去,便于文法改造使用未使用的大写字母作为非终结符号
    text_rule t1;                   //定义了一个文法
    QMap<QChar,QVector<QChar>> firstMap; //存放非终结符号的first集合
    QMap<QChar,QVector<QChar>> followMap; //存放非终结符号的follow集合
    QMap<QChar,QVector<QChar>> tempMap; //用于临时存放
    QMap<QChar,int> judgeLeft;           //存放该非终结符号的左公因子是否被提取，1为已提取
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
    QVector<QChar> vectors_set_union(QVector<QChar> vt1,QVector<QChar> vt2); //求两个Vector的并集
    void simplify();  //化简规则，去除多余规则
    QVector<QChar> findTextRule(QChar leftCh,QChar rightFirst,bool f,QChar lf);  //求左部规则为leftCh，右部规则的第一个字符为rightFirst的右部文法规则
    void leftFactorChar(QChar ch);    //提取某个具体的非终结符号的左公因子
    void leftFactor(); //提取左公因子
    QVector<QVector<QChar>> findRight(QChar ch,QChar left);   //找非终结符号ch的所有右部并返回，left用于查看右部是否包含这个字母，以便决定是否要返回右部规则
    void deleteLeftRecursion();//消除左递归
    QVector<QChar> first(QChar ch);  //求first集合
    void follow(); //求follow集合
    void left_infer(); //最左推导
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
