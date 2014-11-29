#include <algorithm>
#include <sstream>
#include "ast.hh"
#include "flowcontrol.hh"
using namespace std;

string get_pos(AstNode *x) { //taken from type_checker
        int line, col;
        line = x->ini_line();
        col  = x->ini_col();
        string res = to_string(line) + "," + to_string(col+1) + ": ";
        return res;
}

void add_error(AstNode *x, string msg) {
   string pos = get_pos(x);
   string _msg = pos + msg;
   Error *err = new Error(x->ini, _msg);
   x->errors.push_back(err);
}

void add_warning(AstNode *x, string msg) {
   //stub implementation
   add_error(x, msg);
}

//nothing to do for these functions
void FlowControl::visit_macro(Macro* x) {}
void FlowControl::visit_typespec(TypeSpec *x) {}
void FlowControl::visit_enumdecl(EnumDecl *x) {}
void FlowControl::visit_typedefdecl(TypedefDecl *x) {}
void FlowControl::visit_structdecl(StructDecl *x) {}
void FlowControl::visit_paramdecl(ParamDecl *x) {}



void FlowControl::visit_program(Program* x) {
   for (AstNode* n : x->nodes) {
      n->accept(this);
   }
}

void FlowControl::visit_include(Include* x) {
   string s = x->filename;
   string D1 = (x->global ? "<" : "\"");
   string D2 = (x->global ? ">" : "\"");
   if (includes.count(s)) {
      add_error(x, "repeated include "+D1+s+D2);
   }
   includes.insert(s);
}

void FlowControl::visit_using(Using* x) {
   string s = x->namespc;
   if (namespcs.count(s)) {
      add_error(x, "repeated namespace '"+s+"'");
   }
   namespcs.insert(s);
}



void FlowControl::visit_funcdecl(FuncDecl *x) {
   if (x->block) {
      //initialize function declaration-related variables:
      returns_void = x->return_typespec->id->name.compare("void") == 0;
      param_by_nonconst_ref = false;
      for (int i = 0; i < x->params.size(); i++) {
         TypeSpec* param_type = x->params[i]->typespec;
         if (param_type->reference and not param_type->is_const()) {
            param_by_nonconst_ref = true;
         }
      }
     
      //these ones are updated inside the block:
      has_io_actions = false;
      has_some_return = false;

      x->block->accept(this);

      if (not returns_void and not has_some_return and x->id->name.compare("main") != 0) {
         add_error(x, "function '"+x->id->name+"' does not have any return statement");
      }

      if (returns_void and not param_by_nonconst_ref and not has_io_actions) {
         add_warning(x, "function '"+x->id->name+"' does not do anything");
      }

   }
}

void FlowControl::visit_block(Block *x) {
   bool is_bool_lit_expr_copy = is_bool_lit_expr;
   bool bool_lit_expr_copy = bool_lit_expr;

   if (x->stmts.empty()) {
      is_return = false;
      is_bool_lit_expr = false;
      return;
   }
   int n = (int) x->stmts.size();
   int i = 0;
   for (Stmt *s : x->stmts) {
      is_return = false; //updated inside the statements
      is_break = false;
      is_continue = false;
      s->accept(this);
      if (i < n-1) {
         if (is_return) {
            add_warning(s, "unreacheable instructions after return");
         }
         if (inside_loop and is_break) {
            add_warning(s, "unreacheable instructions after break");
         }
         if (inside_loop and is_continue) {
            add_warning(s, "unreacheable instructions after continue");
         }
      }
      if (is_return) {
         has_some_return = true;
      }
      i++;
   }
   is_return = false;
   is_break = false;
   is_continue = false;

   bool_lit_expr = bool_lit_expr_copy;
   is_bool_lit_expr = is_bool_lit_expr_copy;
}

void FlowControl::visit_ident(Ident *x) {
   //out) << "id:";
   if (!x->prefix.empty()) {
      //out) << '[';
      for (int i = 0; i < x->prefix.size(); i++) {
         if (i > 0) {
            //out) << ", ";
         }
         x->prefix[i]->accept(this);
      }
      //out) << ']';
   }
   //out) << "'" << x->name << "'";
   if (!x->subtypes.empty()) {
      //out) << "<";
      for (int i = 0; i < x->subtypes.size(); i++) {
         if (i > 0) {
            //out) << ", ";
         }
         x->subtypes[i]->accept(this);
      }
      //out) << ">";
   }
}

