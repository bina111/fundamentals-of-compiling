#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QMessageBox>
#include<QFileDialog>
#include<QFile>
#include<QTextStream>
#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //保存源文件
    connect(ui->pB_save,&QPushButton::clicked,this,[=](){
        QString text1 = ui->tE_code->toPlainText();
        if(text1 == "")
        {
            QMessageBox::warning(this, tr("提示") ,tr("源程序为空，无法保存！"));
        }
        else{
            QFileDialog fileDialog;
            QString file_name = fileDialog.getSaveFileName(this,tr("Open File"),"/text",tr("Text File(*.txt)"));
            QFile file(file_name);
            if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
                return;
            QTextStream out(&file);
            out<<text1;
        }
    });

    //打开存有源程序的文件
    connect(ui->pB_open,&QPushButton::clicked,this,[=](){
        QString file_name = QFileDialog::getOpenFileName(this,"获取源程序的文本文件",".","*.txt");
        QFile file(file_name);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
        QTextStream in(&file);
        QString text1 = "";
        while(!in.atEnd())
        {
            text1 = text1 + in.readLine() + '\n';
        }
        ui->tE_code->setText(text1);
    });

    //执行
    connect(ui->pB_run,&QPushButton::clicked,this,[=](){
        //先将源代码保存在路径D:/1.txt中
        QString text1 = ui->tE_code->toPlainText();
        if(text1 == "")
        {
            QMessageBox::warning(this, tr("提示") ,tr("源程序为空，请先输入源程序！"));
        }
        else{
            QString file_name = tr("D:/1.txt");
            QFile file(file_name);
            if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
                return;
            QTextStream out(&file);
            out<<text1;
        }

        if(text1 != "")
        {
            //先清空
            treePrint.clear();
            lineno = 0;
            EchoSource = FALSE;
            EOF_flag = FALSE;

            //形成生成树
            TreeNode * syntaxTree;
            source = fopen("D:/1.txt","r");
            if (source==NULL)
            {
               QMessageBox::warning(this,"警告",QObject::tr("File %1 not found\n").arg("D:/1.txt"));
               exit(1);
            }
            listing = stdout; /* send listing to screen */
            treePrint = treePrint + QString::fromStdString("Syntax tree:\n");
            syntaxTree = parse();
            printTree(syntaxTree);
            fclose(source);
        }

    });

    //查看生成树
    connect(ui->pB_look,&QPushButton::clicked,this,[=](){
        ui->tE_parseTree->setText(treePrint);
    });

    //退出程序
    connect(ui->pB_exit,&QPushButton::clicked,this,[=](){
        exit(0);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
