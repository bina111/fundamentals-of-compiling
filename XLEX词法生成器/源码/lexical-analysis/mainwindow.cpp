#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QString>
#include<QVector>
#include<QChar>
#include<QDebug>
#include<QStack>
#include<QFileDialog>
#include<QMessageBox>
#include<QFile>
#include<QTextStream>
#include<algorithm>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tab->setCurrentIndex(0);

    //退出程序
    connect(ui->pB_exit,&QPushButton::clicked,this,[=](){
        exit(0);
    });

    //将正则表达式进行转化
    connect(ui->pB_change,&QPushButton::clicked,this,[=](){
        init();         //将全局变量重新初始化
        QString exp = ui->lE_exp->text();
        QString exp2 = this->addSign(exp);
        QVector<QChar> vexp;
        vexp = changeToRPN(exp2);             //获取逆波兰表示的表达式
        changeToNFA(vexp);              //将表达式转化成NFA
        changeToDFA();                  //将NFA转成DFA
        changeToSmallDFA();             //转成最小化的DFA
        changeToCode();                 //转化成代码
    });

    //保存正则表示式
    connect(ui->pB_save,&QPushButton::clicked,this,[=](){
        QString exp = ui->lE_exp->text();
        if(exp == "")
        {
            QMessageBox::warning(this, tr("提示") ,tr("正则表达式为空，无法保存！"));
        }
        else{
            QFileDialog fileDialog;
            QString file_name = fileDialog.getSaveFileName(this,tr("Open File"),"/text",tr("Text File(*.txt)"));
            QFile file(file_name);
            if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
                return;
            QTextStream out(&file);
            QString exp = ui->lE_exp->text();
            out<<exp;
        }
    });

    //打开写有正则表达式的文本文件到编辑框
    connect(ui->pB_open,&QPushButton::clicked,this,[=](){
        QString file_name = QFileDialog::getOpenFileName(this,"获取正则表达式文本文件",".","*.txt");
        QFile file(file_name);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QString exp = in.readLine();
        ui->lE_exp->setText(exp);
    });
}

void MainWindow::init()             //对全局变量进行初始化
{
    for(int i=0;i<500;i++)
        for(int j=0;j<500;j++)
            e[i][j] = '\0';
    for(int i=0;i<100;i++)
        for(int j=0;j<100;j++)
            e2[i][j] = '\0';
    for(int i=0;i<100;i++)
        for(int j=0;j<50;j++)
            DFASmall[i][j] = 0;
    start = endless = count1 = count2 = count3 = 0;
    dfa.clear();
    tempV.clear();
    num.clear();
    temp.clear();
    endS.clear();
    statusVec.clear(); 
    ui->tableNFA->clearContents();
    ui->tableDFA->clearContents();
    ui->tableDFASmall->clearContents();
}

QString MainWindow::addSign(QString exp)//由于正则表达式没有连接的元符号，了方便根据优先级转成RPN，加入&作为连接
{
    QString exp2 = "";              //存放更改后的表达式
    for(int i=1;i<exp.length();i++)
    {
        exp2 = exp2 + exp[i-1];
        if((exp[i-1] != '|' && exp[i-1] != '(' && exp[i] == '(') || (exp[i-1]=='*' &&  exp[i]!='|' && exp[i]!=')') ||
                (exp[i-1] == ')' && (exp[i] != '|' && exp[i] !='*' && exp[i]!=')')) ||
                (exp[i-1]!=')' && exp[i-1]!='*' && exp[i-1]!='|' && exp[i-1]!='(' && exp[i]!=')' && exp[i]!='*' && exp[i]!='|' && exp[i]!='(')){
            exp2 = exp2 + '&';
        }  
    }
    exp2 = exp2 + exp[exp.length()-1];
    return exp2;
}

