/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include<QObject>

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken( TokenType token, const char* tokenString )
{
    QString str;
  switch (token)
  { case IF:
    case THEN:
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
    case DO:
    case WHILE:
    case FOR:
    case TO:
    case DOWNTO:
    case ENDDO:
        str = QString(QLatin1String(tokenString));
        treePrint = treePrint + QObject::tr("reserved word: %1\n").arg(str);
//      fprintf(listing,
//         "reserved word: %s\n",tokenString);
      break;
    case ASSIGN:
        treePrint = treePrint + QString::fromStdString("=\n");
//        fprintf(listing,"=\n");
        break;
    case LT:
        treePrint = treePrint + QString::fromStdString("<\n");
//        fprintf(listing,"<\n");
        break;
    case EQ:
        treePrint = treePrint + QString::fromStdString("==\n");
//        fprintf(listing,"==\n");
        break;
    case LPAREN:
        treePrint = treePrint + QString::fromStdString("(\n");
//        fprintf(listing,"(\n");
        break;
    case RPAREN:
        treePrint = treePrint + QString::fromStdString(")\n");
//        fprintf(listing,")\n");
        break;
    case SEMI:
        treePrint = treePrint + QString::fromStdString(";\n");
//        fprintf(listing,";\n");
        break;
    case PLUS:
        treePrint = treePrint + QString::fromStdString("+\n");
//        fprintf(listing,"+\n");
        break;
    case MINUS:
        treePrint = treePrint + QString::fromStdString("-\n");
//        fprintf(listing,"-\n");
        break;
    case TIMES:
        treePrint = treePrint + QString::fromStdString("*\n");
//        fprintf(listing,"*\n");
        break;
    case OVER:
        treePrint = treePrint + QString::fromStdString("/\n");
//        fprintf(listing,"/\n");
        break;
    case REASSIGN:
        treePrint = treePrint + QString::fromStdString(":=\n");
//        fprintf(listing,":=\n");
        break;
    case GT:
        treePrint = treePrint + QString::fromStdString(">\n");
//        fprintf(listing,">\n");
        break;
    case GQ:
        treePrint = treePrint + QString::fromStdString(">=\n");
//        fprintf(listing,">=\n");
        break;
    case LQ:
        treePrint = treePrint + QString::fromStdString("<=\n");
//        fprintf(listing,"<=\n");
        break;
    case NEQ:
        treePrint = treePrint + QString::fromStdString("<>\n");
//        fprintf(listing,"<>\n");
        break;
    case AND:
        treePrint = treePrint + QString::fromStdString("and\n");
//        fprintf(listing,"and\n");
        break;
    case ORR:
        treePrint = treePrint + QString::fromStdString("or\n");
//        fprintf(listing,"or\n");
        break;
    case NOT:
        treePrint = treePrint + QString::fromStdString("not\n");
//        fprintf(listing,"not\n");
        break;
    case MINUSASSIGN:
        treePrint = treePrint + QString::fromStdString("-=\n");
//        fprintf(listing,"-=\n");
        break;
    case CPMT:
        treePrint = treePrint + QString::fromStdString("%\n");
//        fprintf(listing,"%\n");
        break;
    case POWER:
        treePrint = treePrint + QString::fromStdString("^\n");
//        fprintf(listing,"^\n");
        break;
    case OR:
        treePrint = treePrint + QString::fromStdString("|\n");
//        fprintf(listing,"|\n");
        break;
    case LINK:
        treePrint = treePrint + QString::fromStdString("&\n");
//        fprintf(listing,"&\n");
        break;
    case CLO:
        treePrint = treePrint + QString::fromStdString("#\n");
//        fprintf(listing,"#\n");
        break;
    case ENDFILE:
        treePrint = treePrint + QString::fromStdString("EOF\n");
//        fprintf(listing,"EOF\n");
        break;
    case NUM:
      str = QString(QLatin1String(tokenString));
      treePrint = treePrint + QObject::tr("NUM, val= %1\n").arg(str);
//      fprintf(listing,
//          "NUM, val= %s\n",tokenString);
      break;
    case ID:
      str = QString(QLatin1String(tokenString));
      treePrint = treePrint + QObject::tr("ID, name= %1\n").arg(str);
//      fprintf(listing,
//          "ID, name= %s\n",tokenString);
      break;
    case ERROR:
      str = QString(QLatin1String(tokenString));
      treePrint = treePrint + QObject::tr("ERROR: %1\n").arg(str);
//      fprintf(listing,
//          "ERROR: %s\n",tokenString);
      break;
    default: /* should never happen */
      str = QString::number(token);
      treePrint = treePrint + QObject::tr("Unknown token: %1\n").arg(str);
//      fprintf(listing,"Unknown token: %d\n",token);
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++) t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = (char*)malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    treePrint = treePrint + QString::fromStdString(" ");
//    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree )
{ int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    {
        QString str;
        switch (tree->kind.stmt) {
            case IfK:
              treePrint = treePrint + QString::fromStdString("If\n");
    //          fprintf(listing,"If\n");
              break;
            case RepeatK:
              treePrint = treePrint + QString::fromStdString("Repeat\n");
    //          fprintf(listing,"Repeat\n");
              break;
            case AssignK:
              if(tree->assignType == ASSIGN)
              {
                  str = QString(QLatin1String(tree->attr.name));
                  treePrint = treePrint + QObject::tr("Assign to: %1\n").arg(str);
              }
    //            fprintf(listing,"Assign to: %s\n",tree->attr.name);
              else
              {
                  str = QString(QLatin1String(tree->attr.name));
                  treePrint = treePrint + QObject::tr("Assign to(assignType is -=): %1\n").arg(str);
              }
    //            fprintf(listing,"Assign to(assignType is -=): %s\n",tree->attr.name);
              break;
            case ReadK:
                {
                    str = QString(QLatin1String(tree->attr.name));
                    treePrint = treePrint + QObject::tr("Read: %1\n").arg(str);
                }
    //          fprintf(listing,"Read: %s\n",tree->attr.name);
              break;
            case WriteK:
                treePrint = treePrint + QString::fromStdString("Write\n");
    //          fprintf(listing,"Write\n");
              break;
            case DowhileK:
                treePrint = treePrint + QString::fromStdString("Dowhile\n");
    //          fprintf(listing,"Dowhile\n");
              break;
            case ForK:
              if(tree->assignType == TO)
              {
                  str = QString(QLatin1String(tree->attr.name));
                  treePrint = treePrint + QObject::tr("For(type = to) Assign to: %1\n").arg(str);
              }
    //            fprintf(listing,"For(type = to) Assign to: %s\n",tree->attr.name);
              else
              {
                  str = QString(QLatin1String(tree->attr.name));
                  treePrint = treePrint + QObject::tr("For(type = downto) Assign to: %1\n").arg(str);
              }
    //            fprintf(listing,"For(type = downto) Assign to: %s\n",tree->attr.name);
              break;
            case RegularAssignK:
                str = QString(QLatin1String(tree->attr.name));
                treePrint = treePrint + QObject::tr("RegularAssign to: %1\n").arg(str);
    //          fprintf(listing,"RegularAssign to: %s\n",tree->attr.name);
              break;
            default:
                treePrint = treePrint + QString::fromStdString("Unknown ExpNode kind\n");
    //          fprintf(listing,"Unknown ExpNode kind\n");
              break;
        }
    }
    else if (tree->nodekind==ExpK)
    {
        QString str;
        switch (tree->kind.exp) {
            case OpK:
                treePrint = treePrint + QString::fromStdString("Op: ");
    //          fprintf(listing,"Op: ");
              printToken(tree->attr.op,"\0");
              break;
            case ConstK:
                str = QString::number(tree->attr.val);
                treePrint = treePrint + QObject::tr("Const: %1\n").arg(str);
    //          fprintf(listing,"Const: %d\n",tree->attr.val);
              break;
            case IdK:
                str = QString(QLatin1String(tree->attr.name));
                treePrint = treePrint + QObject::tr("Id: %1\n").arg(str);
    //          fprintf(listing,"Id: %s\n",tree->attr.name);
              break;
            default:
                treePrint = treePrint + QString::fromStdString("Unknown ExpNode kind\n");
    //          fprintf(listing,"Unknown ExpNode kind\n");
              break;
          }
    }
    else
        treePrint = treePrint + QString::fromStdString("Unknown ExpNode kind\n");
//        fprintf(listing,"Unknown node kind\n");
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
