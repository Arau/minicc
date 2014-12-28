#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <string>
#include "ast.hh"
#include "type_checker.hh"
using namespace std;

const bool dbg = false;

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

//so far only supports basic types
//but it should support all
Var_type* Var_type::convertTypeSpec(const TypeSpec* x) {
   if (dbg) cout << "convert typespec" << endl;
   return new Basic_type(x->id->name);
}

Struct_field::Struct_field(TypeSpec* t, std::string n) {
   type = Var_type::convertTypeSpec(t); name = n;
}

Struct_field::Struct_field(const Struct_field &f) {
   type = f.type->copy();
   name = f.name;
}

bool Var_type::equals(Var_type* other) const { 
   string thisstring = to_string();
   string otherstring = other->to_string();
   return to_string().compare(other->to_string()) == 0;
}

bool Var_type::is_int() const {
   return to_string().compare("int") == 0;
}

bool Var_type::is_bool() const {
   return to_string().compare("bool") == 0;
}

bool Var_type::is_char() const {
   return to_string().compare("char") == 0;
}

bool Var_type::is_float() const {
   return to_string().compare("float") == 0;
}

bool Var_type::is_double() const {
   return to_string().compare("double") == 0;
}



string Basic_type::to_string() const {
   return name;
}

string Struct_type::to_string() const {
   string s = "struct{";
   for (int i = 0; i < (int) fields.size(); i++) {
      if (i != 0) s += ",";
      s += fields[i].name + "(" + fields[i].type->to_string() + ")";
   }
   return s + "}";
}

string Enum_type::to_string() const {
   string s = "enum{";
   for (int i = 0; i < (int) fields.size(); i++) {
      if (i != 0) s += ",";
      s += fields[i].name;
   }
   return s + "}";
}

string Vector_type::to_string() const {
   return "vector<" + content->to_string() + ">";
}

string Typedef_type::to_string() const {
   return real->to_string();
}

string Pointer_type::to_string() const {
   return "*(" + pointed->to_string() + ")";
}

string Basic_type::class_str() const {
   return name;
}

string Struct_type::class_str() const {
   return "struct";
}

string Enum_type::class_str() const {
   return "enum";
}

string Vector_type::class_str() const {
   return "vector";
}

string Typedef_type::class_str() const {
   return name;
}

string Pointer_type::class_str() const {
   return "pointer";
}

Struct_type::Struct_type(StructDecl *x) {
   for (DeclStmt *decl : x->decls) {
      for (int i = 0; i < decl->items.size(); i++) {
         Struct_field f(decl->typespec, decl->items[i].decl->name);
         for (int j = 0; j < i; j++) {
            if (fields[j].name == f.name) {
               add_error(x, "the struct '"+x->id->name+"' has two fields named '"+f.name+"'");
            }
         }
         fields.push_back(f);
      }
   }
}

Enum_type::Enum_type(EnumDecl *x) {
   for (int i = 0; i < x->values.size(); i++) {
      Enum_field f(x->values[i].id);
      for (int j = 0; j < i; j++) {
         if (fields[j].name == f.name) {
            add_error(x, "the enum '"+x->name+"' has two fields named '"+f.name+"'");
         }
      }
      fields.push_back(f);
   }
}

Typedef_type::Typedef_type(TypedefDecl *x) {
   name = x->decl->name;
   real = Var_type::convertTypeSpec(x->decl->typespec);
}

Basic_type::Basic_type(const Basic_type *t) {
   name = t->name;
}

Struct_type::Struct_type(const Struct_type *t) {
   fields = vector<Struct_field> (t->fields);
}

Enum_type::Enum_type(const Enum_type *t) {
   fields = vector<Enum_field> (t->fields);
}

Vector_type::Vector_type(const Vector_type *t) {
   content = t->content->copy();
}

Typedef_type::Typedef_type(const Typedef_type *t) {
   name = t->name;
   real = t->real->copy();
}

Pointer_type::Pointer_type(const Var_type* pointed) {
   pointed = pointed->copy();
}

Pointer_type::Pointer_type(const Pointer_type *t) {
   pointed = t->pointed->copy();
}

Var_type* Basic_type::copy() const {
   return new Basic_type(this);
}

Var_type* Struct_type::copy() const {
   return new Struct_type(this); 
}

Var_type* Enum_type::copy() const {
   return new Enum_type(this); 
}