void FlowControl::visit_literal(Literal *x) {
   if (x->paren) {
      //out) << "(";
   }
   switch (x->type) {
   case Literal::Int:
      is_bool_lit_expr = true;
      bool_lit_expr = x->val.as_int != 0;
      is_int_lit_expr = true;
      int_lit_expr = x->val.as_int;
      break;

   case Literal::Double:
      break;

   case Literal::Bool:
      is_bool_lit_expr = true;
      bool_lit_expr = x->val.as_bool;
      break;

   case Literal::String:
      break;

   case Literal::Char:
      break;

   default:
      break;
      //out) << "Literal<>"; break;
   }
   if (x->paren) {
      //out) << ")";
   }
}

void FlowControl::visit_binaryexpr(BinaryExpr *x) {
   string op = x->op;
   if (op == "<<" or op == ">>") {
      has_io_actions = true;
   }
   if (op == "||" or op == "&&") {
      bool b1, b2;
      is_bool_lit_expr = false;
      x->left->accept(this);
      if (is_bool_lit_expr) b1 = bool_lit_expr;
      else return;
      is_bool_lit_expr = false;
      x->right->accept(this);
      if (is_bool_lit_expr) b2 = bool_lit_expr;
      else return;
      
      is_bool_lit_expr = true;
      if (op == "||") {
         bool_lit_expr = b1 || b2;
      }
      if (op == "&&") {
         bool_lit_expr = b1 && b2;
      }
   }
   if (op=="+" or op=="-" or op=="*" or op=="/" or op=="%") {
      int n1, n2, n;
      is_int_lit_expr = false;
      x->left->accept(this);
      if (is_int_lit_expr) n1 = int_lit_expr;
      else return;
      is_int_lit_expr = false;
      x->right->accept(this);
      if (is_int_lit_expr) n2 = int_lit_expr;
      else return;
      
      is_bool_lit_expr = true;
      if (op == "+") n = n1+n2;
      else if (op == "-") n = n1-n2;
      else if (op == "*") n = n1*n2;
      else if (op == "/") n = n1/n2;
      else n = n1%n2;
      bool_lit_expr = n != 0;

   }
}

void FlowControl::visit_vardecl(VarDecl *x) {
   if (x->kind == Decl::Pointer) {
      //out) << "*";
   }
   //out) << '"' << x->name << '"';
   /*
   if (x->init) {
      //out) << " = ";
      x->init->accept(this);
   }
   */
}

void FlowControl::visit_arraydecl(ArrayDecl *x) {
   //out) << '"' << x->name << "\"(Size = ";
   x->size->accept(this);
   /*
   if (x->init) {
      //out) << ", Init = ";
      x->init->accept(this);
   }
   */
   //out) << ")";
}

void FlowControl::visit_exprlist(ExprList *x) {
   //out) << "{";
   for (int i = 0; i < x->exprs.size(); i++) {
      if (i > 0) {
         //out) << ", ";
      }
      x->exprs[i]->accept(this);
   }
   //out) << "}";
}

void FlowControl::visit_objdecl(ObjDecl *x) {
   //out) << '"' << x->name << "\"(";
   if (!x->args.empty()) {
      //out) << "Args = {";
      for (int i = 0; i < x->args.size(); i++) {
         if (i > 0) {
            //out) << ", ";
         }
         x->args[i]->accept(this);
      }
      out () << "}";
   }
   //out) << ")";
}

void FlowControl::visit_declstmt(DeclStmt* x) {
   //out) << "DeclStmt(";
   x->typespec->accept(this);
   //out) << ", Vars = {";
   bool first = true;
   for (DeclStmt::Item item : x->items) {
      if (!first) {
         //out) << ", ";
      }
      item.decl->accept(this);
      if (item.init) {
         //out) << " = ";
         item.init->accept(this);
      }
      first = false;
   }
   //out) << "})";
}

void FlowControl::visit_exprstmt(ExprStmt* x) {
   if (x->is_return) {
      is_return = true;
      has_exit_stmt = true;
   }
   if (x->expr) {
      x->expr->accept(this);
   }
}

void FlowControl::visit_ifstmt(IfStmt *x) {
   bool is_bool_lit_expr_copy = is_bool_lit_expr;
   bool bool_lit_expr_copy = bool_lit_expr;
   is_bool_lit_expr = false;
   x->cond->accept(this);
   if (is_bool_lit_expr) {
      if (bool_lit_expr) {
         add_warning(x->cond, "the if condition is always true");
      }
      else {
         add_warning(x->cond, "the if condition is always false");
      }
   }
   
   x->then->accept(this);
   if (x->els) {
      x->els->accept(this);
   }
   is_return = false;
   bool_lit_expr = bool_lit_expr_copy;
   is_bool_lit_expr = is_bool_lit_expr_copy;
}