QVector<QChar> MainWindow::changeToRPN(QString exp)    //转为逆波兰表示法
{
    /*
         优先级从大到小排序为：(),*,&,|
         定义两个容器，一个放操作数，一个放运算符
         遇到操作数就直接压入num容器，遇到运算符则要分优先级处理，处理如下：
         遇到左括号，直接压入op容器；
         遇到右括号，把左括号前面的操作数弹出压入num容器；
         遇到*，直接压入容易，优先级较高，先运算
         遇到&，把优先级比&高或相等的运算符弹出到num容器里（注意左括号），再将自己压入num容器
         遇到|，把op容器里的运算符全压入num容器（若有左括号先将左括号后面的运算符压入num容器），再将自己压入num容器
    */
    QVector<QChar> num;
    QVector<QChar> op;
    for(int i=0;i<exp.length();i++){
        if(exp[i]=='('){
            op.push_back(exp[i]);
        }
        else if(!op.empty() && exp[i]==')'){
            while(op.back()!='('){
                num.push_back(op.back());
                op.pop_back();
            }
            op.pop_back();
        }
        else if(exp[i]=='*'){
            op.push_back(exp[i]);
        }
        else if(exp[i]=='&'){
            while(!op.empty()){
                if(op.back()=='|' || op.back()=='(')
                    break;
                num.push_back(op.back());
                op.pop_back();
            }
            op.push_back(exp[i]) ;
        }
        else if(exp[i]=='|'){
            while(!op.empty()){
                if(op.back()=='(')
                    break;
                num.push_back(op.back());
                op.pop_back();
            }
            op.push_back(exp[i]) ;
        }
        else{
            num.push_back(exp[i]);
        }
    }
    while(!op.empty())
    {
        num.push_back(op.back());
        op.pop_back();
    }
    return num;
}

//用于查找某个元素是都在vector里面
template<typename  T> bool MainWindow::find(QVector<T> vt, T value)
{
    for(int i=0;i<vt.size();i++)
        {
            if(vt[i] == value)
                return true;
        }
        return false;
}

void MainWindow::changeToNFA(QVector<QChar> exp)     //将正则表达式转成NFA
{
    QStack<int> s;						   //存放表达式操作数的初始状态和结束状态
    start = endless = 0;
    for(int i=0;i<exp.size();i++){
        if(exp[i] == '|'){
            int start1, start2, end1, end2;
            end2 = s.top();
            s.pop();
            start2 = s.top();
            s.pop();
            end1 = s.top();
            s.pop();
            start1 = s.top();
            s.pop();
            start = ++count1;
            endless = ++count1;
            e[start][start1] = '@';    //该字符用来表示空
            e[start][start2] = '@';
            e[end1][endless] = '@';
            e[end2][endless] = '@';
            s.push(start);
            s.push(endless);

        }
        else if(exp[i] == '&'){
            int start1, start2, end1, end2;
            end2 = s.top();
            s.pop();
            start2 = s.top();
            s.pop();
            end1 = s.top();
            s.pop();
            start1 = s.top();
            s.pop();
            start = start1;
            endless= end2;
            e[end1][start2] = '@';
            s.push(start);
            s.push(endless);
        }
        else if(exp[i] == '*'){
            int start1,end1;
            end1 = s.top();
            s.pop();
            start1 = s.top();
            s.pop();
            start = ++count1;
            endless = ++count1;
            e[start][start1] = '@';
            e[end1][endless] = '@';
            e[start][endless]= '@';
            e[end1][start1] = '@';
            s.push(start);
            s.push(endless);
        }
        else
        {
            if(!find<QChar>(num,exp[i]) && exp[i] != '@')    //将还没出现的操作数放入num
                num.push_back(exp[i]);
            int tempStart,tempEnd;       //存放操作数的起始状态和结束状态压入栈
            tempStart = ++count1;
            tempEnd = ++count1;
            e[tempStart][tempEnd] = exp[i];
            start = tempStart;
            endless = tempEnd;
            s.push(tempStart);
            s.push(tempEnd);
        }
    }

    //将初始状态的数字显示在编辑框里
    ui->lE_startNFA->setText(QString::number(start));

    //将终态的数字显示在编辑框里
    ui->lE_endNFA->setText(QString::number(endless));

    //在table上显示NFA的二维矩阵
    ui->tableNFA->setRowCount(count1);
    ui->tableNFA->setColumnCount(count1);
    QStringList HStrList;
    QStringList VStrList;
    for(int i=1;i<=count1;i++)
    {
        HStrList.push_back(QString::number(i));
        VStrList.push_back(QString::number(i));
    }
    ui->tableNFA->setHorizontalHeaderLabels(HStrList);  //设置行标签和列标签
    ui->tableNFA->setVerticalHeaderLabels(VStrList);
    //设置行和列的大小设为与内容相匹配
    ui->tableNFA->resizeColumnsToContents();
    ui->tableNFA->resizeRowsToContents();

    //将二维数组数据填写进去
    for(int i = 1; i <= count1; i++)
    {
        for(int j = 1; j<= count1; j++)
        {
            ui->tableNFA->setItem((i-1),(j-1),new QTableWidgetItem(QString(e[i][j])));
            ui->tableNFA->item((i-1),(j-1))->setTextAlignment(Qt::AlignHCenter);
        }
    }
}

