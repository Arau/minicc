#ifndef WALKER_HH
#define WALKER_HH

#include <assert.h>
#include <iostream>
#include "ast.hh"

class Walker : public AstVisitor, public ReadWriter {
   
public:
   Walker(std::ostream *o = &std::cout) : ReadWriter(o) {}

   virtual void walk(AstNode *n) = 0;

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
   void visit_binaryexpr(BinaryExpr *x);
   void visit_literal(Literal *x);
   void visit_ident(Ident *x);
   void visit_vardecl(VarDecl *x);
   void visit_arraydecl(ArrayDecl *x);
   void visit_objdecl(ObjDecl *x);
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
   void visit_paramdecl(ParamDecl *x);

   void visit_errorstmt(Stmt::Error *x);
   void visit_errorexpr(Expr::Error *x);
};

struct ErrorCollector : public Walker {
   std::vector<Error*>& errors;
   
   ErrorCollector(std::vector<Error*>& v) : errors(v) {}
   
   void walk(AstNode *n) {
      const std::vector<Error*>& ve = n->errors;
      errors.insert(errors.end(), ve.begin(), ve.end());
   }
};

inline void collect_errors(AstNode *x, std::vector<Error*>& v) {
   if (x != 0) {
      ErrorCollector e(v);
      x->accept(&e);
   }
}

#endif
