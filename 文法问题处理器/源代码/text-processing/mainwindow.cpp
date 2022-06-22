#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<QFile>
#include<QTextStream>
#include<QString>
#include<QMessageBox>
#include<algorithm>
#include<iostream>
#include<QDebug>
#include<QStack>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(0);

    //退出系统
    connect(ui->pB_exit,&QPushButton::clicked,this,[=](){
        exit(0);
    });

    //保存文法规则
    connect(ui->pB_save,&QPushButton::clicked,this,[=](){
        QString text1 = ui->textEdit->toPlainText();
        if(text1 == "")
        {
            QMessageBox::warning(this, tr("提示") ,tr("文法规则为空，无法保存！"));
        }
        else{
            QFileDialog fileDialog;
            QString file_name = fileDialog.getSaveFileName(this,tr("Open File"),"/text",tr("Text File(*.txt)"));
            QFile file(file_name);
            if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
                return;
            QTextStream out(&file);
            QString text1 = ui->textEdit->toPlainText();
            out<<text1;
        }
    });

    //打开存有文法规则的文件
    connect(ui->pB_open,&QPushButton::clicked,this,[=](){
        QString file_name = QFileDialog::getOpenFileName(this,"获取正则表达式文本文件",".","*.txt");
        QFile file(file_name);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QString text1 = "";
        while(!in.atEnd())
        {
            text1 = text1 + in.readLine() + '\n';
        }
        ui->textEdit->setText(text1);
    });

    //进行文法规则处理
    connect(ui->pB_process,&QPushButton::clicked,this,[=](){
        ui->tabWidget->setCurrentIndex(0);
        QString text1 = ui->textEdit->toPlainText();        //获取编辑框的文法规则
        QStringList text_list = text1.split("\n");          //一条文法规则存储为一个QString
        int j = 0;
        for(int i=0;i<text_list.size();i++)
        {
            if(j==i && text_list[i]=="")
            {
                j=i+1;
            }
        }
        for(int i =0;i<j;i++)
            text_list.removeFirst();
        if(text1 == "")
        {
            QMessageBox::warning(this,tr("提示"),tr("文法规则为空，请先输入文法规则再进行处理！！！"));
            return;
        }

        this->init();  //处理文法前先把前一次内容清空初始化

        textNode *p;
        //存储文法规则
        for(int i=0;i<text_list.length();i++)
        {
            if(text_list[i] == "")
                continue;
            QStringList textRule = text_list[i].split("->");     //将文法规则以->分割开，一边是文法规则的左部，一边是文法规则的右部
            for(int j=0;j<textRule.length();j++)
            {
                textRule[j].remove(QRegExp("\\s"));  //去掉文法规则左右两边里面的空格
            }
            if(textRule[0] == textRule[1])      //删除有害规则U->U
            {
                continue;
            }
            if(i==0)
            {
                t1.start = textRule[0][0];   //文法开始符号为第一条规则的左部符号
                t1.textG = new textNode(textRule[0][0]);
                p = t1.textG;
            }
            else{
                textNode *q = new textNode(textRule[0][0]);
                p ->next = q;
                p = q;
            }
            if(t1.non_terminalSet.indexOf(textRule[0][0])==-1)
            {
                t1.non_terminalSet.append(textRule[0][0]);
            }
            if(textRule[1][0] != '|')
            {
                if(t1.non_terminalSet.indexOf(textRule[1][0])==-1 && textRule[1][0] >='A' && textRule[1][0] <='Z')
                    t1.non_terminalSet.append(textRule[1][0]);
                else if(t1.terminalSet.indexOf(textRule[1][0])==-1 && ((textRule[1][0] >='a' && textRule[1][0] <='z') || textRule[1][0] =='@') )
                        t1.terminalSet.append(textRule[1][0]);
            }
            p->right = new rightNode(textRule[1][0]);
            rightNode *p1 = p->right;
            for(int j=1;j<textRule[1].length();j++)         //存储右部规则
            {
                if(textRule[1][j] != '|')
                {
                    if(t1.non_terminalSet.indexOf(textRule[1][j])==-1 && textRule[1][j] >='A' && textRule[1][j] <='Z')
                        t1.non_terminalSet.append(textRule[1][j]);
                    else if(t1.terminalSet.indexOf(textRule[1][j])==-1 && ((textRule[1][j] >='a' && textRule[1][j] <='z') || textRule[1][j] =='@') )
                            t1.terminalSet.append(textRule[1][j]);
                }
                if(textRule[1][j] == '|')
                {
                    p1->next=NULL;
                    textNode *q = new textNode(textRule[0][0]);
                    p ->next = q;
                    p = q;
                    p->right = new rightNode(textRule[1][j+1]);
                    p1 = p->right;
                    j = j+1;
                    if(t1.non_terminalSet.indexOf(textRule[1][j])==-1 && textRule[1][j] >='A' && textRule[1][j] <='Z')
                        t1.non_terminalSet.append(textRule[1][j]);
                    else if(t1.terminalSet.indexOf(textRule[1][j])==-1 && ((textRule[1][j] >='a' && textRule[1][j] <='z') || textRule[1][j] =='@') )
                            t1.terminalSet.append(textRule[1][j]);
                    continue;
                }
                rightNode *q1 = new rightNode(textRule[1][j]);
                p1->next = q1;
                p1 = q1;
            }
            p1->next = NULL;  //右部规则的最后一个的指向为NULL
        }
        p->next=NULL;  //最后一条文法规则的指向为NULL

        this->simplify();  //化简文法结构，删掉多余规则
        //输出化简后的存储结构中的化简后文法规则
        p = t1.textG;
        QString str = "";
        while(p!=NULL)
        {
            rightNode *p1 = p->right;
            str = str + p->leftCh + "->";
            while(p1!= NULL)
            {
                str = str + p1->rightCh;
                p1= p1->next;
            }
            str = str + "\n";
            p = p->next;
        }
        ui->textEdit_2->setText(str);

        str = "";
        str = str  + t1.start;
        ui->lineEdit_start1->setText(str);

        str = "";
        for(int i=0;i<t1.non_terminalSet.size();i++)
        {
            str = str +t1.non_terminalSet[i];
        }
        ui->lineEdit_non1->setText(str);

        str = "";
        for(int i=0;i<t1.terminalSet.size();i++)
        {
            str = str +t1.terminalSet[i];
        }
        ui->lineEdit_te1->setText(str);

        //将化简后的非终结符号
        for(int i=0;i<t1.non_terminalSet.size();i++)
        {
            upperCase.removeAll(t1.non_terminalSet[i]);
        }


        this->deleteLeftRecursion();        //消除左递归
        this->leftFactor();                 //提取左公因子

        this->simplify();  //求完左公因子和左递归后再次化简

        //输出存储结构中的化简后文法规则
        p =t1.textG;
        str = "";
        while(p!=NULL)
        {
            rightNode *p1 = p->right;
            str = str + p->leftCh + "->";
            while(p1!= NULL)
            {
                str = str + p1->rightCh;
                p1= p1->next;
            }
            str = str + "\n";
            p = p->next;
        }
        ui->textEdit_5->setText(str);
        ui->textEdit_14->setText(str);

        str = "";
        str = str  + t1.start;
        ui->lineEdit_start4->setText(str);
        ui->lineEdit_start5->setText(str);

        str = "";
        for(int i=0;i<t1.non_terminalSet.size();i++)
        {
            str = str +t1.non_terminalSet[i];
        }
        ui->lineEdit_non4->setText(str);
        ui->lineEdit_non5->setText(str);

        str = "";
        for(int i=0;i<t1.terminalSet.size();i++)
        {
            str = str +t1.terminalSet[i];
        }
        ui->lineEdit_te4->setText(str);
        ui->lineEdit_te5->setText(str);
    });

    //求first和follow集合
    connect(ui->pB_ff,&QPushButton::clicked,this,[=](){
        firstMap.clear();
        followMap.clear();
        //求first集合并把非终结符号显示的first集合显示在表格上
        this->first(t1.start);
        for(int i=0;i<t1.non_terminalSet.size();i++)
        {
            if(!firstMap.contains(t1.non_terminalSet[i]))
                this->first(t1.non_terminalSet[i]);
        }
        for(int i=0;i<t1.terminalSet.size();i++)
            this->first(t1.terminalSet[i]);

        //求follow集合，先将开始符号对应的follow集合填上$;
        QVector<QChar> temp;
        temp.append('$');
        followMap[t1.start] = temp;
        this->follow();

        ui->tableWidget->setRowCount(t1.non_terminalSet.size());
        ui->tableWidget->setColumnCount(2);
        QStringList HStrList;
        QStringList VStrList;
        for(int i = 0;i<t1.non_terminalSet.size();i++)
        {
            HStrList.push_back(t1.non_terminalSet[i]);
        }
        VStrList.push_back("first集合");
        VStrList.push_back("follow集合");
        ui->tableWidget->setHorizontalHeaderLabels(VStrList);  //设置行标签和列标签
        ui->tableWidget->setVerticalHeaderLabels(HStrList);
        for(int j=0;j<t1.non_terminalSet.size();j++)
        {
            QString str = "";
            for(int k=0;k<firstMap[t1.non_terminalSet[j]].size();k++)
            {
                str = str + firstMap[t1.non_terminalSet[j]][k];
            }
            ui->tableWidget->setItem(j,0,new QTableWidgetItem(str));
        }
        for(int j=0;j<t1.non_terminalSet.size();j++)
        {
            QString str = "";
            for(int k=0;k<followMap[t1.non_terminalSet[j]].size();k++)
            {
                str = str + followMap[t1.non_terminalSet[j]][k];
            }
            ui->tableWidget->setItem(j,1,new QTableWidgetItem(str));
        }

    });

    //分析句子
    connect(ui->pB_ala,&QPushButton::clicked,this,[=](){
        this->left_infer();

    });
}

