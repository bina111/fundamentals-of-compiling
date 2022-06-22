#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QString>
#include<QVector>
#include<QChar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //@字符用来表示空字符
    QChar e[500][500];                     //NFA边数组(最多有500个状态）（略掉了0行和0列的存储空间）
    int count1 = 0;						   //记录NFA状态数
    int start,endless;					   //记录NFA整个图的开始状态和结束状态
    QChar e2[100][100];                    //记录DFA边数组 ,省略掉了0行和0列的存储空间
    int count2 = 0;						   //记录DFA的状态数
    QVector<QVector<int>> dfa;			   //记录DFA顶点的集合
    QVector<QVector<int>> tempV;			//存放每个状态经过‘@’的集合
    QVector<QChar> num;					   //存放表达式的操作数
    QVector<int> temp;					   //存放临时状态的集合
    QVector<int> endS;					   //用于判断某个DFA的状态集合是否是结束状态
    int DFASmall[100][50];                 //最小化DFA的状态表(最多有100个状态）
    int count3 = 0;							//记录最小化DFA的状态数
    QVector<QVector<int>> statusVec;			//存放最小化DFA划分状态的集合

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();                //全局元素重新初始化
    QString addSign(QString exp);//由于正则表达式没有连接的元符号，为了方便根据优先级转成RPN，加入&作为连接
    QVector<QChar> changeToRPN(QString exp);   //转为逆波兰表示法
    template<typename  T> bool find(QVector<T> vt, T value);//用于查找某个元素是否在vector里面
    void changeToNFA(QVector<QChar> exp);     //将正则表达式转成NFA
    //求图的第一个邻接顶点
    int getFirstNeighbor(int vertex);
    //求图的其他邻接顶点
    int getNextNeighbor(int vertex,int w);
    //判断经过空操作数（‘@’）到达的状态集合（深搜）
    void getThroughNumV(int vertex, char ch, bool visit[]);
    //判断某个顶点有没有经过某个操作数到达另一个状态数，将其经过某个操作数到达的状态集合保存起来
    QVector<int> getSetThroughNum(int vertex, QChar ch);
    //两个vector求并集
    QVector<int> vectors_set_union(QVector<int> vt1,QVector<int> vt2);
    //NFA转DFA
    void changeToDFA();
    //求经过某个操作数后到达的状态集合是哪个
    int getStatus(int vertex, QChar ch);
    //判断某个状态属于终态还是非终态集合
    int judgeVec(int vec);
    //合并状态将到达s2状态的改为s1的函数
    void mergeStatus(int s1, int s2);
    //合并状态，将s状态删除掉
    void deleteStatus(int s);
    //从DFA转成最小化的DFA
    void changeToSmallDFA();
    //将最小化的DFA转成词法分析程序
    void changeToCode();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
