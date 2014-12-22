#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <string>
#include "ast.hh"
#include "type_checker.hh"
using namespace std;

string get_pos(AstNode *x, int i = 0) {
        int line, col;
        line = x->ini_line();
        col  = x->ini_col();
        string res = to_string(line) + "," + to_string(col+1) + ": ";
        return res;
}

void add_error(AstNode *x, string msg, int i = 0) {
   string pos = get_pos(x);
   string _msg = pos + msg;
   Error *err = new Error(x->ini, _msg);
   x->errors.push_back(err);
}

void add_warning(AstNode *x, string msg, int i = 0) {
   //stub implementation
   add_error(x, msg);
}

Struct_field::Struct_field(const Struct_field &f) {
   type = new Type(*f.type);
   name = t.name;
}

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

string Basic::to_string() {
   return name;
}

string Struct::to_string() {
   string s = "struct{";
   for (int i = 0; i < (int) fields.size(); i++) {
      if (i != 0) s += ",";
      s += fields[i].name + "(" + fields[i].type->to_string() + ")";
   }
   return s + "}";
}

string Enum::to_string() {
   string s = "enum{";
   for (int i = 0; i < (int) fields.size(); i++) {
      if (i != 0) s += ",";
      s += fields[i].name + "(" + std::to_string(fields[i].value) + ")";
   }
   return s + "}";
}

string Vector::to_string() {
   return "vector<" + content->to_string() + ">";
}

string Typedef::to_string() {
   return real->to_string();
}

string Type::to_string() {
   return "*(" + pointed->to_string() + ")";
}

string Basic::class_str() {
   return name;
}

string Struct::class_str() {
   return "struct";
}

string Enum::class_str() {
   return "enum";
}

string Vector::class_str() {
   return "vector";
}

string Typedef::class_str() {
   return name;
}

string Pointer::class_str() {
   return "pointer";
}

Type::Type() {}

Basic::Basic(TypeSpec *x) {
   name = x->id->name;
}

Struct::Struct(TypeSpec *x) {
   //not implemented yet
}

Enum::Enum(TypeSpec *x) {
   //not implemented yet
}

Vector::Vector(TypeSpec *x) {
   //not implemented yet
}

Typedef::Typedef(TypeSpec *x) {
   //not implemented yet
}

Pointer::Pointer(TypeSpec *x) {
   //not implemented yet
}

Basic::Basic(Basic *t) {
   name = t.name;
}

Struct::Struct(Struct *t) {
   fields = vector<Struct_field> (t.fields);
}

Enum::Enum(Enum *t) {
   fields = vector<Enum_field> (t.fields);
}

Vector::Vector(Vector *t) {
   content = Type(*t.content);
}

Typedef::Typedef(Typedef *t) {
   name = t.name;
   real = Type(*t.real);
}

Pointer::Pointer(Pointer *t) {
   pointer = Type(*t.pointed);
}

SymbolTable::SymbolTable() {
   scope_stack.push_back(Scope()); //scope_stack[0] is the global scope
}

void SymbolTable::insert(EnumDecl* x) {
   //search for name clash
   string id = x->name;
   if (ident2func.count(id)) {
      add_error(x, "there exists a function with the same name as the enum '"+id+"'");
      return;
   }
   for (int i = scope_stack.size()-1; i >= 0; i--) {
      if (scope_stack[i].ident2type.count(id)) {
         Type* t = scope_stack[i].ident2type.find(id)->second;
         if (i == scope_stack.size()-1) {
            add_error(x, "there exists a "+t->class_str()+" with the same name as the enum '"+id+"'");
            return;
         }
         else {
            add_warning(x, "the enum '"+id+"' overshadows a previous "+t->class_str());
         }
      }
   }
   // create the new enum
   Type* new_enum = new Type();
   new_enum->T = enm;
   for (int i = 0; i < x->values.size(); i++) {
      Enum_field f;
      f.name = x->values[i].id;
      if (x->values[i].has_val) {
         f.value =  x->values[i].val;
      }
      else if (i == 0) {
         f.value = 0;
      }
      else {
         f.value = new_enum->enum_fields[i-1].value + 1;
      }
      for (int j = 0; j < i; j++) {
         if (new_enum->enum_fields[j].name == f.name) {
            add_error(x, "the enum '"+id+"' has two fields named '"+f.name+"'");
         }
      }
      new_enum->enum_fields.push_back(f);
   }

   //insert the new enum
   scope_stack[scope_stack.size()-1].ident2type[id] = new_enum;
}