void MainWindow::init()         //全局变量的初始化
{
    t1.terminalSet.clear();
    t1.non_terminalSet.clear();
    t1.start = '\0';
    textNode *p;
    p = t1.textG;
    while(p!=NULL)
    {
        rightNode *p1=p->right;
        while(p1!=NULL)
        {
            rightNode *q1 = p1;
            p1 = p1 ->next;
            delete q1;
        }
        textNode *q = p;
        p = p->next;
        delete q;
    }
    t1.textG = NULL;
    upperCase.clear();
    for(int i= 65;i<=90;i++)
        upperCase.append(i);
    tempMap.clear();
    firstMap.clear();
    followMap.clear();
    judgeLeft.clear();
}

//两个vector求并集
QVector<QChar> MainWindow::vectors_set_union(QVector<QChar> vt1,QVector<QChar> vt2){
    QVector<QChar> v;
    sort(vt1.begin(),vt1.end());
    sort(vt2.begin(),vt2.end());
    set_union(vt1.begin(),vt1.end(),vt2.begin(),vt2.end(),back_inserter(v));	//求交集
    return v;
}


void MainWindow::simplify()   //删除多余规则
{
    QVector<QChar> temp1,temp2,temp3;       //存放可以推到出到终结符号的非终结符号
    temp1 = t1.terminalSet;
    while(true)         //消除不可终止
    {
        textNode *p = t1.textG;
        temp3 = temp2;
        while(p!= NULL)       //判断文法规则是否结束
        {
            rightNode *p1 = p->right;
            while(p1!=NULL)
            {
                if(temp1.indexOf(p1->rightCh) == -1 && temp2.indexOf(p1->rightCh) == -1)
                    break;
                p1 = p1->next;
            }
            if(p1 == NULL)
            {
                if(temp2.indexOf(p->leftCh) == -1)
                    temp2.append(p->leftCh);
            }
            p = p->next;
        }
        if(temp2 == temp3)
            break;
    }
    textNode *p = t1.textG;
    textNode *q = t1.textG;          //文法的链表结构的链接
    while(p!= NULL)       //删掉不可终止的文法规则
    {
        if(temp2.indexOf(p->leftCh) == -1)
        {
            if(q==p)
            {
                q = p->next;
                while(p->right!=NULL)
                {
                    rightNode *p1 = p->right;
                    p->right = p->right->next;
                    delete p1;
                }
                delete p;
                p = q;
                t1.textG = p;
                continue;
            }
            q->next = p->next;
            while(p->right!=NULL)
            {
                rightNode *p1 = p->right;
                p->right = p->right->next;
                delete p1;
            }
            delete p;
            p = q->next;
        }
        else
        {
            rightNode *p1 = p->right;
            while(p1!=NULL)
            {
                if(p1->rightCh >= 65 && p1->rightCh <= 90 && temp2.indexOf(p1->rightCh) == -1)
                {

                    if(q==p)
                    {
                        q = p->next;
                        while(p->right!=NULL)
                        {
                            rightNode *p1 = p->right;
                            p->right = p->right->next;
                            delete p1;
                        }
                        delete p;
                        p = q;
                        t1.textG = p;
                        break;
                    }
                    q->next = p->next;
                    while(p->right!=NULL)
                    {
                        rightNode *p1 = p->right;
                        p->right = p->right->next;
                        delete p1;
                    }
                    delete p;
                    p = q->next;
                    break;
                }
                p1 = p1->next;
            }
            if(p1 == NULL)
            {
                q = p;
                p = p->next;
            }
        }
    }

    t1.non_terminalSet.clear();
    t1.non_terminalSet = temp2;   //将不可终止的消除后的非终结符号的集合

    temp1.clear();
    temp2.clear();
    temp1.append(t1.start);
    while(true)     //消除不可到达
    {
        textNode *p = t1.textG;
        temp2 = temp1;
        while(p!=NULL)
        {
            if(temp1.indexOf(p->leftCh) != -1)
            {
                rightNode *p1 = p->right;
                while(p1!=NULL)
                {
                    if(temp1.indexOf(p1->rightCh) == -1)
                    {
                        temp1.append(p1->rightCh);
                    }
                    p1 = p1->next;
                }
            }
            p = p->next;
        }
        if(temp1 == temp2)
            break;
    }

    //先将文法规则的终结符号集合和非终极符号集合清空，以边存入化简后的新集合
    t1.non_terminalSet.clear();
    t1.terminalSet.clear();
    for(int i=0;i<temp1.size();i++)
    {
        if(temp1[i]>=65 && temp1[i]<=90)
            t1.non_terminalSet.append(temp1[i]);
        else
            t1.terminalSet.append(temp1[i]);
    }

    p = t1.textG;
    q = t1.textG;   //文法的链表结构的链接
    while(p!= NULL)       //删掉不可到达的文法规则
    {
        if(temp2.indexOf(p->leftCh) == -1)
        {
            if(q==p)
            {
                q = p->next;
                while(p->right!=NULL)
                {
                    rightNode *p1 = p->right;
                    p->right = p->right->next;
                    delete p1;
                }
                delete p;
                p = q;
                t1.textG = p;
                continue;
            }
            q->next = p->next;
            while(p->right!=NULL)
            {
                rightNode *p1 = p->right;
                p->right = p->right->next;
                delete p1;
            }
            delete p;
            p = q->next;
        }
        else
        {
            rightNode *p1 = p->right;
            while(p1!=NULL)
            {
                if(p1->rightCh >= 65 && p1->rightCh <= 90 && temp2.indexOf(p1->rightCh) == -1)
                {
                    if(q==p)
                    {
                        q = p->next;
                        while(p->right!=NULL)
                        {
                            rightNode *p1 = p->right;
                            p->right = p->right->next;
                            delete p1;
                        }
                        delete p;
                        p = q;
                        t1.textG = p;
                        break;
                    }
                    q->next = p->next;
                    while(p->right!=NULL)
                    {
                        rightNode *p1 = p->right;
                        p->right = p->right->next;
                        delete p1;
                    }
                    delete p;
                    p = q->next;
                    break;
                }
                p1 = p1->next;
            }
            if(p1 == NULL)
            {
                q = p;
                p = q->next;
            }
        }
    }

    if(t1.textG==NULL)
    {
        t1.non_terminalSet.clear();
        t1.terminalSet.clear();
        t1.start = '\0';
    }
}

