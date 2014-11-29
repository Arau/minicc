#ifndef FLOWCONTROL_HH
#define FLOWCONTROL_HH

#include <assert.h>
#include <iostream>
#include <set>
#include <string>
#include "ast.hh"

/*
Detects repeated includes (not strictly flow control, but easy to add here)

Detects repeated namespaces (same thing)

Detects functions that do nothing:
- they return void
- they don't have any param by non const reference
- they don't have any IO actions

Detects gotos (any goto is wrong!)

Detects return statements in the middle of a block,
which makes posterior instr unreacheable

Detects non-void non-main functions without return

Detects infinite loops (static expression evaluation only detects the 'true' literal for now)

Detects loops and if/else branches that are never executed

Detects unreacheable instructoins after break or continue

Detect break/contiune outside loop

To do:

Detects goto undefined labels //i think its not implemented in the parser

Detect functions that have return, but may not always reach it (if/else branches) // very hard :-)

Detect that a function is useless if it has parameters by reference BUT
they are never assigned to (be careful with function calls if they appear as ref params) //maybe this should be done with the symbol table... think about it
*/


class FlowControl : public AstVisitor, public ReadWriter {
   void print_block(Block *);

public:
   FlowControl(std::ostream *o = &std::cout) 
      : ReadWriter(o) {

      }

   void print(AstNode* x) { x->accept(this); }

   void visit_comment(CommentSeq *x);
   void visit_include(Include *x);
   void visit_macro(Macro *x);
   void visit_program(Program *x);
   void visit_using(Using *x);
   void visit_typespec(TypeSpec *x);
   void visit_funcdecl(FuncDecl *x);
   void visit_structdecl(StructDecl *x);
   void visit_typedefdecl(TypedefDecl *x);
   void visit_enumdecl(EnumDecl *x);
   void visit_block(Block *x);
   void visit_ident(Ident *x);
   void visit_binaryexpr(BinaryExpr *x); 
   void visit_vardecl(VarDecl *);
   void visit_arraydecl(ArrayDecl *);
   void visit_objdecl(ObjDecl *);
   void visit_declstmt(DeclStmt *x);
   void visit_exprstmt(ExprStmt *x);
   void visit_ifstmt(IfStmt *x);
   void visit_iterstmt(IterStmt *x);
   void visit_jumpstmt(JumpStmt *x);
   void visit_callexpr(CallExpr *x);
   void visit_indexexpr(IndexExpr *x);
   void visit_fieldexpr(FieldExpr *x);
   void visit_condexpr(CondExpr *x);
   void visit_exprlist(ExprList *x);
   void visit_signexpr(SignExpr *x);
   void visit_increxpr(IncrExpr *x);
   void visit_negexpr(NegExpr *x);
   void visit_addrexpr(AddrExpr *x);
   void visit_derefexpr(DerefExpr *x);
   void visit_literal(Literal *x);
   void visit_paramdecl(ParamDecl *x);

   void visit_errorstmt(Stmt::Error *x);
   void visit_errorexpr(Expr::Error *x);

private:

   std::set<std::string> includes; //to detect repeated includes
   
   std::set<std::string> namespcs; //same thing with namespaces
   
   //variables relevant to detect useless functions
   bool returns_void;
   bool param_by_nonconst_ref;
   bool has_io_actions;

   //variables relevant to detect unreacheable code
   bool is_return;

   //variables relevant to detect non-void functions that
   //do not reach a return
   bool has_some_return;

   //variables relevant to detect infinite loops
   bool is_bool_lit_expr;
   bool bool_lit_expr;
   bool is_int_lit_expr;
   int int_lit_expr;
   bool has_exit_stmt;
   
   //variables relevant to detect unreacheable instructions after break/continue
   bool inside_loop;
   bool is_break;
   bool is_continue;

};

#endif
