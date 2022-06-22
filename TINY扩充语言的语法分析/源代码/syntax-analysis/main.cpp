#include "mainwindow.h"
#include <QApplication>
#include<QString>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include"globals.h"

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

QString treePrint = "";

int EOF_flag = FALSE;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;

int Error = FALSE;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