void FlowControl::visit_iterstmt(IterStmt *x) {
   bool is_bool_lit_expr_copy = is_bool_lit_expr;
   bool bool_lit_expr_copy = bool_lit_expr;

   bool cond_bool_lit_expr;
   if (x->is_for()) {
      x->init->accept(this);
      is_bool_lit_expr = false; //modified inside the condition
      x->cond->accept(this);
      cond_bool_lit_expr = is_bool_lit_expr;
      x->post->accept(this);
   } else {
      is_bool_lit_expr = false;
      x->cond->accept(this);
      cond_bool_lit_expr = is_bool_lit_expr;
   }
   has_exit_stmt = false;

   bool inside_loop_copy = inside_loop;
   inside_loop = true;
   x->substmt->accept(this);
   inside_loop = inside_loop_copy;

   if (cond_bool_lit_expr) {
      if (not bool_lit_expr) {
         if (x->is_for()) {
            add_warning(x->cond, "the for condition is always false");
         }
         else {
            add_warning(x->cond, "the while condition is always false");
         }
      }
      else if (not has_exit_stmt) {
         if (x->is_for()) add_error(x, "infinite for loop");
         else add_error(x, "infinite while loop");
      }
   }
   is_return = false;
   is_bool_lit_expr = false;

   bool_lit_expr = bool_lit_expr_copy;
   is_bool_lit_expr = is_bool_lit_expr_copy;
}

void FlowControl::visit_jumpstmt(JumpStmt *x) {
   string keyword[3] = { "break", "continue", "goto" };
   string kind = keyword[x->kind];
   if (kind == "goto") {
      add_warning(x, "using a goto instruction");
   }
   if (kind == "goto" or kind == "break") {
      has_exit_stmt = true;
   }
   if (kind == "break" and not inside_loop) {
      add_error(x, "break statement outside loop");
   }
   if (kind == "continue" and not inside_loop) {
      add_error(x, "continue statement outside loop");
   }
   if (kind == "continue") {
      is_continue = true;
   }
   if (kind == "break") {
      is_break = true;
   }

   if (x->kind == JumpStmt::Goto) {
      //out) << '"' << x->label << '"';
   }
   //out) << ")";
}

void FlowControl::visit_callexpr(CallExpr *x) {
   //out) << "CallExpr(";
   x->func->accept(this);
   //out) << ", Args = {";
   for (int i = 0; i < x->args.size(); i++) {
      if (i > 0) {
         //out) << ", ";
      }
      x->args[i]->accept(this);
   }
   //out) << "})";
}

void FlowControl::visit_indexexpr(IndexExpr *x) {
   //out) << "IndexExpr(";
   x->base->accept(this);
   //out) << ", ";
   x->index->accept(this);
   //out) << ")";
}

void FlowControl::visit_fieldexpr(FieldExpr *x) {
   //out) << "FieldExpr";
   if (x->pointer) {
      //out) << "<pointer>";
   }
   //out) << "(";
   x->base->accept(this);
   //out) << ", ";
   x->field->accept(this);
   //out) << ")";
}

void FlowControl::visit_condexpr(CondExpr *x) {
   if (x->paren) {
      //out) << "(";
   }
   //out) << "CondExpr(";
   x->cond->accept(this);
   //out) << ", ";
   x->then->accept(this);
   //out) << ", ";
   x->els->accept(this);
   //out) << ")";
   if (x->paren) {
      //out) << ")";
   }
}

void FlowControl::visit_signexpr(SignExpr *x) {
   //out) << "SignExpr<";
   //out) << (x->kind == SignExpr::Positive ? "+" : "-");
   //out) << ">(";
   x->expr->accept(this);
   //out) << ")";
}

void FlowControl::visit_increxpr(IncrExpr *x) {
   //out) << "IncrExpr<";
   //out) << (x->kind == IncrExpr::Positive ? "++" : "--") << ", ";
   //out) << (x->preincr ? "pre" : "post");
   //out) << ">(";
   x->expr->accept(this);
   //out) << ")";
}

void FlowControl::visit_negexpr(NegExpr *x) {
   //out) << "NegExpr(";
   x->expr->accept(this);
   //out) << ")";
}

void FlowControl::visit_addrexpr(AddrExpr *x) {
   //out) << "AddrExpr(";
   x->expr->accept(this);
   //out) << ")";
}

void FlowControl::visit_derefexpr(DerefExpr *x) {
   //out) << "DerefExpr(";
   x->expr->accept(this);
   //out) << ")";
}

void FlowControl::visit_errorstmt(Stmt::Error *x) {
   //out) << "ErrorStmt(\"" << x->code << "\")";
}

void FlowControl::visit_errorexpr(Expr::Error *x) {
   //out) << "ErrorExpr(\"" << x->code << "\")";
}