QVector<QChar> MainWindow::findTextRule(QChar leftCh,QChar rightFirst, bool f,QChar lf)  //求左部规则为leftCh，右部规则的第一个字符为rightFirst的右部文法规则
{
    if(judgeLeft[leftCh] != 1)          //若该非终结符号还没提取左公因子就先提取
        this->leftFactorChar(leftCh);
    textNode *p = t1.textG;
    QVector<QChar> temp;
    while(p!=NULL)
    {
        if(p->leftCh==leftCh)
        {
            rightNode *p1 = p->right;
            if(p1->rightCh != rightFirst && p1->rightCh !=lf)
            {
                p = p->next;
                continue;
            }
            while(p1!=NULL)
            {
                temp.append(p1->rightCh);
                p1 = p1->next;
            }
            break;
        }
        p = p->next;
    }
    if(p!=NULL)
    {
        return temp;
    }
    else
    {
        p = t1.textG;
        temp.clear();
        while(p!=NULL)
        {
            if(p->leftCh==leftCh)
            {
                rightNode *p1 = p->right;
                if(t1.non_terminalSet.indexOf(p1->rightCh) != -1)
                {
                    if(firstMap[p1->rightCh].indexOf(rightFirst) != -1)
                    {
                        if(f)
                        {
                            temp = findTextRule(p1->rightCh,rightFirst,true,lf);
                            p1 = p1->next;
                            while(p1!=NULL)
                            {
                                temp.append(p1->rightCh);
                                p1 = p1->next;
                            }
                        }
                        else
                        {
                            while(p1!=NULL)
                            {
                                temp.append(p1->rightCh);
                                p1 = p1->next;
                            }
                        }
                        break;
                    }
                }
            }
            p = p->next;
        }
        return temp;
    }
}