//求图的第一个邻接顶点
int MainWindow::getFirstNeighbor(int vertex)
{
    for(int i=1;i<=count1;i++)
    {
        if(e[vertex][i]!='\0')
            return i;
    }
    return -1;					//没有邻接顶点返回-1
}

//求图的其他邻接顶点
int MainWindow::getNextNeighbor(int vertex,int w)
{
    for(int i=w+1; i <= count1;i++)
    {
        if(e[vertex][i]!='\0')
            return i;
    }
    return -1;					//没有在w之后的邻接顶点了
}

//判断经过空操作数（‘@’）到达的顶点有哪些
void MainWindow::getThroughNumV(int vertex, char ch, bool visit[])
{
    visit[vertex] = true;
    temp.push_back(vertex);
    int w = getFirstNeighbor(vertex);
    while(w!=-1)
    {
        if(e[vertex][w] == ch && !visit[w])
        {
            getThroughNumV(w, ch, visit);
        }
        w = getNextNeighbor(vertex, w);
    }
}

//判断某个顶点有没有经过某个操作数到达另一个状态数，将其经过某个操作数到达的状态集合保存起来
QVector<int> MainWindow::getSetThroughNum(int vertex, QChar ch)
{
    QVector<int> vt;
    for(int i=1;i<=count1;i++)
    {
        if(e[vertex][i]==ch){
            vt.push_back(i);
        }

    }
    return vt;
}

//两个vector求并集
QVector<int> MainWindow::vectors_set_union(QVector<int> vt1,QVector<int> vt2){
    QVector<int> v;
    sort(vt1.begin(),vt1.end());
    sort(vt2.begin(),vt2.end());
    set_union(vt1.begin(),vt1.end(),vt2.begin(),vt2.end(),back_inserter(v));	//求交集
    return v;
}

