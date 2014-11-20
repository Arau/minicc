#include <algorithm>
#include <sstream>
#include <string>
#include "ast.hh"
#include "type_checker.hh"
using namespace std;


string TypeChecker::get_pos(AstNode *x) {
        int line, col;
        line = x->ini_line();
        col  = x->ini_col();
        string res = to_string(line) + "," + to_string(col+1) + ": ";
        return res;
}

bool TypeChecker::is_boolean_expr(AstNode *x) {
   return _curr == "bool";
}

void TypeChecker::visit_program(Program* x) {
   //out() << "Program{" << endl;
   indent(+1);
   for (AstNode* n : x->nodes) {
      //out(beginl);
      n->accept(this);
      //out() << endl;
   }
   indent(-1);
   //out(beginl) << "}" << endl;
}

void TypeChecker::visit_include(Include* x) {
   string D = (x->global ? "<>" : "\"\"");
   //out() << "Include(" << D[0] << x->filename << D[1] << ")";
}

void TypeChecker::visit_macro(Macro* x) {
   //out() << "Macro(" << x->macro << ")" << endl;
}

void TypeChecker::visit_using(Using* x) {
   //out() << "Using(" << x->namespc << ")";
}

void TypeChecker::visit_typespec(TypeSpec *x) {
   //out() << "Type" << (x->reference ? "<&>" : "") << "(";
   x->id->accept(this);
   if (!x->qual.empty()) {
      //out() << ", {";
      int i = 0, numq = 0;
      for (int q = TypeSpec::Const; q <= TypeSpec::Extern; q++) {
         if (find(x->qual.begin(), x->qual.end(), q) != x->qual.end()) {
            if (numq > 0) {
               //out() << ", ";
            }
            //out() << TypeSpec::QualifiersNames[i];
            numq++;
         }
         i++;
      }
      //out() << "}";
   }
   //out() << ")";
}

void TypeChecker::visit_enumdecl(EnumDecl *x) {
   //out() << "EnumDecl(\"" << x->name << "\", {";
   for (int i = 0; i < x->values.size(); i++) {
      if (i > 0) {
         //out() << ", ";
      }
      //out() << '"' << x->values[i].id << '"';
      if (x->values[i].has_val) {
         //out() << " = " << x->values[i].val;
      }
   }
   //out() << "})";
}

void TypeChecker::visit_typedefdecl(TypedefDecl *x) {
   //out() << "TypedefDecl(\"" << x->decl->name << "\" = ";
   x->decl->typespec->accept(this);
   //out() << ")";
}

void TypeChecker::visit_structdecl(StructDecl *x) {
   //out() << "StructDecl(";
   x->id->accept(this);
   //out() << ", {" << endl;
   indent(+1);
   for (DeclStmt *decl : x->decls) {
      //out(beginl);
      decl->accept(this);
      //out() << endl;
   }
   indent(-1);
   //out(beginl) << "})";
}

void TypeChecker::visit_funcdecl(FuncDecl *x) {
   //out() << "FuncDecl(";
   x->id->accept(this);
   //out() << ", ";
   x->return_typespec->accept(this);
   //out() << ", Params = {";
   for (int i = 0; i < x->params.size(); i++) {
      if (i > 0) {
         //out() << ", ";
      }
      //out() << "\"" << x->params[i]->name << "\": ";
      x->params[i]->typespec->accept(this);
   }
   if (x->block) {
      //out() << "}, {" << endl;
      indent(+1);
      //out(beginl);
      x->block->accept(this);
      //out() << endl;
      indent(-1);
      //out(beginl);
   }
   //out() << "})";
}

void TypeChecker::visit_block(Block *x) {
   //out() << "Block(";
   if (x->stmts.empty()) {
      //out() << "{})";
      return;
   }
   //out() << "{" << endl;
   indent(+1);
   for (Stmt *s : x->stmts) {
      //out(beginl);
      s->accept(this);
      //out() << endl;
   }
   indent(-1);
   //out(beginl) << "})";
}

