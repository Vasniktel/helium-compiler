# Helium programming language
_TODO_
## Grammar
```
Bool : 'true' | 'false' ;

Integer : 0 | [1-9][0-9]* ;

Real : Integer '.' [0-9]* ;

Identifier : [_a-zA-Z][_a-zA-Z0-9]* ;

ValidSymbol
: [^\n\t\r\'\"\\]
| Escape
;

// TODO
Escape
: '\' (ValidSymbol|'\''|'"'|'\\')
;

Space : '\n' | '\t' | '\r' ;

// TODO
String
: '"' (ValidSymbol|'\''|Space)* '"'
;

// TODO
Char
: '\'' (ValidSymbol|'"') '\''
;

Program : EOL* Decls? EOL* EOF ;

Decls
: Decl
| Decls EOL+ Decl
;

Decl
: Stmt
;

Stmt
: VarStmt
| Expr
;

// TODO: make type specifier optional
VarStmt
: 'var' IDENTIFIER EOL* ':' EOL* Type ( EOL* '=' Expr)?
;

Type
: IDENTIFIER
;

Expr
: Assignment
;

// s.t. change
Assignment
: Assignable '=' Assignment
| Addition
;

Addition
: Multiplication (('+'|'-') Multiplication)*
;

Multiplication
: Unary (('*'|'/') Unary)*
;

Unary
: ('-'|'+') Unary
| Primary
;

Primary
: Identifier
| Integer
| Real
| String
| Bool
| Char
| '(' EOL* Expr EOL* ')'
| IfExpr
| BlockExpr
| WhileExpr
;

Assignable
: Identifier
;

IfExpr
: 'if' '(' EOL* Expr EOL* ')' Expr (EOL* 'else' Expr)?
;

BlockExpr
: '{' EOL* Stmts? EOL* '}'
;

Stmts
: Stmt
| Stmts EOL+ Stmt
;

WhileExpr
: 'while' '(' EOL* Expr EOL* ')' Expr
;
```