void MainWindow::leftFactorChar(QChar ch)    //提取某个具体的非终结符号的左公因子
{
    QMap<QChar,QVector<int>> tempStatus;   //记录哪几条文法规则有相同的左公因子，以在文法规则中出现的序号(以序号0为开始计数）为记录依据
    QVector<QVector<QChar>> saveRightFirst;  //存放右部规则的第一个字母的first集合
    QVector<QChar> tempLeft;        //记录所有文法规则可能出现的最左公因子
    textNode *p = t1.textG;
    int i=0;
    while(p!=NULL)
    {
        if(p->leftCh == ch)
        {
            if(firstMap[p->right->rightCh].empty())
                qDebug()<<"为空！";
            saveRightFirst.append(firstMap[p->right->rightCh]);
            i++;
        }
        p = p->next;
    }
    bool *save = new bool[i];
    for(int j =0;j<i;j++)
        save[j] = true;         //true代表第几条左部规则为ch的文法规则需要删掉
    for(int j=0;j<saveRightFirst.size();j++)
        tempLeft = this->vectors_set_union(tempLeft,saveRightFirst[j]);

    for(int j=0;j<tempLeft.size();j++)
    {
        for(int k=0;k<saveRightFirst.size();k++)
        {
            if(saveRightFirst[k].indexOf(tempLeft[j]) != -1)
                tempStatus[tempLeft[j]].append(k);
        }
    }

    for(int j = 0;j<tempLeft.size();j++)
    {
        if(tempStatus[tempLeft[j]].size()>1)
        {
            QVector<QVector<QChar>> findLeftT;          //存放具有相同左公因子的文法规则的右部
            QVector<QChar> left;        //存放左公因子
            left.append(tempLeft[j]);
            int n = 0;  //记录文法规则左部为ch在文法中的位置
            p = t1.textG;
            textNode *q;
            while (p!=NULL) {
                if(p->leftCh==ch)
                {
                    if(tempStatus[tempLeft[j]].indexOf(n) !=-1)
                    {
                        QVector<QChar> temp;
                        rightNode *p1 = p->right;
                        if(t1.non_terminalSet.indexOf(p1->rightCh) != -1 && p1->rightCh!=ch)
                        {
                            temp = findTextRule(p1->rightCh,tempLeft[j],true,ch);
                        }
                        else
                        {
                            temp.append(p1->rightCh);
                        }
                        p1 = p1->next;
                        while(p1!=NULL)
                        {
                            temp.append(p1->rightCh);
                            p1 = p1->next;
                        }
                        findLeftT.append(temp);
                    }
                    n++;
                }
                if(p->next==NULL)
                    q = p;
                p = p->next;
            }
            for(int k =1;k<findLeftT[0].size();k++)     //用于求右部规则剩下的符号中有没有存在相同的左公因子
            {
                bool flag = true;
                for(int k1=1;k1<findLeftT.size();k1++)
                {
                    if(findLeftT[k1].size()>k)
                    {
                        if(findLeftT[0][k] != findLeftT[k1][k])
                        {
                            flag = false;
                            break;
                        }
                    }
                    else
                    {
                        flag = false;
                        break;
                    }
                }
                if(flag)
                    left.append(findLeftT[0][k]);   //存在则加入左公因子集合
                else
                    break;
            }
            for(int k1 = 0;k1<left.size();k1++)     //删除右部规则的相同的左公因子，便于改造文法规则
            {
                for(int k=0;k<findLeftT.size();k++)
                {
                    findLeftT[k].removeOne(left[k1]);
                }
            }
            //将改造后的文法加入的文法规则的末尾
            textNode *s = new textNode(ch);
            s->right = new rightNode(left[0]);
            rightNode *s1 = s->right;
            for(int k =1; k<left.size();k++)
            {
                rightNode *q1 = new rightNode(left[k]);
                s1->next = q1;
                s1 = q1;
            }
            rightNode *q1 = new rightNode(upperCase[0]);
            s1->next = q1;
            s1 = q1;
            s1->next=NULL;
            q->next = s;
            q = s;
            t1.non_terminalSet.append(upperCase[0]);
            judgeLeft[upperCase[0]] = 0;
            for(int k=0;k<findLeftT.size();k++)
            {
                s = new textNode(upperCase[0]);
                if(findLeftT[k].empty())
                {
                    if(t1.terminalSet.indexOf('@')==-1)
                    {
                        t1.terminalSet.append('@');
                        this->first('@');
                    }
                    s->right = new rightNode('@');
                    s->right->next=NULL;
                    q->next = s;
                    q = s;
                }
                else
                {
                    s->right = new rightNode(findLeftT[k][0]);
                    rightNode *s1 = s->right;
                    for(int k1 =1; k1<findLeftT[k].size();k1++)
                    {
                        rightNode *q1 = new rightNode(findLeftT[k][k1]);
                        s1->next = q1;
                        s1 = q1;
                    }
                    s1->next=NULL;
                    q->next = s;
                    q =s;
                }
            }
            q->next=NULL;
            this->first(upperCase[0]);  //求这个新符号的firstSet
            upperCase.removeFirst();  //在大写字母中删除已用的非终结符号
        }
        else
        {

            bool flag = true;   //用于判断文法是否出现在其他状态集合tempStatus[tempLeft[k]]里
            for(int k =0;k<tempLeft.size();k++)
            {
                if(k!=j)
                {
                    if(tempStatus[tempLeft[k]].indexOf(tempStatus[tempLeft[j]][0]) != -1 && tempStatus[tempLeft[k]].size()>1)
                    {
                        flag = false;
                        break;
                    }
                }
            }
            if(flag)
            {
                save[tempStatus[tempLeft[j]][0]] = false;    //不含公共左公因子的文法不删除
                continue;
            }
            else
            {
                int n =0;
                p = t1.textG;
                QVector<QChar> temp;
                while(p!=NULL)
                {
                    if(p->leftCh == ch)
                    {
                        if(n == tempStatus[tempLeft[j]][0])
                        {
                            rightNode *p1 = p->right;
                            temp = findTextRule(p1->rightCh,tempLeft[j],false,ch);
                            if(temp.empty())
                            {
                                QMessageBox::warning(this,"警告","输出文法非上下文无关文法！");
                                exit(-1);
                            }
                            if(temp[0]=='@')
                                temp.clear();
                            p1 = p1->next;
                            while(p1!=NULL)
                            {
                                temp.append(p1->rightCh);
                                p1 = p1->next;
                            }
                            break;
                        }
                        n++;
                    }
                    p = p->next;
                }
                while(p->next != NULL)
                {
                    p = p->next;
                }

                textNode *s = new textNode(ch);
                if(temp.empty())
                {
                    if(t1.terminalSet.indexOf('@')==-1)
                    {
                        t1.terminalSet.append('@');
                        this->first('@');
                    }
                    s->right = new rightNode('@');
                    s->right->next=NULL;
                    p->next = s;
                    p = s;
                }
                else
                {
                    s->right = new rightNode(temp[0]);
                    rightNode *s1 = s->right;
                    for(int k =1; k<temp.size();k++)
                    {
                        rightNode *q1 = new rightNode(temp[k]);
                        s1->next = q1;
                        s1 = q1;
                    }
                    s1->next =NULL;
                    p->next = s;
                    p = s;
                }
                p->next=NULL;
            }
        }
    }
    p = t1.textG;
    int n = 0;
    textNode *q = p;
    while (p!=NULL) {
        if(p->leftCh==ch)
        {
            if(save[n])
            {
                if(q == p)
                {
                    q = p->next;
                    delete p;
                    p = q;
                    t1.textG = p;
                }
                else
                {
                    q->next = p->next;
                    delete p;
                    p = q->next;
                }
                n++;
                if(n==i)
                    break;
                continue;
            }
            n++;
            if(n==i)
                break;
        }
        q = p;
        p = p->next;
    }
    judgeLeft[ch] = 1;
    delete []save;
}