void TypeChecker::visit_ident(Ident *x) {
   //out() << "id:";
   if (!x->prefix.empty()) {
      //out() << '[';
      for (int i = 0; i < x->prefix.size(); i++) {
         if (i > 0) {
            //out() << ", ";
         }
         x->prefix[i]->accept(this);
      }
      //out() << ']';
   }
   //out() << "'" << x->id << "'";
   if (!x->subtypes.empty()) {
      //out() << "<";
      for (int i = 0; i < x->subtypes.size(); i++) {
         if (i > 0) {
            //out() << ", ";
         }
         x->subtypes[i]->accept(this);
      }
      //out() << ">";
   }
}

void TypeChecker::visit_literal(Literal *x) {
   if (x->paren) {
      //out() << "(";
   }
   switch (x->type) {
   case Literal::Int:
      _curr = "int";
      //out() << "Int<" << x->val.as_int << ">";
      break;

   case Literal::Double:
      _curr = "double";
      break;

   case Literal::Bool:
      _curr = "bool";
      break;

   case Literal::String:
      _curr = "string";
      break;

   case Literal::Char:
      _curr = "char";
      break;

   default:
      cerr << "unexpected literal" << endl;
      //out() << "Literal<>";
      break;
   }
   if (x->paren) {
      //out() << ")";
   }
}

void TypeChecker::visit_binaryexpr(BinaryExpr *x) {
   if (x->paren) {
      //out() << "(";
   }
   switch (x->kind) {
   default:
      //out() << x->op << "(";
      if (x->left) {
         x->left->accept(this);
      }
      if (x->right) {
         //out() << ", ";
         x->right->accept(this);
      }
      //out() << ")";
   }
   if (x->paren) {
      //out() << ")";
   }
}

void TypeChecker::visit_vardecl(VarDecl *x) {
   if (x->kind == Decl::Pointer) {
      //out() << "*";
   }
   //out() << '"' << x->name << '"';
   /*
   if (x->init) {
      //out() << " = ";
      x->init->accept(this);
   }
   */
}

void TypeChecker::visit_arraydecl(ArrayDecl *x) {
   //out() << '"' << x->name << "\"(Size = ";
   x->size->accept(this);
   /*
   if (x->init) {
      //out() << ", Init = ";
      x->init->accept(this);
   }
   */
   //out() << ")";
}

void TypeChecker::visit_exprlist(ExprList *x) {
   //out() << "{";
   for (int i = 0; i < x->exprs.size(); i++) {
      if (i > 0) {
         //out() << ", ";
      }
      x->exprs[i]->accept(this);
   }
   //out() << "}";
}

void TypeChecker::visit_objdecl(ObjDecl *x) {
   //out() << '"' << x->name << "\"(";
   if (!x->args.empty()) {
      //out() << "Args = {";
      for (int i = 0; i < x->args.size(); i++) {
         if (i > 0) {
            //out() << ", ";
         }
         x->args[i]->accept(this);
      }
      //out () << "}";
   }
   //out() << ")";
}

void TypeChecker::visit_declstmt(DeclStmt* x) {
   //out() << "DeclStmt(";
   x->typespec->accept(this);
   //out() << ", Vars = {";
   bool first = true;
   for (DeclStmt::Item item : x->items) {
      if (!first) {
         //out() << ", ";
      }
      item.decl->accept(this);
      if (item.init) {
         //out() << " = ";
         item.init->accept(this);
      }
      first = false;
   }
   //out() << "})";
}

void TypeChecker::visit_exprstmt(ExprStmt* x) {
   //out() << "ExprStmt" << (x->is_return ? "<return>" : "") << "(";
   if (x->expr) {
      x->expr->accept(this);
   }
   //out() << ")";
}

