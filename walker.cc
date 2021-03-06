#include <sstream>
#include "ast.hh"
#include "walker.hh"
using namespace std;

void Walker::visit_include(Include* x)       { walk(x); }
void Walker::visit_macro(Macro* x)           { walk(x); }
void Walker::visit_using(Using* x)           { walk(x); }
void Walker::visit_ident(Ident *x)           { walk(x); }
void Walker::visit_literal(Literal *x)       { walk(x); }
void Walker::visit_jumpstmt(JumpStmt *x)     { walk(x); }
void Walker::visit_errorstmt(Stmt::Error *x) { walk(x); }
void Walker::visit_errorexpr(Expr::Error *x) { walk(x); }
void Walker::visit_vardecl(VarDecl *x)       { walk(x); }
void Walker::visit_arraydecl(ArrayDecl *x)   { walk(x); }
void Walker::visit_objdecl(ObjDecl *x)       { walk(x); }
void Walker::visit_enumdecl(EnumDecl *x)     { walk(x); }

void Walker::visit_program(Program* x) {
   walk(x);
   for (AstNode* n : x->nodes) {
      n->accept(this);
   }
}

void Walker::visit_typespec(TypeSpec *x) {
   walk(x);
   x->id->accept(this);
}

void Walker::visit_typedefdecl(TypedefDecl *x) {
   walk(x);
   x->decl->accept(this);
}

void Walker::visit_structdecl(StructDecl *x) {
   walk(x);
   x->id->accept(this);
   for (auto decl : x->decls) {
      decl->accept(this);
   }
}

void Walker::visit_funcdecl(FuncDecl *x) {
   walk(x);
   x->return_typespec->accept(this);
   for (int i = 0; i < (int)x->params.size(); i++) {
      x->params[i]->accept(this);
   }
   if (x->block) {
      x->block->accept(this);
   }
}

void Walker::visit_block(Block *x) {
   walk(x);
   for (Stmt *s : x->stmts) {
      s->accept(this);
   }
}

void Walker::visit_binaryexpr(BinaryExpr *x) {
   walk(x);
   if (x->left) {
      x->left->accept(this);
   }
   if (x->right) {
      x->right->accept(this);
   }
}

void Walker::visit_declstmt(DeclStmt* x) {
   walk(x);
   x->typespec->accept(this);
   for (DeclStmt::Item item : x->items) {
      item.decl->accept(this);
      if (item.init) {
         item.init->accept(this);
      }
   }
}

void Walker::visit_exprstmt(ExprStmt* x) {
   walk(x);
   if (x->expr) {
      x->expr->accept(this);
   } 
}

void Walker::visit_ifstmt(IfStmt *x) {
   walk(x);
   x->cond->accept(this);
   x->then->accept(this);
   if (x->els) {
      x->els->accept(this);
   }
}

void Walker::visit_iterstmt(IterStmt *x) {
   walk(x);
   if (x->is_for()) {
      x->init->accept(this);
      x->cond->accept(this);
      x->post->accept(this);
   } else {
      x->cond->accept(this);
   }
   x->substmt->accept(this);
}

void Walker::visit_callexpr(CallExpr *x) {
   walk(x);
   x->func->accept(this);
   for (int i = 0; i < x->args.size(); i++) {
      x->args[i]->accept(this);
   }
}

void Walker::visit_indexexpr(IndexExpr *x) {
   walk(x);
   x->base->accept(this);
   x->index->accept(this);
}

void Walker::visit_fieldexpr(FieldExpr *x) {
   walk(x);
   x->base->accept(this);
   x->field->accept(this);
}

void Walker::visit_condexpr(CondExpr *x) {
   walk(x);
   x->cond->accept(this);
   x->then->accept(this);
   x->els->accept(this);
}

void Walker::visit_exprlist(ExprList *x) {
   walk(x);
   for (Expr *e : x->exprs) {
      e->accept(this);
   }
}

void Walker::visit_signexpr(SignExpr *x) {
   walk(x);
   x->expr->accept(this);
}

void Walker::visit_increxpr(IncrExpr *x) {
   walk(x);
   x->expr->accept(this);
}

void Walker::visit_negexpr(NegExpr *x) {
   walk(x);
   x->expr->accept(this);
}

void Walker::visit_addrexpr(AddrExpr *x) {
   walk(x);
   x->expr->accept(this);
}

void Walker::visit_derefexpr(DerefExpr *x) {
   walk(x);
   x->expr->accept(this);
}

void Walker::visit_paramdecl(ParamDecl *x) {
   walk(x);
   x->typespec->accept(this);
}