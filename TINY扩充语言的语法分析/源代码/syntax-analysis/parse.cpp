/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include<QObject>

static TokenType token; /* holds current token */

static TokenType temp;

char *tempString;

/* function prototypes for recursive calls */
static TreeNode * stmt_sequence(void);
static TreeNode * statement(void);
static TreeNode * if_stmt(void);
static TreeNode * repeat_stmt(void);
static TreeNode * assign_stmt(void);
static TreeNode * regular_stmt(void);
static TreeNode * read_stmt(void);
static TreeNode * write_stmt(void);
static TreeNode * dowhile_stmt(void);
static TreeNode * for_stmt(void);
static TreeNode * regular_stmt(void);
static TreeNode * regular_exp(void);
static TreeNode * regular_term(void);
static TreeNode * regular_term2(void);
static TreeNode * regular_factor(void);
static TreeNode * exp(void);
static TreeNode * exp_term(void);
static TreeNode * exp_factor(void);
static TreeNode * comparison_exp(void);
static TreeNode * simple_exp(void);
static TreeNode * term(void);
static TreeNode * term2(void);
static TreeNode * factor(void);

static void syntaxError(char * message)
{
    QString num = QString::number(lineno);
    QString str = QString(QLatin1String(message));
    treePrint = treePrint + QString::fromStdString("\n>>> ") + QObject::tr("Syntax error at line %1: %2").arg(num,str);
//    fprintf(listing,"\n>>> ");
//    fprintf(listing,"Syntax error at line %d: %s",lineno,message);
    Error = TRUE;
}

static void match(TokenType expected)
{ if (token == expected) token = getToken();
  else {
    syntaxError((char *)"unexpected token -> ");
    printToken(token,tokenString);
    treePrint = treePrint + QString::fromStdString("      ");
//    fprintf(listing,"      ");
  }
}

TreeNode * stmt_sequence(void)
{ TreeNode * t = statement();
  TreeNode * p = t;
  while ((token!=ENDFILE) && (token!=END) &&
         (token!=ELSE) && (token!=UNTIL) && (token!=ENDDO) && (token!=WHILE))
  { TreeNode * q;
    match(SEMI);
    q = statement();
    if (q!=NULL) {
      if (t==NULL) t = p = q;
      else /* now p cannot be NULL either */
      { p->sibling = q;
        p = q;
      }
    }
  }
  return t;
}


//P394
//lineno: 961
TreeNode * statement(void)
{ TreeNode * t = NULL;
  switch (token) {
    case IF : t = if_stmt(); break;
    case REPEAT : t = repeat_stmt(); break;
    case ID :
        tempString = copyString(tokenString);
        temp = getToken();
        if(temp == ASSIGN || temp == MINUSASSIGN)
            t = assign_stmt();
        else if(temp == REASSIGN)
            t = regular_stmt();
        break;
    case READ : t = read_stmt(); break;
    case WRITE : t = write_stmt(); break;
    case DO : t = dowhile_stmt(); break;
    case FOR : t = for_stmt(); break;
    default : syntaxError((char*)"unexpected token -> ");
              printToken(token,tokenString);
              token = getToken();
              break;
  } /* end case */
  return t;
}


//P394
//lineno: 977
TreeNode * if_stmt(void)
{ TreeNode * t = newStmtNode(IfK);
  match(IF);
  if (t!=NULL) t->child[0] = exp();
  match(THEN);
  if (t!=NULL) t->child[1] = stmt_sequence();
  if (token==ELSE) {
    match(ELSE);
    if (t!=NULL) t->child[2] = stmt_sequence();
  }
  match(END);
  return t;
}

//P394
//lineno:991
TreeNode * repeat_stmt(void)
{ TreeNode * t = newStmtNode(RepeatK);
  match(REPEAT);
  if (t!=NULL) t->child[0] = stmt_sequence();
  match(UNTIL);
  if (t!=NULL) t->child[1] = exp();
  return t;
}

TreeNode * assign_stmt(void)
{
    TreeNode * t = newStmtNode(AssignK);
    if ((t!=NULL) && (token==ID))
        t->attr.name = copyString(tempString);
    token = temp;
    if(token==ASSIGN)
    {
        t->assignType = ASSIGN;
        match(ASSIGN);
    }
    else
    {
        t->assignType = MINUSASSIGN;
        match(MINUSASSIGN);
    }
    if (t!=NULL) t->child[0] = exp();
    return t;
}

TreeNode * regular_stmt(void)
{
    TreeNode * t = newStmtNode(RegularAssignK);
    if ((t!=NULL) && (token==ID))
        t->attr.name = copyString(tempString);
    token = temp;
    match(REASSIGN);
    if (t!=NULL) t->child[0] = regular_exp();
    return t;
}