//NFA转DFA
void MainWindow::changeToDFA()
{
    bool *visit = new bool[count1+1];
    for(int j=1;j<=count1;j++){         //求各个状态经过‘@’达到的集合,并放入tempV二维QVector容器中
        for(int i=1;i<=count1;i++)
            visit[i] = false;
        getThroughNumV(j, '@', visit);
        sort(temp.begin(),temp.end());
        tempV.push_back(temp);
        temp.clear();
    }
    dfa.push_back(tempV[start-1]);          //将DFA初始状态加入dfa容器中
    endS.push_back(find<int>(tempV[start-1], endless)); //判断该终态是不是初始状态
    count2++;
    int k =0;			//用于记录当前处理的状态集合
    while(true)         //直到没有新的状态产生即退出循环
    {
        for(int i=0;i<num.size();i++)
        {
            QVector<int> temp2;
            for(int j=0;j<dfa[k].size();j++)
            {
                temp = getSetThroughNum(dfa[k][j],num[i]);
                if (!temp.empty()){
                    temp2 = vectors_set_union(temp,temp2);
                    temp.clear();
                }
            }
            if(!temp2.empty())
            {
                bool flag1 = false;     //判断经过某个操作数达到的集合是否已经存在
                QVector<int> temp3;
                for(int j = 0;j<temp2.size();j++){
                    temp = tempV[temp2[j]-1];
                    temp3 = vectors_set_union(temp,temp3);
                    temp.clear();
                }
                sort(temp3.begin(),temp3.end());
                for(int j = 0;j<dfa.size();j++)
                {
                    if(dfa[j] == temp3){                //若状态集合已经存在即自己经过num[i]到达自己
                        e2[k+1][j+1] = num[i];
                        flag1 = true;
                        break;
                    }
                }
                if(!flag1){             //否则产生新的状态
                    count2++;
                    e2[k+1][count2] = num[i];
                    dfa.push_back(temp3);
                    endS.push_back(find<int>(temp3, endless));
                }
            }
        }
        k++;
        if(k==count2)
            break;
    }
}

//求经过某个操作数后到达的状态是哪个(DFA边数组）
int MainWindow::getStatus(int vertex, QChar ch)
{
    for(int i=1;i<=count2;i++)
    {
        if(e2[vertex][i] == ch)
        {
            return i;
        }
    }
    return -1;					//-1代表为空
}

//判断某个状态属于哪个划分集合
int MainWindow::judgeVec(int vec)
{
    for(int i=0;i<statusVec.size();i++)
    {
        if(find<int>(statusVec[i], vec))
            return i;
    }
    return -1;
}

//合并状态将到达s2状态的改为s1的函数
void MainWindow::mergeStatus(int s1, int s2)
{
    for(int i=1;i<=count3;i++)
    {
        if(i==s2)
        {
            for(int j=0;j<num.size();j++)
                DFASmall[i][j] = 0;				//0表示该状态是被合并了
            continue;
        }
        for(int j=0;j<num.size();j++)
        {
            if(DFASmall[i][j]==s2)
            {
                DFASmall[i][j] = s1;
            }
        }
    }
}

//合并状态，将s状态删除掉
void MainWindow::deleteStatus(int s)
{
    for(int i=s;i<=count3;i++)
    {
        for(int j=0;j<num.size();j++)
        {
            DFASmall[i][j] = DFASmall[i+1][j];
        }
    }
    for(int i=1;i<=count3;i++)
    {
        for(int j=0;j<num.size();j++)
        {
            if(DFASmall[i][j]>s)
                DFASmall[i][j] -= 1;         //将某一行删除时还得将指向后面行的状态数减1，s状态后面的整体向前移动了一行
        }
    }
    count3--;
}