Var_type* Vector_type::copy() const {
   return new Vector_type(this); 
}

Var_type* Typedef_type::copy() const {
   return new Typedef_type(this); 
}

Var_type* Pointer_type::copy() const {
   return new Pointer_type(this); 
}

SymbolTable::SymbolTable() {
   scope_stack.push_back(Scope()); //scope_stack[0] is the global scope
}

Variable::Variable(string n, Var_type* t) {
   name = n;
   type = t->copy();
}

Variable::Variable(TypeSpec* t, std::string n) {
    name = n;
    type = Var_type::convertTypeSpec(t);
    reference = t->reference;
    is_const = t->is_const();
}

void SymbolTable::check_function_clash(string id, string class_name, AstNode* x) {
   if (ident2func.count(id)) {
      add_error(x, "there exists a function with the same name as the "+class_name+" '"+id+"'");
   }
}

void SymbolTable::check_type_clash(string id, string class_name, AstNode* x) {
   for (int i = scope_stack.size()-1; i >= 0; i--) {
      if (scope_stack[i].ident2type.count(id)) {
         Var_type* t = scope_stack[i].ident2type.find(id)->second;
         if (i == scope_stack.size()-1) {
            add_error(x, "there exists a "+t->class_str()+" with the same name as the "+class_name+" '"+id+"'");
            return;
         }
         else {
            add_warning(x, "the "+class_name+" '"+id+"' overshadows a previous "+t->class_str());
            return;
         }
      }
   }
}

void SymbolTable::insert(string id, Var_type* t) {
   scope_stack[scope_stack.size()-1].ident2type[id] = t;
}

void SymbolTable::insert(string id, Variable* v) {
   scope_stack[scope_stack.size()-1].ident2variable[id] = v;
}

void SymbolTable::insert(EnumDecl* x) {
   string id = x->name;
   check_function_clash(id, "enum", x);
   check_type_clash(id, "enum", x);
   insert(id, new Enum_type(x));
}

void SymbolTable::insert(TypedefDecl* x) {
   string id = x->decl->name;
   check_function_clash(id, "typedef", x);
   check_type_clash(id, "typedef", x);
   insert(id, new Typedef_type(x));
}

void SymbolTable::insert(StructDecl* x) {
   string id = x->id->name;
   check_function_clash(id, "struct", x);
   check_type_clash(id, "struct", x);
   insert(id, new Struct_type(x));
}

FuncParam::FuncParam(TypeSpec* t, std::string n) {
   type= Var_type::convertTypeSpec(t);
   name=n;
   ref=t->reference;
}

