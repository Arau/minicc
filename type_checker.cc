#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <string>
#include "ast.hh"
#include "type_checker.hh"
using namespace std;



string Type::to_string() {
   if (T == basic) {
      return basic_name;
   }
   if (T == strct) {
      string s = "struct{";
      for (int i = 0; i < (int) struct_fields.size(); i++) {
         if (i != 0) s += ",";
         s += struct_fields[i].name + "(" + struct_fields[i].type->to_string() + ")";
      }
      return s + "}";
   }
   if (T == enm) {
      string s = "enum{";
      for (int i = 0; i < (int) enum_fields.size(); i++) {
         if (i != 0) s += ",";
         s += enum_fields[i].name + "(" + std::to_string(enum_fields[i].value) + ")";
      }
      return s + "}";
   }
   if (T == vec) {
      return "vector<" + content->to_string() + ">";
   }
   if (T == alias) {
      //return "alias(" + alias_name + ":" + to_string(real) + ")";
      return real->to_string();
   }
   if (T == pointer) {
      return "*(" + pointed->to_string() + ")";
   }
   return "cannot reach here";
}


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
   symTable = SymbolTable();
   for (AstNode* n : x->nodes) {
      n->accept(this);
   }
}

void TypeChecker::visit_include(Include* x) {}

void TypeChecker::visit_macro(Macro* x) {}

void TypeChecker::visit_using(Using* x) {}

void TypeChecker::visit_typespec(TypeSpec *x) {
   typespec_type = new Type;
   typespec_type.T = basic;
   typespec_type.basic_name = x->id->name; //only works correctly for basic types for now

   // //out() << "Type" << (x->reference ? "<&>" : "") << "(";
   // //x->id->accept(this);
   // if (!x->qual.empty()) {
   //    //out() << ", {";
   //    int i = 0, numq = 0;
   //    for (int q = TypeSpec::Const; q <= TypeSpec::Extern; q++) {
   //       if (find(x->qual.begin(), x->qual.end(), q) != x->qual.end()) {
   //          if (numq > 0) {
   //             //out() << ", ";
   //          }
   //          //out() << TypeSpec::QualifiersNames[i];
   //          numq++;
   //       }
   //       i++;
   //    }
   //    //out() << "}";
   // }
   // //out() << ")";
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

void TypeChecker::visit_paramdecl(ParamDecl *x) {

}

void TypeChecker::visit_funcdecl(FuncDecl *x) {
   symTable.initCurrFuncScope(x);
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
      //x->params[i]->typespec->accept(this);
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
   if (not symTable.currentFunction->containsSymbol(x->name)) {
      string pos = get_pos(x);
      string msg = pos + "undeclared variable '" + x->name + "'";
      Error *err = new Error(x->ini, msg);
      //x->errors.push_back(err);
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
   x->typespec->accept(this);
   bool first = true;
   for (DeclStmt::Item item : x->items) {
      item.decl->accept(this);
      if (item.init) {
         item.init->accept(this);
      }
      first = false;
   }
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
        string msg = pos + _curr + " expression in if condition";
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



bool LocalScope::containsSymbol(string name, string& type) {
   auto it = ident2type.find(name);
   if (it != ident2type.end()) {
      type = it->second->type->to_string();
      return true;
   }
   return false;
}

bool LocalScope::containsSymbol(string name) {
   auto it = ident2type.find(name);
   if (it != ident2type.end()) {
      return true;
   }
   return false;
}

bool LocalScope::addSymbol(string name, string type) {
   string existingType;
   bool exists = containsSymbol(name, existingType);
   if (exists) {
      return false;
   }
   else {
      ident2type[name] = NULL;
      return true;
   }
}

void FunctionScope::pushLocalScope() {
   scopeStack.push_back(LocalScope());
}

void FunctionScope::popLocalScope() {
   scopeStack.pop_back();
}

bool FunctionScope::addSymbol(string name, string type) {
   int topIndex = scopeStack.size()-1;
   string found_type; //not used
   if (scopeStack[topIndex].containsSymbol(name, found_type)) {
      return false;
   }
   else {
      scopeStack[topIndex].addSymbol(name, type);
      return true;
   }
}

bool FunctionScope::containsSymbol(string name, string& type) {
   for (int i = scopeStack.size()-1; i >= 0; i--) {
      if (scopeStack[i].containsSymbol(name, type)) {
         return true;
      }
   }
   return false;
}

bool FunctionScope::containsSymbol(string name) {
   for (int i = scopeStack.size()-1; i >= 0; i--) {
      if (scopeStack[i].containsSymbol(name)) {
         return true;
      }
   }
   return false;
}

void SymbolTable::initCurrFuncScope(FuncDecl *x) {
   currentFunction = new FunctionScope();
   currentFunction->pushLocalScope();
   for (int i = 0; i < (int) x->params.size(); i++) {
      cout << "init func scope: " << x->id->name << endl;
      currentFunction->addSymbol(x->params[i]->name, x->params[i]->typespec->typestr());
   }
}