//从DFA转成最小化的DFA
void MainWindow::changeToSmallDFA()
{
    int length = num.size();//存放操作数的个数
    count3 = count2;		//未简化之前最小化DFA的状态数为count2
    //初始划分，划分为终态和非终态两个集合
    QVector<int> fs;			//存放终态的顶点
    QVector<int> nfs;		//存放非终态的顶点
    for(int i=0;i<endS.size();i++)
    {
        if(endS[i]==0)
            nfs.push_back(i+1);
        else
            fs.push_back(i+1);
    }
    for(int i=1;i<=count3;i++)              //构建未简化的DFA的状态图
    {
        for(int j=0;j<length;j++)
        {
            DFASmall[i][j] = getStatus(i,num[j]);
        }
    }

    //将初始状态的集合显示在编辑框里
    QString textStart = "{" + QString::number(dfa[0][0]);
    for(int i = 1; i < dfa[0].size(); i++)
    {
        textStart = textStart + "," + QString::number(dfa[0][i]);
    }
    textStart = textStart + "}";
    ui->lE_startDFA->setText(textStart);

    //将终止状态的集合显示在编辑框里
    QString textEnd = "";
    for(int i=0; i<endS.size(); i++)
    {
        if(endS[i])
        {
            textEnd = textEnd + "{" + QString::number(dfa[i][0]);
            for(int k = 1; k < dfa[i].size(); k++)
            {
                textEnd = textEnd + "," + QString::number(dfa[i][k]);
            }
            textEnd = textEnd + "}\n";
        }
    }
    ui->lE_endDFA->setText(textEnd);

    //将未合并的DFA的状态图填写到tableDFA
    ui->tableDFA->setRowCount(count2);
    ui->tableDFA->setColumnCount(length);
    QStringList HStrList;
    QStringList VStrList;
    for(int i=1;i<=count2;i++)
    {
        QString tempstr = "{" + QString::number(dfa[i-1][0]);
        for(int j = 1; j < dfa[i-1].size(); j++)
        {
            tempstr = tempstr + "," + QString::number(dfa[i-1][j]);
        }
        tempstr = tempstr + "}";
        VStrList.push_back(tempstr);
    }
    for(int i =0; i < length; i++)
    {
        HStrList.push_back(QString(num[i]));
    }
    ui->tableDFA->setHorizontalHeaderLabels(HStrList);  //设置行标签和列标签
    ui->tableDFA->setVerticalHeaderLabels(VStrList);

    //将二维数组数据填写进去
    for(int i = 1; i <= count2; i++)
    {
        ui->tableDFA->setRowHeight(i, 40);
        for(int j = 0; j< length; j++)
        {
            ui->tableDFA->setColumnWidth(j, 200);
            int s = DFASmall[i][j] - 1;
            if(s == -2)
                continue;
            QString text = "{" + QString::number(dfa[s][0]);
            for(int k = 1; k < dfa[s].size(); k++)
            {
                text = text + "," + QString::number(dfa[s][k]);
            }
            text = text + "}";
            ui->tableDFA->setItem((i-1),(j),new QTableWidgetItem(text));
            ui->tableDFA->item((i-1),(j))->setTextAlignment(Qt::AlignHCenter);
        }
    }

    if(!nfs.empty())
            statusVec.push_back(nfs);
    statusVec.push_back(fs);
    QVector<QVector<int>> tempVec;                  //用于临时存放每次划分集合的结果
    bool *dealFlag = new bool[count2+1];			//记录该顶点是否已被合并了
    while(true)
    {
        int num1 = statusVec.size();					//临时存放划分的个数
        for(int i=0;i<=count2;i++)
        {
            dealFlag[i] = false;        //每次判断前先赋值为全部未合并
        }
        for(int i=0;i<num1;i++)         //对现存状态进行划分直到不能再划分则退出循环
        {
            if(statusVec[i].size()==1)
            {
                tempVec.push_back(statusVec[i]);
                continue;
            }
            for(int j=0;j<statusVec[i].size()-1;j++)
            {
                if(dealFlag[statusVec[i][j]])
                    continue;				//如果已被合并过则直接下一个循环
                QVector<int> temp1;         //存放属于同一划分的集合
                temp1.push_back(statusVec[i][j]);
                for(int k=j+1;k<statusVec[i].size();k++)
                {
                    if(dealFlag[statusVec[i][k]])
                        continue;				//如果已被合并过则直接下一个循环
                    int t;
                    for(t = 0;t<num.size();t++)     //判断是否相同，相同则划分到同一个集合
                    {
                        if(judgeVec(DFASmall[statusVec[i][j]][t]) != judgeVec(DFASmall[statusVec[i][k]][t]))
                            break;
                    }
                    if(t==num.size())
                    {
                        dealFlag[statusVec[i][k]] = true;
                        temp1.push_back(statusVec[i][k]);
                    }
                }
                tempVec.push_back(temp1);
            }
        }
        statusVec.clear();
        while(!tempVec.empty())
        {
            statusVec.push_back(tempVec.back());
            tempVec.pop_back();
        }
        if(num1 == statusVec.size())
            break;
    }

    delete []dealFlag;					//删除开辟的空间

    for(int i = 0; i < statusVec.size(); i++) //合并相同的状态
    {
        if(statusVec[i].size() == 1)
            continue;
        for(int j=1;j<statusVec[i].size();j++)
        {
            mergeStatus(statusVec[i][0], statusVec[i][j]);
        }
    }

    //将最小化的DFA初始状态的集合显示在编辑框里
    ui->lE_startDFASmall->setText(QString::number(1));

    //将最小化的DFA终止状态的集合显示在编辑框里
    QString textEnd1 = "";
    int k=0;
    for(int i=0; i<endS.size(); i++)
    {
        if(DFASmall[i+1][0] != 0)
        {
            k++;
            if(endS[i])
                textEnd1 = textEnd1 + QString::number(k) + "  ";
        }

    }
    ui->lE_endDFASmall->setText(textEnd1);


    //将标记为0即被合并的行数移动掉
    for(int i=1;i<=count3;i++)
    {
        for(int j=0;j<num.size();j++)
        {
            if(DFASmall[i][j]==0)
            {
                deleteStatus(i);
                i=i-1;
                break;
            }
        }
    }

    //将最小化的DFA显示在tableDFASmall表格上
    ui->tableDFASmall->setRowCount(count3);
    ui->tableDFASmall->setColumnCount(length);
    QStringList HStrList1;
    QStringList VStrList1;
    for(int i = 1;i <= count3; i++)
    {
        VStrList1.push_back(QString::number(i));
    }
    for(int i = 0; i < length; i++)
    {
        HStrList1.push_back(QString(num[i]));
    }
    ui->tableDFASmall->setHorizontalHeaderLabels(HStrList1);  //设置行标签和列标签
    ui->tableDFASmall->setVerticalHeaderLabels(VStrList1);

    //将二维数组数据填写进去
    for(int i = 1; i <= count3; i++)
    {
        ui->tableDFASmall->setRowHeight(i, 40);
        for(int j = 0; j< length; j++)
        {
            ui->tableDFASmall->setColumnWidth(j, 200);
            if (DFASmall[i][j]!=-1)
            {
                ui->tableDFASmall->setItem((i-1),(j),new QTableWidgetItem(QString::number(DFASmall[i][j])));
                ui->tableDFASmall->item((i-1),(j))->setTextAlignment(Qt::AlignHCenter);
            }
        }
    }
}