void MainWindow::leftFactor()   //提取左公因子
{
    //先求一遍非终结符号的first集合，以便查看右部出现第一个字符为非终结符号时是否需要替换
    this->first(t1.start);
    for(int i=0;i<t1.non_terminalSet.size();i++)
    {
        if(!firstMap.contains(t1.non_terminalSet[i]))
            this->first(t1.non_terminalSet[i]);
    }
    for(int i=0;i<t1.terminalSet.size();i++)
        this->first(t1.terminalSet[i]);

    for(int i=0;i<t1.non_terminalSet.size();i++)
        judgeLeft[t1.non_terminalSet[i]] = 0;

    for(int i=0;i<t1.non_terminalSet.size();i++)
    {
        if(judgeLeft[t1.non_terminalSet[i]])
            continue;
        else
            this->leftFactorChar(t1.non_terminalSet[i]);
    }
    //输出存储结构中的化简后文法规则
    textNode *p =t1.textG;
    QString str = "";
    while(p!=NULL)
    {
        rightNode *p1 = p->right;
        str = str + p->leftCh + "->";
        while(p1!= NULL)
        {
            str = str + p1->rightCh;
            p1= p1->next;
        }
        str = str + "\n";
        p = p->next;
    }
    ui->textEdit_3->setText(str);
    qDebug()<<t1.start;

    str = "";
    str = str  + t1.start;
    ui->lineEdit_start2->setText(str);

    str = "";
    for(int i=0;i<t1.non_terminalSet.size();i++)
    {
        str = str +t1.non_terminalSet[i];
    }
    ui->lineEdit_non2->setText(str);

    str = "";
    for(int i=0;i<t1.terminalSet.size();i++)
    {
        str = str +t1.terminalSet[i];
    }
    ui->lineEdit_te2->setText(str);
}