void SymbolTable::insert(TypedefDecl* x) {
   //search for name clash
   string id = x->decl->name;
   if (ident2func.count(id)) {
      add_error(x, "there exists a function with the same name as the typedef '"+id+"'");
      return;
   }
   for (int i = scope_stack.size()-1; i >= 0; i--) {
      if (scope_stack[i].ident2type.count(id)) {
         Type* t = scope_stack[i].ident2type.find(id)->second;
         if (i == scope_stack.size()-1) {
            add_error(x, "there exists a "+t->class_str()+" with the same name as the typedef '"+id+"'");
            return;
         }
         else {
            add_warning(x, "the typedef '"+id+"' overshadows a previous "+t->class_str());
         }
      }
   }
   // create the new typedef
   Type* new_alias = new Type();
   new_alias->T = alias;
   new_alias->alias_name = id;
   new_alias->real = new Type(x->decl->typespec);
   //insert the new enum
   scope_stack[scope_stack.size()-1].ident2type[id] = new_alias;
}

void SymbolTable::insert(StructDecl* x) {
   //search for name clash
   string id = x->id->name;
   if (ident2func.count(id)) {
      add_error(x, "there exists a function with the same name as the struct '"+id+"'");
      return;
   }
   for (int i = scope_stack.size()-1; i >= 0; i--) {
      if (scope_stack[i].ident2type.count(id)) {
         Type* t = scope_stack[i].ident2type.find(id)->second;
         if (i == scope_stack.size()-1) {
            add_error(x, "there exists a "+t->class_str()+" with the same name as the struct '"+id+"'");
            return;
         }
         else {
            add_warning(x, "the struct '"+id+"' overshadows a previous "+t->class_str());
         }
      }
   }
   // create the new struct
   Type* new_struct = new Type();
   new_struct->T = strct;
   for (DeclStmt *decl : x->decls) {
      for (int i = 0; i < decl->items.size(); i++) {
         Struct_field f;
         f.type = new Type(decl->typespec);
         f.name = decl->items[i].decl->name;
         for (int j = 0; j < i; j++) {
            if (new_struct->struct_fields[j].name == f.name) {
               add_error(x, "the struct '"+id"' has two fields named '"+f.name+"'");
            }
         }
         new_struct->struct_fields.push_back(f);
      }
   }
   //to do: detect repeated fields
   //insert the new enum
   scope_stack[scope_stack.size()-1].ident2type[id] = new_struct;
}

FuncHeader::FuncHeader(FuncDecl* x) {
   return_type = new Type(x->return_typespec);
   for (int i = 0; i < x->params.size(); i++) {
      FuncParam p;
      p.name = x->params[i]->name;
      p.type = new Type(x->params[i]->typespec);
      p.ref = x->params[i]->typespec->reference;
      p.init = NULL; //default value for parameters not accepted by the AST for now
      for (int j = 0; j < i; j++) {
         if (params[j].name == p.name) {
            add_error(x, "the function '"+x->id->name+"' has two parameters named '"+p.name+"'");
         }
      }
      params.push_back(p);
   }
}

bool FuncHeader::equals(FuncHeader* other) {
   if (params.size() != other->params.size()) return false;
   for (int i = 0; i < params.size(); i++) {
      if (params[i].type->to_string() != other->params[i].type->to_string()) {
         return false;
      }
   }
   return true;
}

bool SymbolTable::contains(FuncDecl* x) {
   string id = x->id->name;
   FuncHeader* func = new FuncHeader(x);
   if (ident2func.count(id)) {
      for (FuncHeader* fh : ident2func.find(id)->second) {
         if (fh->equals(func)) {
            return true;
         }
      }
   }
   return false;
}