FuncHeader::FuncHeader(FuncDecl* x) {
   return_type = Var_type::convertTypeSpec(x->return_typespec);
   for (int i = 0; i < x->params.size(); i++) {
      auto param = x->params[i];
      FuncParam p(param->typespec, param->name);
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

bool SymbolTable::contains(string id) {
   for (int i = scope_stack.size() - 1; i >= 0; i++) {
      if (scope_stack[i].ident2variable.count(id)) {
         return true;
      }
   }
   return false;
}

Var_type* SymbolTable::getType(string id) {
   for (int i = scope_stack.size() - 1; i >= 0; i--) {
      if (scope_stack[i].ident2variable.count(id)) {
         return scope_stack[i].ident2variable[id]->type->copy();
      }
   }
   return NULL;   
}


void SymbolTable::insert(FuncDecl* x) {
   //search for name clash
   string id = x->id->name;
   if (scope_stack[0].ident2type.count(id)) {
      Var_type* t = scope_stack[0].ident2type.find(id)->second;
      add_warning(x, "there exists a "+t->class_str()+" with the same name as the function '"+id+"'");
   }
   if (scope_stack[0].ident2variable.count(id)) {
      add_warning(x, "there exists a global variable "+scope_stack[0].ident2type.find(id)->first+" with the same name as the function '"+id+"'");
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
   if (dbg) cout << "visit program" << endl;
   symTable = SymbolTable();
   for (AstNode* n : x->nodes) {
      n->accept(this);
   }
}


void TypeChecker::visit_include(Include* x) {}
void TypeChecker::visit_macro(Macro* x) {}
void TypeChecker::visit_using(Using* x) {}
void TypeChecker::visit_paramdecl(ParamDecl *x) {}

void TypeChecker::visit_ident(Ident *x) {
   if (dbg) cout << "visit ident" << endl;
   expr_type = symTable.getType(x->name);
   if (expr_type == NULL) {
      add_error(x, "undeclared variable '"+x->name+"'");
   }
}

void TypeChecker::visit_typespec(TypeSpec *x) {}
void TypeChecker::visit_jumpstmt(JumpStmt *x) {}


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
      add_warning(x, "there exists a function with the same name as the parameter '"+id+"'");
   }
   //note that the scope_stack has exactly 2 levels, the global one and the function one
   if (scope_stack[0].ident2variable.count(id)) {
      Variable* v = scope_stack[0].ident2variable.find(id)->second;
      add_warning(x, "the parameter '"+id+"' overshadows a global variable of type '"+v->type->to_string()+"'");
   }
   insert(id, new Variable(x->typespec, id));
}

void TypeChecker::visit_funcdecl(FuncDecl *x) {
   if (dbg) cout << "visit funcdecl" << endl;
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
   if (dbg) cout << "visit arraydecl" << endl;
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
   if (dbg) cout << "visit vardecl" << endl;
   //if (x->kind == Decl::Pointer_type) {
      //out() << "*";
   //}
   //out() << '"' << x->name << '"';
   /*
   if (x->init) {
      x->init->accept(this);
   }
   */
}

void TypeChecker::visit_objdecl(ObjDecl *x) {
   if (dbg) cout << "visit objdecl" << endl;
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
   if (dbg) cout << "visit block" << endl;
   if (x->stmts.empty()) return;
   symTable.scope_stack.push_back(Scope());
   for (Stmt *s : x->stmts) {
      s->accept(this);
   }
   symTable.scope_stack.pop_back();
}

void TypeChecker::check_bool_type(AstNode* x) {
   x->accept(this);
   if (expr_type->to_string() != "bool") {
      if (expr_type->to_string() == "int") {
         add_warning(x, "the if condition is implicitly converted from type int to bool");
      }
      else {
         add_error(x, "the if condition has type "+expr_type->to_string()+" instead of bool");
      }
   }
}

void TypeChecker::visit_ifstmt(IfStmt *x) {
   check_bool_type(x->cond);
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
   check_bool_type(x->cond);
   symTable.scope_stack.push_back(Scope());
   if (x->is_for()) {
      x->init->accept(this);
      x->post->accept(this);
   }
   x->substmt->accept(this);
   symTable.scope_stack.pop_back();
}

void TypeChecker::visit_exprstmt(ExprStmt* x) {
   if (dbg) cout << "visit exprstmt" << endl;
   if (x->expr) {
      x->expr->accept(this);
   }
}

void TypeChecker::visit_declstmt(DeclStmt* x) {
   if (dbg) cout << "visit declstmt" << endl;
   //x->typespec->accept(this);
   Var_type* declared = Var_type::convertTypeSpec(x->typespec);   
   for (DeclStmt::Item item : x->items) {
      string name = item.decl->name;
      if (item.init) {
         item.init->accept(this);
         if (expr_type == NULL) continue;
         if (not declared->equals(expr_type)) {
            if ((declared->is_int() or declared->is_bool() or declared->is_char()) and
               (expr_type->is_int() or expr_type->is_bool() or expr_type->is_char())) {
               add_warning(item.init, "Expression of type "+expr_type->to_string()+
                  " implicitly transformed to type "+declared->to_string());
            }
            else {
               add_error(item.init, "Expression of type "+expr_type->to_string()+
                  " assigned to variable "+name+" of type "+declared->to_string());
            }
         }
      }
      symTable.insert(name, new Variable(name, declared));
   }
}

//expressions //////////////////////////////////////////////

void TypeChecker::visit_literal(Literal *x) {
   if (dbg) cout << "visit literal" << endl;
   //simply set expr_type to the correct type
   expr_type = new Basic_type();
   switch (x->type) {
   case Literal::Int:
      dynamic_cast<Basic_type*>(expr_type)->name = "int";
      break;
   case Literal::Double:
      dynamic_cast<Basic_type*>(expr_type)->name = "double";
      break;
   case Literal::Bool:
      dynamic_cast<Basic_type*>(expr_type)->name = "bool";
      break;
   case Literal::String:
      dynamic_cast<Basic_type*>(expr_type)->name = "string";
      break;
   case Literal::Char:
      dynamic_cast<Basic_type*>(expr_type)->name = "char";
      break;
   default:
      cerr << "unexpected literal" << endl;
      break;
   }
}

void TypeChecker::visit_binaryexpr_assignment(Var_type* t1, Var_type* t2) {
   if (dbg) cout << "visit binaryexpr assignment" << endl;
   expr_type = t2->copy();
}

void TypeChecker::visit_binaryexpr(BinaryExpr *x) {
   if (dbg) cout << "visit binary expr" << endl;
   x->left->accept(this);
   Var_type* left = expr_type;
   x->right->accept(this);
   Var_type* right = expr_type;

   if (x->op == "<<") {
      //how do you handle cout?
   }
   if (x->op == ">>") {
      //how do you handle cin?
      // if (not right.assignable) {
      //    add_error(x, "you can only read into variables");
      // }
      // if (right.is_const) {
      //    add_error(x, "you cannot read into a constant variable '"+right.name+"'");
      // }
   }

   if (x->op == "=") {
       visit_binaryexpr_assignment(left, right);
       return;
   }
   // if (x->op == "+=" || x->op == "-=" || x->op == "*=" || x->op == "/=" ||
   //     x->op == "&=" || x->op == "|=" || x->op == "^=") {
   //    visit_binaryexpr_op_assignment(x->op[0], left, right);
   //    return;
   // } 
   // else if (x->op == "&" || x->op == "|" || x->op == "^") {
   //    bool ret = false;
   //    switch (x->op[0]) {
   //    case '&': ret = visit_bitop<_And>(left, right); break;
   //    case '|': ret = visit_bitop<_Or >(left, right); break;
   //    case '^': ret = visit_bitop<_Xor>(left, right); break;
   //    }
   //    if (ret) {
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' son incompatibles", x->op.c_str()));
   // }
   // else if (x->op == "+" || x->op == "*" || x->op == "-" || x->op == "/") {
   //    bool ret = false;
   //    switch (x->op[0]) {
   //    case '+': {
   //       if (left.is<String>() and right.is<String>()) {
   //          _curr = Value(left.as<String>() + right.as<String>());
   //          ret = true;
   //       } else {
   //          ret = visit_sumprod<_Add>(left, right);
   //       }
   //       break;
   //    }
   //    case '*': ret = visit_sumprod<_Mul>(left, right); break;
   //    case '-': ret = visit_sumprod<_Sub>(left, right); break;
   //    case '/': ret = visit_sumprod<_Div>(left, right); break;
   //    }
   //    if (ret) {
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' son incompatibles", x->op.c_str()));
   // }
   // else if (x->op == "%") {
   //    if (left.is<Int>() and right.is<Int>()) {
   //       _curr = Value(left.as<Int>() % right.as<Int>());
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' son incompatibles", "%"));
   // }
   // else if (x->op == "%=") {
   //    if (!left.is<Reference>()) {
   //       _error(_T("Para usar '%s' se debe poner una variable a la izquierda", x->op.c_str()));
   //    }
   //    left = Reference::deref(left);
   //    if (left.is<Int>() and right.is<Int>()) {
   //       left.as<Int>() %= right.as<Int>();
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' son incompatibles", "%="));
   // }
   // else if (x->op == "&&" or x->op == "and" || x->op == "||" || x->op == "or")  {
   //    if (left.is<Bool>() and right.is<Bool>()) {
   //       _curr = Value(x->op == "&&" or x->op == "and" 
   //                     ? left.as<Bool>() and right.as<Bool>()
   //                     : left.as<Bool>() or  right.as<Bool>());
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' no son de tipo 'bool'", x->op.c_str()));
   // }
   // else if (x->op == "==" || x->op == "!=") {
   //    if (left.same_type_as(right)) {
   //       _curr = Value(x->op == "==" ? left.equals(right) : !left.equals(right));
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' no son del mismo tipo", x->op.c_str()));
   // }
   // else if (x->op == "<" || x->op == ">" || x->op == "<=" || x->op == ">=") {
   //    bool ret = false;
   //    if (x->op[0] == '<') {
   //       ret = (x->op.size() == 1 
   //              ? visit_comparison<_Lt>(left, right)
   //              : visit_comparison<_Le>(left, right));
   //    } else {
   //       ret = (x->op.size() == 1 
   //              ? visit_comparison<_Gt>(left, right)
   //              : visit_comparison<_Ge>(left, right));
   //    }
   //    if (ret) {
   //       return;
   //    }
   //    _error(_T("Los operandos de '%s' no son compatibles", x->op.c_str()));
   // }
   // _error(_T("Interpreter::visit_binaryexpr: UNIMPLEMENTED (%s)", x->op.c_str()));
}

void TypeChecker::visit_exprlist(ExprList *x) {
   if (dbg) cout << "visit exprlist" << endl;
   for (int i = 0; i < x->exprs.size(); i++) {
      x->exprs[i]->accept(this);
   }
}

void TypeChecker::visit_callexpr(CallExpr *x) {
   if (dbg) cout << "visit callexpr" << endl;
   //to do.......
   //here, it should be checked that the type of the arguments is correct
   //and assign to expr_type the return type of the function
   x->func->accept(this);
   for (int i = 0; i < x->args.size(); i++) {
      x->args[i]->accept(this);
   }
}

void TypeChecker::visit_indexexpr(IndexExpr *x) {
   if (dbg) cout << "visit indexexpr" << endl;
   x->base->accept(this);
   Var_type* baseType = expr_type;
   //here it should be checked that baseType has type vector or array
   x->index->accept(this);
   if (not expr_type->is_int()) {
      add_error(x, "the index of an array or vector should be an integer, but it has type "+expr_type->to_string());
   }
   //here, the type of the elements of the vector/array baseType should be assigned to expr_type
}

void TypeChecker::visit_fieldexpr(FieldExpr *x) {
   if (dbg) cout << "visit fieldexpr" << endl;
   //to do...
   //imagine that we have s.f, where s is an instance of an struct S
   //and f a field of S
   //first, we should find the definition of S in the scope_stack,
   //then, check if it has a field named f
   //finally, return the type of that field
   if (x->pointer) {
      //i don't know the meaning of this boolean
   }
   x->base->accept(this);
   x->field->accept(this);
}

void TypeChecker::visit_condexpr(CondExpr *x) {
   if (dbg) cout << "visit condexpr" << endl;
   x->cond->accept(this);
   if (not expr_type->is_bool()) {
      add_error(x, "the condition of the ternary operator '?:' should have type bool, but it has type "+expr_type->to_string());
   }
   x->then->accept(this);
   Var_type* thenType = expr_type;
   x->els->accept(this);
   Var_type* elsType = expr_type;
   if (not thenType->equals(elsType)) {
      add_error(x, "the two possible values of the ternary operator '?:' should have the same type, but they have types "+thenType->to_string()+" and "+elsType->to_string());
   }
   expr_type = thenType->copy();
}

void TypeChecker::visit_signexpr(SignExpr *x) {
   if (dbg) cout << "visit signexpr" << endl;
   x->expr->accept(this);
   if (not expr_type->is_int() or expr_type->is_float() or expr_type->is_double()) {
      add_error(x, "applying a sign operator to an expression of type "+expr_type->to_string()+", but it should have type int, float or double");
   }
   //expr_type doesn't change
}

void TypeChecker::visit_increxpr(IncrExpr *x) {
   if (dbg) cout << "visit increxpr" << endl;
   x->expr->accept(this);
   if (not expr_type->is_int() and not expr_type->is_char()) {
      add_error(x, "applying an increment operator to an expression of type "+expr_type->to_string()+", but it should have type int or char");
   }
   //expr_type doesn't change
}

void TypeChecker::visit_negexpr(NegExpr *x) {
   if (dbg) cout << "visit negexpr" << endl;
   x->expr->accept(this);
   if (not expr_type->is_bool()) {
      add_error(x, "applying negation operator to an expression of type "+expr_type->to_string()+", but it should have type bool");
   }
   expr_type = new Basic_type("bool");
}

void TypeChecker::visit_addrexpr(AddrExpr *x) {
   if (dbg) cout << "visit addrexpr" << endl;
   x->expr->accept(this);
   expr_type = new Pointer_type(expr_type);
}

void TypeChecker::visit_derefexpr(DerefExpr *x) {
   if (dbg) cout << "visit derefexpr" << endl;
   x->expr->accept(this);
   //here, it should be checked that expr_type
   //is an address, and assign expr_type to ???
}


// i dont know what is this  //////////////////////////////////////////////

void TypeChecker::visit_errorstmt(Stmt::Error *x) {}

void TypeChecker::visit_errorexpr(Expr::Error *x) {}