QVector<QVector<QChar>> MainWindow::findRight(QChar ch,QChar left)   //找非终结符号ch的所有右部并返回，left用于查看右部是否包含这个字母，以便决定是否要返回右部规则
{
    QVector<QVector<QChar>> right;
    textNode *p = t1.textG;
    bool flag = false;   //判断是否要返回
    while (p!=NULL) {
        if(p->leftCh == ch){
            QVector<QChar> temp;
            rightNode *p1 = p->right;
            if(p1->rightCh == left)
                flag = true;
            while(p1!=NULL)
            {
                temp.append(p1->rightCh);
                p1=p1->next;
            }
            right.append(temp);
        }
        p= p->next;
    }
    if(flag)
        return right;
    else
    {
        right.clear();
        return right;
    }
}

void MainWindow::deleteLeftRecursion()          //消除左递归
{
    //消除左递归，非终结符号按存放非终态集合的顺序排列t1.non_terminalSet
    int length = t1.non_terminalSet.size();
    for(int i=0;i<length;i++)
    {
        QVector<QVector<QChar>> rightRule; //存放所有右部规则，便于查看哪些右部规则是左递归
        QVector<QVector<QChar>> sameLeft;  //存放是左递归的文法规则右部
        QVector<QChar> cleanCh;     //存放已被消除左递归的非终结符号
        QMap<QChar,int> flag;       //用于代表前面已经消除左递归的非终结符号是否包含间接左递归，若包含则为，否则为0
        for(int j=0;j<=i-1;j++)
        {
            cleanCh.append(t1.non_terminalSet[j]);
            flag[t1.non_terminalSet[j]] = 0;
        }
        int n =0;   //用于记录原先左部为t1.non_terminalSet[i]的前n条规则，便于后面删除
        textNode *p=t1.textG;
        textNode *q;
        while(p!=NULL)
        {
            if(p->leftCh == t1.non_terminalSet[i])
            {
                n++;
                QVector<QChar> temp;
                rightNode *p1 = p->right;
                if(cleanCh.indexOf(p1->rightCh)!=-1)
                {
                    QVector<QVector<QChar>> tempRight = this->findRight(p1->rightCh, t1.non_terminalSet[i]);
                    if(tempRight.empty())       //若返回为空，说明改规则不存在间接左递归，无须将右部带回规则中
                    {
                        temp.clear();
                        while(p1!=NULL)
                        {
                            temp.append(p1->rightCh);
                            p1=p1->next;
                        }
                        rightRule.append(temp);
                        p = p->next;
                        continue;
                    }
                    flag[p1->rightCh] = 1;
                    for(int j =0;j<tempRight.size();j++)
                    {
                        temp.clear();
                        rightNode *q1 = p1->next;
                        for(int k=0;k<tempRight[j].size();k++)
                        {
                            temp.append(tempRight[j][k]);
                        }
                        while(q1!=NULL)
                        {
                            temp.append(q1->rightCh);
                            q1=q1->next;
                        }
                        rightRule.append(temp);
                    }
                }
                else
                {
                    temp.clear();
                    while(p1!=NULL)
                    {
                        temp.append(p1->rightCh);
                        p1=p1->next;
                    }
                    rightRule.append(temp);
                }
            }
            if(p->next==NULL)
                q=p;
            p = p->next;
        }
        for(int j=0;j<rightRule.size();j++)     //把右部分为包含左递归和非左递归
        {
            if(rightRule[j][0]==t1.non_terminalSet[i])
            {
                rightRule[j].removeOne(t1.non_terminalSet[i]);
                sameLeft.append(rightRule[j]);
                rightRule.remove(j);
                j--;
            }
        }
        if(sameLeft.empty())
            continue;
        for(int j=0;j<rightRule.size();j++)         //将右部不包含左递归的构建以t1.non_terminalSet[i]为左部的文法规则加入文法结构中
        {
            if(!rightRule[j].empty())
            {
                textNode *s = new textNode(t1.non_terminalSet[i]);
                s->right = new rightNode(rightRule[j][0]);
                rightNode *p1 = s->right;
                for(int k=1;k<rightRule[j].size();k++)
                {
                    rightNode *q1 = new rightNode(rightRule[j][k]);
                    p1->next = q1;
                    p1 = q1;
                }
                rightNode *q1 = new rightNode(upperCase[0]);
                p1->next = q1;
                p1=q1;
                p1->next=NULL;
                q->next=s;
                q=s;
            }
        }
        q->next=NULL;
        for(int j=0;j<sameLeft.size();j++) //将右部包含左递归的构建以upperCase[0]为左部的文法规则加入文法结构中
        {
            if(!sameLeft[j].empty())
            {
                textNode *s = new textNode(upperCase[0]);
                s->right = new rightNode(sameLeft[j][0]);
                rightNode *p1 = s->right;
                for(int k=1;k<sameLeft[j].size();k++)
                {
                    rightNode *q1 = new rightNode(sameLeft[j][k]);
                    p1->next = q1;
                    p1 = q1;
                }
                rightNode *q1 = new rightNode(upperCase[0]);
                p1->next = q1;
                p1=q1;
                p1->next=NULL;
                q->next=s;
                q=s;
            }
        }
        textNode *s = new textNode(upperCase[0]);           //加上文法规则upperCase[0]->@
        if(t1.terminalSet.indexOf('@')==-1)
            t1.terminalSet.append('@');
        s->right = new rightNode('@');
        s->right->next =NULL;
        q->next = s;
        q = s;
        q->next=NULL;
        judgeLeft[upperCase[0]] = 0;
        t1.non_terminalSet.append(upperCase[0]);
        upperCase.removeFirst();  //删除已用的非终结符


        p = t1.textG;   //将以t1.non_terminalSet[i]为左部的文法规则前n条规则删除掉
        int n1 = 0;
        q = p;
        while (p!=NULL) { //将以t1.non_terminalSet[i]为左部的文法规则前n条规则删除掉
            if(p->leftCh==t1.non_terminalSet[i])
            {
                if(q == p)
                {
                    q = p->next;
                    delete p;
                    p = q;
                    t1.textG = p;
                    n1++;
                    if(n1==n)
                        break;
                    continue;
                }
                else
                {
                    q->next = p->next;
                    delete p;
                    p = q->next;
                    n1++;
                    if(n1==n)
                        break;
                    continue;
                }
                n1++;
                if(n1==n)
                    break;
            }
            q = p;
            p = p->next;
        }
    }
    //输出存储结构中的消除左递归后的文法规则
    textNode *p =t1.textG;
    QString str = "";
    while(p!=NULL)
    {
        rightNode *p1 = p->right;
        str = str + p->leftCh + "->";
        while(p1!= NULL)
        {
            str = str + p1->rightCh;
            p1= p1->next;
        }
        str = str + "\n";
        p = p->next;
    }
    ui->textEdit_4->setText(str);

    str = "";
    str = str  + t1.start;
    ui->lineEdit_start3->setText(str);

    str = "";
    for(int i=0;i<t1.non_terminalSet.size();i++)
    {
        str = str +t1.non_terminalSet[i];
    }
    ui->lineEdit_non3->setText(str);

    str = "";
    for(int i=0;i<t1.terminalSet.size();i++)
    {
        str = str +t1.terminalSet[i];
    }
    ui->lineEdit_te3->setText(str);
}