void SymbolTable::insert(FuncDecl* x) {
   //search for name clash
   string id = x->id->name;
   if (scope_stack[0].ident2type.count(id)) {
      Type* t = scope_stack[0].ident2type.find(id)->second;
      add_error(x, "there exists a "+t->class_str()+" with the same name as the function '"+id+"'");
      return;
   }
   if (scope_stack[0].ident2value.count(id)) {
      add_error(x, "there exists a global variable "+scope_stack[0].ident2type.find(id)->first+" with the same name as the function '"+id+"'");
      return;
   }
   // search for header clash and insert new header
   FuncHeader* new_func = new FuncHeader(x);
   if (ident2func.count(id)) {
      for (FuncHeader* fh : ident2func.find(id)->second) {
         if (fh->equals(new_func)) {
            add_error(x, "there already exists a function '"+id+"' with the same type and number of parameters");
            return;
         }
      }
      ident2func[id].push_back(new_func);
   }
   else {
      ident2func[id] = vector<FuncHeader*> (0);
      ident2func[id].push_back(new_func);
   }
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
void TypeChecker::visit_paramdecl(ParamDecl *x) {}
void TypeChecker::visit_ident(Ident *x) {}
void TypeChecker::visit_typespec(TypeSpec *x) {}


// 'type' declarations //////////////////////////////

void TypeChecker::visit_enumdecl(EnumDecl *x) {
   symTable.insert(x);
}

void TypeChecker::visit_typedefdecl(TypedefDecl *x) {
   symTable.insert(x);
}

void TypeChecker::visit_structdecl(StructDecl *x) {
   symTable.insert(x);
}

void SymbolTable::insert(ParamDecl* x) {
   //search for name clashes
   string id = x->name;
   if (ident2func.count(id)) {
      add_error(x, "there exists a function with the same name as the parameter '"+id+"'");
      return;
   }
   //note that the scope_stack has exactly 2 levels, the global one and the function one
   if (scope_stack[0].ident2value.count(id)) {
      Value* v = scope_stack[0].ident2value.find(id)->second;
      add_warning(x, "the parameter '"+id+"' overshadows a global variable of type '"+v->type->to_string()+"'");
   }
   //construct new value
   Value* val = new Value();
   val->name = id;
   val->type = new Type(x->typespec);
   val->initialized = true;
   val->reference = x->typespec->reference;
   val->is_const = x->typespec->is_const();
   val->assignable = true;
   //insert it
   scope_stack[1].ident2value[id] = val;
}

void TypeChecker::visit_funcdecl(FuncDecl *x) {
   if (not x->block) {
      symTable.insert(x);
      return;
   }
   if (not symTable.contains(x)) {
      symTable.insert(x);
   }
   symTable.scope_stack.push_back(Scope());
   for (int i = 0; i < (int)x->params.size(); i++) {
      symTable.insert(x->params[i]);
   }
   x->block->accept(this);
   symTable.scope_stack.pop_back();
}

// variable declarations //////////////////////////////

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



// statements //////////////////////////////////////////////

void TypeChecker::visit_block(Block *x) {
   if (x->stmts.empty()) return;
   symTable.scope_stack.push_back(Scope());
   for (Stmt *s : x->stmts) {
      s->accept(this);
   }
   symTable.scope_stack.pop_back();
}

void TypeChecker::visit_ifstmt(IfStmt *x) {
   //check that the condition types correctly
   x->cond->accept(this);
   //and it has type bool
   if (expr_type.to_string() != "bool") {
      if (expr_type.to_string() == "int") {
         add_warning(x->cond, "the if condition is implicitly converted from type int to bool");
      }
      else {
         add_error(x->cond, "the if condition does not have type bool");
      }
   }
   symTable.scope_stack.push_back(Scope());
   x->then->accept(this);
   symTable.scope_stack.pop_back();
   if (x->els) {
      symTable.scope_stack.push_back(Scope());
      x->els->accept(this);
      symTable.scope_stack.pop_back();
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

void TypeChecker::visit_exprstmt(ExprStmt* x) {
   //out() << "ExprStmt" << (x->is_return ? "<return>" : "") << "(";
   if (x->expr) {
      x->expr->accept(this);
   }
   //out() << ")";
}

void TypeChecker::visit_jumpstmt(JumpStmt *x) {
   string keyword[3] = { "break", "continue", "goto" };
   //out() << "JumpStmt<" << keyword[x->kind] << ">(";
   if (x->kind == JumpStmt::Goto) {
      //out() << '"' << x->label << '"';
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

//expressions //////////////////////////////////////////////


void TypeChecker::visit_literal(Literal *x) {
   //simply set expr_value to the correct type
   expr_value.type.T = basic;
   expr_value.assignable = false;
   switch (x->type) {
   case Literal::Int:
      expr_value.type.basic_name = "int";
      //out() << "Int<" << x->val.as_int << ">";
      break;

   case Literal::Double:
      expr_value.type.basic_name = "double";
      break;

   case Literal::Bool:
      expr_value.type.basic_name = "bool";
      break;

   case Literal::String:
      expr_value.type.basic_name = "string";
      break;

   case Literal::Char:
      expr_value.type.basic_name = "char";
      break;

   default:
      cerr << "unexpected literal" << endl;
      break;
   }
}

void TypeChecker::visit_binaryexpr(BinaryExpr *x) {
   x->left->accept(this);
   Type left = expr_type;
   x->right->accept(this);
   Type right = expr_type;

   if (x->op == "<<") {
      //how do you handle cout?
   }
   if (x->op == ">>") {
      //how do you handle cin?
      if (not right.assignable) {
         add_error(x, "you can only read into variables");
      }
      if (right.is_const) {
         add_error(x, "you cannot read into a constant variable '"+right.name+"'");
      }
   }

   if (x->op == "=") {
      visit_binaryexpr_assignment(left, right);
      return;
   }
   if (x->op == "+=" || x->op == "-=" || x->op == "*=" || x->op == "/=" ||
       x->op == "&=" || x->op == "|=" || x->op == "^=") {
      visit_binaryexpr_op_assignment(x->op[0], left, right);
      return;
   } 
   else if (x->op == "&" || x->op == "|" || x->op == "^") {
      bool ret = false;
      switch (x->op[0]) {
      case '&': ret = visit_bitop<_And>(left, right); break;
      case '|': ret = visit_bitop<_Or >(left, right); break;
      case '^': ret = visit_bitop<_Xor>(left, right); break;
      }
      if (ret) {
         return;
      }
      _error(_T("Los operandos de '%s' son incompatibles", x->op.c_str()));
   }
   else if (x->op == "+" || x->op == "*" || x->op == "-" || x->op == "/") {
      bool ret = false;
      switch (x->op[0]) {
      case '+': {
         if (left.is<String>() and right.is<String>()) {
            _curr = Value(left.as<String>() + right.as<String>());
            ret = true;
         } else {
            ret = visit_sumprod<_Add>(left, right);
         }
         break;
      }
      case '*': ret = visit_sumprod<_Mul>(left, right); break;
      case '-': ret = visit_sumprod<_Sub>(left, right); break;
      case '/': ret = visit_sumprod<_Div>(left, right); break;
      }
      if (ret) {
         return;
      }
      _error(_T("Los operandos de '%s' son incompatibles", x->op.c_str()));
   }
   else if (x->op == "%") {
      if (left.is<Int>() and right.is<Int>()) {
         _curr = Value(left.as<Int>() % right.as<Int>());
         return;
      }
      _error(_T("Los operandos de '%s' son incompatibles", "%"));
   }
   else if (x->op == "%=") {
      if (!left.is<Reference>()) {
         _error(_T("Para usar '%s' se debe poner una variable a la izquierda", x->op.c_str()));
      }
      left = Reference::deref(left);
      if (left.is<Int>() and right.is<Int>()) {
         left.as<Int>() %= right.as<Int>();
         return;
      }
      _error(_T("Los operandos de '%s' son incompatibles", "%="));
   }
   else if (x->op == "&&" or x->op == "and" || x->op == "||" || x->op == "or")  {
      if (left.is<Bool>() and right.is<Bool>()) {
         _curr = Value(x->op == "&&" or x->op == "and" 
                       ? left.as<Bool>() and right.as<Bool>()
                       : left.as<Bool>() or  right.as<Bool>());
         return;
      }
      _error(_T("Los operandos de '%s' no son de tipo 'bool'", x->op.c_str()));
   }
   else if (x->op == "==" || x->op == "!=") {
      if (left.same_type_as(right)) {
         _curr = Value(x->op == "==" ? left.equals(right) : !left.equals(right));
         return;
      }
      _error(_T("Los operandos de '%s' no son del mismo tipo", x->op.c_str()));
   }
   else if (x->op == "<" || x->op == ">" || x->op == "<=" || x->op == ">=") {
      bool ret = false;
      if (x->op[0] == '<') {
         ret = (x->op.size() == 1 
                ? visit_comparison<_Lt>(left, right)
                : visit_comparison<_Le>(left, right));
      } else {
         ret = (x->op.size() == 1 
                ? visit_comparison<_Gt>(left, right)
                : visit_comparison<_Ge>(left, right));
      }
      if (ret) {
         return;
      }
      _error(_T("Los operandos de '%s' no son compatibles", x->op.c_str()));
   }
   _error(_T("Interpreter::visit_binaryexpr: UNIMPLEMENTED (%s)", x->op.c_str()));
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


// i dont know what is this  //////////////////////////////////////////////

void TypeChecker::visit_errorstmt(Stmt::Error *x) {
   //out() << "ErrorStmt(\"" << x->code << "\")";
}

void TypeChecker::visit_errorexpr(Expr::Error *x) {
   //out() << "ErrorExpr(\"" << x->code << "\")";
}