TreeNode * read_stmt(void)
{ TreeNode * t = newStmtNode(ReadK);
  match(READ);
  if ((t!=NULL) && (token==ID))
    t->attr.name = copyString(tokenString);
  match(ID);
  return t;
}

TreeNode * write_stmt(void)
{ TreeNode * t = newStmtNode(WriteK);
  match(WRITE);
  if (t!=NULL) t->child[0] = exp();
  return t;
}

TreeNode * dowhile_stmt(void)
{
    TreeNode * t = newStmtNode(DowhileK);
    match(DO);
    if (t!=NULL) t->child[0] =stmt_sequence();
    match(WHILE);
    match(LPAREN);
    if (t!=NULL) t->child[1] =exp();
    match(RPAREN);
    return t;
}

TreeNode * for_stmt(void)
{
    TreeNode * t = newStmtNode(ForK);
    match(FOR);
    if ((t!=NULL) && (token==ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t!=NULL) t->child[0] = simple_exp();
    if(token==TO)
    {
        t->assignType = TO;
        match(TO);
    }
    else
    {
        t->assignType = DOWNTO;
        match(DOWNTO);
    }
    if (t!=NULL) t->child[1] = simple_exp();
    match(DO);
    if (t!=NULL) t->child[2] = stmt_sequence();
    match(ENDDO);
    return t;
}

TreeNode * regular_exp(void)
{
    TreeNode * t = regular_term();
    while (token==OR)
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = regular_term();
        }
    }
    return t;
}

TreeNode * regular_term(void)
{
    TreeNode * t = regular_term2();
    while (token==LINK)
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = regular_term2();
        }
    }
    return t;
}

TreeNode * regular_term2(void)
{
    TreeNode * t = regular_factor();
    if(token == CLO)
    {
        TreeNode * p = newExpNode(OpK);
        if(p!=NULL){
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
        }
    }
    return t;
}

TreeNode * regular_factor(void)
{
    TreeNode * t = NULL;
    switch (token) {
    case NUM :
        t = newExpNode(ConstK);
        if ((t!=NULL) && (token==NUM))
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID :
        t = newExpNode(IdK);
        if ((t!=NULL) && (token==ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        break;
    case LPAREN :
        match(LPAREN);
        t = regular_exp();
        match(RPAREN);
        break;
    default:
        syntaxError((char*)"unexpected token -> ");
        printToken(token,tokenString);
        token = getToken();
        break;
    }
    return t;
}

TreeNode * exp(void)
{
    TreeNode * t = exp_term();
    while (token==ORR)
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = exp_term();
        }
    }
    return t;
}

TreeNode * exp_term(void)
{
    TreeNode * t = exp_factor();
    while (token==AND)
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = exp_factor();
        }
    }
    return t;
}

TreeNode * exp_factor(void)
{
    TreeNode * t = NULL;
    if(token == NOT)
    {
        t = newExpNode(OpK);
        t->attr.op = token;
        if(t!=NULL){
            match(token);
            t->child[0] = comparison_exp();
        }
    }
    else
    {
        t = comparison_exp();
    }
    return t;
}
TreeNode * comparison_exp(void)
{
    TreeNode * t = simple_exp();
    if ((token==EQ)||(token==NEQ)||(token==LT)||(token==LQ)||(token==GQ)||(token==GT)) {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t!=NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

TreeNode * simple_exp(void)
{
    TreeNode * t = term();
    while ((token==PLUS)||(token==MINUS))
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

TreeNode * term(void)
{
    TreeNode * t = term2();
    while ((token==TIMES)||(token==OVER)||(token==CPMT))
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = term2();
        }
    }
    return t;
}

TreeNode * term2(void)
{
    TreeNode * t = factor();
    while (token==POWER)
    {
        TreeNode * p = newExpNode(OpK);
        if (p!=NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

TreeNode * factor(void)
{
    TreeNode * t = NULL;
    switch (token) {
        case NUM :
            t = newExpNode(ConstK);
            if ((t!=NULL) && (token==NUM))
                t->attr.val = atoi(tokenString);
            match(NUM);
            break;
        case ID :
            t = newExpNode(IdK);
            if ((t!=NULL) && (token==ID))
                t->attr.name = copyString(tokenString);
            match(ID);
            break;
        case LPAREN :
            match(LPAREN);
            t = exp();
            match(RPAREN);
            break;
        default:
            syntaxError((char*)"unexpected token -> ");
            printToken(token,tokenString);
            token = getToken();
            break;
    }
    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode * parse(void)
{ TreeNode * t;
  token = getToken();
  t = stmt_sequence();
  if (token!=ENDFILE)
    syntaxError((char*)"Code ends before file\n");
  return t;
}