void MainWindow::left_infer()       //最左推导
{
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->setColumnCount(3);           //三列，分析栈，输入，动作
    QStringList VStrList;
    VStrList.push_back("分析栈");
    VStrList.push_back("输入");
    VStrList.push_back("动作");
    ui->tableWidget_2->setHorizontalHeaderLabels(VStrList);
    QStack<QChar> s;            //存放分析栈
    QString sentence = ui->lineEdit->text();  //获取用户输入的句子
    sentence = sentence;
    QString sCh = "";   //存放分析栈的数据
    s.push('$');  //终止符号入栈
    s.push(t1.start); //文法开始符号入栈
    sCh.append('$');
    sCh.append(t1.start);
    while(!s.empty())
    {
        QChar ch = s.pop(); //取出栈顶符号
        if(ch == sentence[0])
        {
            if(sentence[0] == '$')
            {
                int count = ui->tableWidget_2->rowCount();
                ui->tableWidget_2->insertRow(count);
                ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                ui->tableWidget_2->setItem(count,2,new QTableWidgetItem("接受"));
                sCh = sCh.left(sCh.size()-1);  //取出栈顶符号
                break;
            }
            else
            {
                int count = ui->tableWidget_2->rowCount();
                ui->tableWidget_2->insertRow(count);
                ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                ui->tableWidget_2->setItem(count,2,new QTableWidgetItem("匹配"));
                sCh = sCh.left(sCh.size()-1);  //取出栈顶符号
                sentence.remove(0,1);
                continue;
            }

        }
        else
        {
            if(firstMap[ch].indexOf(sentence[0]) != -1)
            {
                if(followMap[ch].indexOf(sentence[0])!=-1 && firstMap[ch].indexOf('@')!=-1 && sentence[1] == "$")
                {
                    int count = ui->tableWidget_2->rowCount();
                    QString t1 = "";
                    t1 = t1  + ch + "->" + "@";
                    ui->tableWidget_2->insertRow(count);
                    ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                    ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                    ui->tableWidget_2->setItem(count,2,new QTableWidgetItem(t1));
                    sCh = sCh.left(sCh.size()-1);  //取出栈顶符号
                    continue;
                }
                textNode *p=t1.textG;
                while(p!=NULL)
                {
                    if(p->leftCh == ch)
                    {
                        rightNode *p1=p->right;
                        if(firstMap[p1->rightCh].indexOf(sentence[0]) != -1)
                        {
                            int count = ui->tableWidget_2->rowCount();
                            QString rule = "";
                            while(p1!=NULL)
                            {
                                rule = rule + p1->rightCh;
                                p1 = p1->next;
                            }
                            for(int i = rule.size()-1;i>=0;i--)
                            {
                                s.push(rule[i]);
                            }
                            QString t1 = "";
                            t1 = t1  + ch + "->" + rule;
                            ui->tableWidget_2->insertRow(count);
                            ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                            ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                            ui->tableWidget_2->setItem(count,2,new QTableWidgetItem(t1));
                            sCh = sCh.left(sCh.size()-1);  //取出栈顶符号
                            for(int i = rule.size()-1;i>=0;i--)
                            {
                                sCh = sCh +rule[i];
                            }
                            break;
                        }
                    }
                    p = p->next;
                }
            }
            else
            {
                if(followMap[ch].indexOf(sentence[0])!=-1)
                {
                    if(firstMap[ch].indexOf('@')==-1)
                    {
                        QString t1 = "";
                        t1 = t1 + "此刻输入" +sentence[0] + "出错！";
                        int count = ui->tableWidget_2->rowCount();
                        ui->tableWidget_2->insertRow(count);
                        ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                        ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                        ui->tableWidget_2->setItem(count,2,new QTableWidgetItem(t1));
                        break;
                    }
                    int count = ui->tableWidget_2->rowCount();
                    QString t1 = "";
                    t1 = t1  + ch + "->" + "@";
                    ui->tableWidget_2->insertRow(count);
                    ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                    ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                    ui->tableWidget_2->setItem(count,2,new QTableWidgetItem(t1));
                    sCh = sCh.left(sCh.size()-1);  //取出栈顶符号
                }
                else
                {
                    QString t1 = "";
                    t1 = t1 + "此刻输入" +sentence[0] + "出错！";
                    int count = ui->tableWidget_2->rowCount();
                    ui->tableWidget_2->insertRow(count);
                    ui->tableWidget_2->setItem(count,0,new QTableWidgetItem(sCh));
                    ui->tableWidget_2->setItem(count,1,new QTableWidgetItem(sentence));
                    ui->tableWidget_2->setItem(count,2,new QTableWidgetItem(t1));
                    break;
                }
            }
        }
    }
}