void TypeChecker::visit_ifstmt(IfStmt *x) {
   x->cond->accept(this);
   if (not is_boolean_expr( x->cond )) {
        string pos = get_pos(x->cond);
        string msg = pos + "int expression in if condition";
        Error *err = new Error(x->ini, msg);
        x->errors.push_back(err);
   }
   if (x->els) {
      x->els->accept(this);
   }
}

void TypeChecker::visit_iterstmt(IterStmt *x) {
   if (x->is_for()) {
      //out() << "IterStmt<for>(";
      x->init->accept(this);
      //out() << ", ";
      x->cond->accept(this);
      //out() << ", ";
      x->post->accept(this);
      //out() << ", {" << endl;
   } else {
      //out() << "IterStmt<while>(";
      x->cond->accept(this);
      //out() << ", {" << endl;
   }
   indent(+1);
   //out(beginl);
   x->substmt->accept(this);
   //out() << endl;
   indent(-1);
   //out(beginl) << "})";
}

void TypeChecker::visit_jumpstmt(JumpStmt *x) {
   string keyword[3] = { "break", "continue", "goto" };
   //out() << "JumpStmt<" << keyword[x->kind] << ">(";
   if (x->kind == JumpStmt::Goto) {
      //out() << '"' << x->label << '"';
   }
   //out() << ")";
}

void TypeChecker::visit_callexpr(CallExpr *x) {
   //out() << "CallExpr(";
   x->func->accept(this);
   //out() << ", Args = {";
   for (int i = 0; i < x->args.size(); i++) {
      if (i > 0) {
         //out() << ", ";
      }
      x->args[i]->accept(this);
   }
   //out() << "})";
}

void TypeChecker::visit_indexexpr(IndexExpr *x) {
   //out() << "IndexExpr(";
   x->base->accept(this);
   //out() << ", ";
   x->index->accept(this);
   //out() << ")";
}

void TypeChecker::visit_fieldexpr(FieldExpr *x) {
   //out() << "FieldExpr";
   if (x->pointer) {
      //out() << "<pointer>";
   }
   //out() << "(";
   x->base->accept(this);
   //out() << ", ";
   x->field->accept(this);
   //out() << ")";
}

void TypeChecker::visit_condexpr(CondExpr *x) {
   if (x->paren) {
      //out() << "(";
   }
   //out() << "CondExpr(";
   x->cond->accept(this);
   //out() << ", ";
   x->then->accept(this);
   //out() << ", ";
   x->els->accept(this);
   //out() << ")";
   if (x->paren) {
      //out() << ")";
   }
}

void TypeChecker::visit_signexpr(SignExpr *x) {
   //out() << "SignExpr<";
   //out() << (x->kind == SignExpr::Positive ? "+" : "-");
   //out() << ">(";
   x->expr->accept(this);
   //out() << ")";
}

void TypeChecker::visit_increxpr(IncrExpr *x) {
   //out() << "IncrExpr<";
   //out() << (x->kind == IncrExpr::Positive ? "++" : "--") << ", ";
   //out() << (x->preincr ? "pre" : "post");
   //out() << ">(";
   x->expr->accept(this);
   //out() << ")";
}

void TypeChecker::visit_negexpr(NegExpr *x) {
   //out() << "NegExpr(";
   x->expr->accept(this);
   //out() << ")";
}

void TypeChecker::visit_addrexpr(AddrExpr *x) {
   //out() << "AddrExpr(";
   x->expr->accept(this);
   //out() << ")";
}

void TypeChecker::visit_derefexpr(DerefExpr *x) {
   //out() << "DerefExpr(";
   x->expr->accept(this);
   //out() << ")";
}

void TypeChecker::visit_errorstmt(Stmt::Error *x) {
   //out() << "ErrorStmt(\"" << x->code << "\")";
}

void TypeChecker::visit_errorexpr(Expr::Error *x) {
   //out() << "ErrorExpr(\"" << x->code << "\")";
}