//将最小化的DFA转成词法分析程序
void MainWindow::changeToCode()
{
    QString code = "";              //存放词法分析程序
    code += code + "int status = 1;\nwhile(";
    for(int i=1;i<=count3;i++)      //每个状态进行选择，不在这些状态内则退出循环
    {
        if(i == count3)
            code = code + "status == " + QString::number(i) + ")\n{";
        else
            code = code + "status == " + QString::number(i) + " || ";
    }
    code = code + " switch(status){\n";
    for(int i=1;i<=count3;i++)      //进行每个状态
    {
        int k=0;        //判断该状态是否没有经过任何操作数
        code = code + "     case " + QString::number(i) + " :\n";
        for(int j = 0; j<num.size(); j++)     //判断经过某个状态后到达哪个转台
        {
            if(DFASmall[i][j] != -1){
                code = code + "         if input() == " + "'" + num[j] + "'" + ":\n";
                code = code + "             status = " + QString::number(DFASmall[i][j]) + ";\n";
            }
            else
                k++;
        }
        if(k!=num.size())
            code = code + "         else: status = -1;  //进入错误状态或其他状态跳出循环\n";
        else
            code = code + "         status = -1;  //进入错误状态或其他状态跳出循环\n";
        code = code + "         break;\n";
    }
    code = code + "     default: break;\n   }\n}";
    ui->textEdit->setText(code);
}

MainWindow::~MainWindow()
{
    delete ui;
}