QVector<QChar> MainWindow::first(QChar ch)
{
    QVector<QChar> temp;
    textNode *p = t1.textG;     //获取文法规则
    firstMap[ch] = temp;

    if(t1.terminalSet.indexOf(ch)!=-1)      //若为终结符则first为自己，返回
    {
        firstMap[ch].append(ch);
        return firstMap[ch];
    }
    while(p!=NULL)
    {
        if(p->leftCh == ch)
        {
            temp.clear();
            rightNode *p1 = p->right;
            while(p1!=NULL)
            {
                if(t1.terminalSet.indexOf(p1->rightCh)!=-1)//若为终结符号，直接加入，否则求first
                {
                    temp.append(p1->rightCh);
                    break;
                }
                else
                {
                    QVector<QChar> temp1;
                    if(firstMap.contains(p1->rightCh))
                        temp1 = firstMap[p1->rightCh];
                    else
                        temp1 = first(p1->rightCh);
                    QVector<QChar> temp2 = temp1;
                    temp2.removeAll('@');
                    temp = this->vectors_set_union(temp,temp2);
                    if(temp1.indexOf('@') == -1)
                        break;
                    p1 = p1->next;
                }
            }
            if(p1 == NULL)
                temp.append('@');
            firstMap[ch] = this->vectors_set_union(firstMap[ch],temp);
        }
        p = p->next;
    }
    return firstMap[ch];
}


void MainWindow::follow()
{
    while(true)
    {
        tempMap = followMap;
        textNode *p = t1.textG;
        while(p!=NULL)
        {
            rightNode *p1 = p->right;
            while(p1!=NULL)
            {
                if(t1.non_terminalSet.indexOf(p1->rightCh)!=-1)
                {
                    rightNode *q1 = p1->next;
                    if(q1==NULL)
                    {
                        followMap[p1->rightCh] = this->vectors_set_union(followMap[p1->rightCh],followMap[p->leftCh]);
                    }
                    else
                    {
                        while(q1!=NULL)
                        {
                            if(t1.terminalSet.indexOf(q1->rightCh)!=-1)
                            {
                                if(followMap[p1->rightCh].indexOf(q1->rightCh) == -1)
                                    followMap[p1->rightCh].append(q1->rightCh);
                                break;
                            }
                            else
                            {
                                QVector<QChar> temp = firstMap[q1->rightCh];
                                if(temp.indexOf('@')==-1)
                                {
                                    followMap[p1->rightCh] = this->vectors_set_union(followMap[p1->rightCh],temp);
                                    break;
                                }
                                else
                                {
                                    temp.removeAll('@');
                                    followMap[p1->rightCh] = this->vectors_set_union(followMap[p1->rightCh],temp);
                                    q1 = q1->next;
                                }
                            }
                        }
                        if(q1 == NULL)
                        {
                            followMap[p1->rightCh] = this->vectors_set_union(followMap[p1->rightCh],followMap[p->leftCh]);
                        }
                    }
                }
                p1 = p1->next;
            }
            p = p->next;
        }
        if(followMap == tempMap)
            break;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
