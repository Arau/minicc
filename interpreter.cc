#include "ast.hh"
#include "translator.hh"
#include "interpreter.hh"
using namespace std;

void Interpreter::_init() {}

void Interpreter::setenv(string id, Value v, bool hidden) {
   _env.back().set(id, v, hidden);
}

bool Interpreter::getenv(string id, Value& v) {
   for (int i = _env.size()-1; i >= 0; i--) {
      if (_env[i].get(id, v)) {
         return true;
      }
   }
   return false;
}

void Interpreter::actenv() {
   for (int i = 0; i < _env.size(); i++) {
      _env[i].active = false;
   }
   _env.back().active = true;
}

void Interpreter::popenv() { 
   _env.pop_back(); 
   _env.back().active = true;
}


string Interpreter::env2json() const {
   ostringstream json;
   json << "[";
   for (int i = 1; i < _env.size(); i++) {
      if (i > 1) {
         json << ",";
      }
      json << "{\"name\":\"" << _env[i].name << "\",\"tab\":";
      json << _env[i].to_json();
      json << "}";
   }
   json << "]";
   return json.str();
}


void Interpreter::invoke_func_prepare(FuncDecl *fn, const vector<Value>& args) {
   if (fn->params.size() != args.size()) {
      _error(_T("Error en el número de argumentos al llamar a '%s'", 
                fn->funcname().c_str()));
   }
   for (int i = 0; i < args.size(); i++) {
      if (args[i].is<Reference>()) {
         Value v = args[i];
         if (!fn->params[i]->typespec->reference) {
            v = Reference::deref(v);
         }
         setenv(fn->params[i]->name, v);
      } else {
         if (fn->params[i]->typespec->reference) {
            _error(_T("En el parámetro %d se requiere una variable.", i+1));
         }
         setenv(fn->params[i]->name, args[i]);
      }
   }
}

Value _max(const vector<Value>& args) {
   assert(args.size() == 2);
   assert(args[0].is<Int>());
   assert(args[1].is<Int>());
   return Value(std::max(args[0].as<Int>(), args[1].as<Int>()));
}

void Interpreter::prepare_global_environment() {
   _env.clear();
   _env.push_back(Environment("<global>"));

   bool hidden = true;
   setenv("endl", Endl, hidden);
   setenv("cout", Cout, hidden);
   setenv("cin",  Cin,  hidden);

   Function *max_func_type = new Function(Int::self);
   max_func_type->add_params(Int::self, Int::self);
   setenv("max",  max_func_type->mkvalue("max", new BuiltinFunc(_max)));
}

void Interpreter::visit_program_prepare(Program *x) {
   prepare_global_environment();
   for (AstNode *n : x->nodes) {
      n->accept(this);
   }
}

void Interpreter::visit_program_find_main() {
   if (!getenv("main", _curr)) {
      _error(_T("The '%s' function does not exist.", "main"));
   }
   if (!_curr.is<Function>()) {
      _error(_T("'main' is not a function."));
   }
}

void Interpreter::visit_program(Program* x) {
   visit_program_prepare(x);
   visit_program_find_main();
   _curr.as<Function>().invoke(this, vector<Value>());
}

void Interpreter::visit_comment(CommentSeq* cn) {}
void Interpreter::visit_macro(Macro* x) {}

void Interpreter::visit_using(Using* x) {
   // TODO: Augment current environment with  
   // an environment for the specified namespace
}

void Interpreter::visit_include(Include* x) {
   // TODO: Depending on include, register 'fake' functions & types.
   // (on the 'std' environment)
}

void Interpreter::visit_funcdecl(FuncDecl *x) {
   string funcname = x->funcname();
   Type *return_type = Type::get(x->return_typespec);  // return_type == 0 means 'void'
   Function *functype = new Function(return_type);
   for (auto p : x->params) {
      Type *param_type = Type::get(p->typespec);
      assert(param_type != 0);
      functype->add_param(param_type);
   }
   setenv(x->funcname(), 
          functype->mkvalue(funcname,
                            new UserFunc(x)));
}

void Interpreter::visit_structdecl(StructDecl *x) {
   // Create a new Struct type now
   Struct *type = new Struct(x->struct_name());
   for (int i = 0; i < x->decls.size(); i++) {
      DeclStmt& decl = *x->decls[i];
      Type *field_type = Type::get(decl.typespec);
      assert(type != 0);
      for (DeclStmt::Item& item : decl.items) {
         if (item.decl->is<ArrayDecl>()) {
            Expr *size_expr = dynamic_cast<ArrayDecl*>(item.decl)->size;
            Literal *size_lit = dynamic_cast<Literal*>(size_expr);
            assert(size_lit != 0);
            assert(size_lit->type == Literal::Int);
            const int sz = size_lit->val.as_int;
            // TODO: don't create new Array type every time?
            type->add_field(item.decl->name, new Array(field_type, sz)); 
         } else {
            type->add_field(item.decl->name, field_type);
         }
      }
   }
   Type::register_type(x->struct_name(), type);
}

void Interpreter::visit_ident(Ident *x) {
   Value v;
   if (!getenv(x->name, v)) {
      _error(_T("La variable '%s' no existe.", x->name.c_str()));
   }
   _curr = (v.is<Reference>() ? v : Reference::mkref(v));
}

void Interpreter::visit_literal(Literal *x) {
   switch (x->type) {
   case Literal::String: _curr = Value(*x->val.as_string.s); break;
   case Literal::Int:    _curr = Value(x->val.as_int);       break;
   case Literal::Double: _curr = Value(x->val.as_double);    break;
   case Literal::Bool:   _curr = Value(x->val.as_bool);      break;
   case Literal::Char:   _curr = Value((*x->val.as_string.s)[0] /* FIXME */);
      break;
   default:
      _error(_T("Interpreter::visit_literal: UNIMPLEMENTED"));
   }
}

struct _Add { template<typename T> static T eval(const T& a, const T& b) { return a + b; } };
struct _Sub { template<typename T> static T eval(const T& a, const T& b) { return a - b; } };
struct _Mul { template<typename T> static T eval(const T& a, const T& b) { return a * b; } };
struct _Div { template<typename T> static T eval(const T& a, const T& b) { return a / b; } };

struct _And { template<typename T> static T eval(const T& a, const T& b) { return a & b; } };
struct _Or  { template<typename T> static T eval(const T& a, const T& b) { return a | b; } };
struct _Xor { template<typename T> static T eval(const T& a, const T& b) { return a ^ b; } };

struct _AAdd { template<typename T> static void eval(T& a, const T& b) { a += b; } };
struct _ASub { template<typename T> static void eval(T& a, const T& b) { a -= b; } };
struct _AMul { template<typename T> static void eval(T& a, const T& b) { a *= b; } };
struct _ADiv { template<typename T> static void eval(T& a, const T& b) { a /= b; } };
struct _AAnd { template<typename T> static void eval(T& a, const T& b) { a &= b; } };
struct _AOr  { template<typename T> static void eval(T& a, const T& b) { a |= b; } };
struct _AXor { template<typename T> static void eval(T& a, const T& b) { a ^= b; } };

struct _Lt { template<typename T> static bool eval(const T& a, const T& b) { return a <  b; } };
struct _Le { template<typename T> static bool eval(const T& a, const T& b) { return a <= b; } };
struct _Gt { template<typename T> static bool eval(const T& a, const T& b) { return a >  b; } };
struct _Ge { template<typename T> static bool eval(const T& a, const T& b) { return a >= b; } };


template<class Op>
bool Interpreter::visit_op_assignment(Value left, Value _right) {
   Value right = left.type()->convert(_right);
   if (left.is<Int>() and right.is<Int>()) {
      Op::eval(left.as<Int>(), right.as<Int>());
      return true;
   } 
   if (left.is<Float>() and right.is<Float>()) {
      Op::eval(left.as<Float>(), right.as<Float>());
      return true;
   }
   if (left.is<Double>() and right.is<Double>()) {
      Op::eval(left.as<Double>(), right.as<Double>());
      return true;
   }
   return false;
}

template<class Op>
bool Interpreter::visit_bitop_assignment(Value left, Value _right) {
   Value right = left.type()->convert(_right);
   if (left.is<Int>() and right.is<Int>()) {
      Op::eval(left.as<Int>(), right.as<Int>());
      return true;
   } 
   return false;
}

template<class Op>
bool Interpreter::visit_sumprod(Value left, Value _right) {
   Value right = left.type()->convert(_right);
   if (left.is<Int>()) {
      _curr = Value(Op::eval(left.as<Int>(), right.as<Int>()));
      return true;
   }
   if (left.is<Float>()) {
      _curr = Value(Op::eval(left.as<Float>(), right.as<Float>()));
      return true;
   }
   if (left.is<Double>()) {
      _curr = Value(Op::eval(left.as<Double>(), right.as<Double>()));
      return true;
   }
   return false;
}

template<class Op>
bool Interpreter::visit_bitop(Value left, Value right) {
   if (left.is<Int>() and right.is<Int>()) {
      _curr = Value(Op::eval(left.as<Int>(), right.as<Int>()));
      return true;
   }
   return false;
}

template<class Op>
bool Interpreter::visit_comparison(Value left, Value right) {
   if (left.is<Int>() and right.is<Int>()) {
      _curr = Value(Op::eval(left.as<Int>(), right.as<Int>()));
      return true;
   }
   if (left.is<Float>() and right.is<Float>()) {
      _curr = Value(Op::eval(left.as<Float>(), right.as<Float>()));
      return true;
   }
   if (left.is<Double>() and right.is<Double>()) {
      _curr = Value(Op::eval(left.as<Double>(), right.as<Double>()));
      return true;
   }
   if (left.is<String>() and right.is<String>()) {
      _curr = Value(Op::eval(left.as<String>(), right.as<String>()));
      return true;
   }
   return false;
}

void Interpreter::visit_binaryexpr(BinaryExpr *x) {
   x->left->accept(this);
   Value left = _curr;
   Value leftderef = Reference::deref(left);
   if (x->kind != Expr::Assignment) {
      left = Reference::deref(left);
   }

   // cout << ...
   if (leftderef == Cout && x->op == "<<") {
      Value old = _curr;
      x->right->accept(this);
      out() << Reference::deref(_curr);
      _curr = old;
      return;
   }

   // cin >> ...
   if (leftderef == Cin && x->op == ">>") {
      Value old = _curr;
      Ident *id = dynamic_cast<Ident*>(x->right);
      if (id == 0) {
         _error(_T("La lectura con 'cin' requiere que pongas variables"));
      }
      Value right;
      if (!getenv(id->name, right)) {
         _error(_T("La variable '%s' no está declarada", id->name.c_str()));
      }
      assert(leftderef.as<Istream>() == cin);
      right = Reference::deref(right);
      in() >> right;
      _curr = old;
      return;
   }

   x->right->accept(this);
   Value right = _curr;
   right = Reference::deref(right);
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

inline bool assignment_types_ok(const Value& a, const Value& b) {
   return 
      (a.same_type_as(b)) or
      (a.is<Float>() and b.is<Double>()) or
      (a.is<Double>() and b.is<Float>());
}

void Interpreter::visit_binaryexpr_assignment(Value left, Value right) {
   if (!left.is<Reference>()) {
      _error(_T("Intentas asignar sobre algo que no es una variable"));
   }
   left = Reference::deref(left);
   right = left.type()->convert(right);
   if (right == Value::null) {
      _error(_T("La asignación no se puede hacer porque los "
                "tipos no son compatibles (%s) vs (%s)", 
                left.type_name().c_str(), 
                right.type_name().c_str()));
   }
   left.assign(right);
   _curr = left;
}

void Interpreter::visit_binaryexpr_op_assignment(char op, Value left, Value right) {
   if (!left.is<Reference>()) {
      _error(_T("Para usar '%s=' se debe poner una variable a la izquierda", op));
   }
   left = Reference::deref(left);
   bool ok = false;
   switch (op) {
   case '+': {
      if (left.is<String>() and right.is<String>()) {
         left.as<String>() += right.as<String>();
         ok = true;
      } else {
         ok = visit_op_assignment<_AAdd>(left, right);
      }
      break;
   }
   case '-': ok = visit_op_assignment<_ASub>(left, right); break;
   case '*': ok = visit_op_assignment<_AMul>(left, right); break;
   case '/': ok = visit_op_assignment<_ADiv>(left, right); break;
   case '&': ok = visit_bitop_assignment<_AAnd>(left, right); break;
   case '|': ok = visit_bitop_assignment<_AOr >(left, right); break;
   case '^': ok = visit_bitop_assignment<_AXor>(left, right); break;
   }
   if (!ok) {
      string _op = "?=";
      _op[0] = op;
      _error(_T("Los operandos de '%s' no son compatibles", _op.c_str()));
   }
}

void Interpreter::visit_block(Block *x) {
   for (Stmt *stmt : x->stmts) {
      stmt->accept(this);
   }
}

void Interpreter::visit_vardecl(VarDecl *x) {
   string type_name = x->typespec->typestr();
   Type *type = Type::get(x->typespec);
   if (type == 0) {
      _error(_T("El tipo '%s' no existe.", type_name.c_str()));
   }
   _curr = Reference::deref(_curr);
   try {
      setenv(x->name, (_curr.is_null() ? type->create() : type->convert(_curr)));
   } catch (TypeError& e) {
      _error(e.msg);
   }
}

void Interpreter::visit_arraydecl(ArrayDecl *x) {
   Value init = _curr;
   x->size->accept(this);
   if (!_curr.is<Int>()) {
      _error(_T("El tamaño de una tabla debe ser un entero"));
   }
   if (_curr.as<Int>() <= 0) {
      _error(_T("El tamaño de una tabla debe ser un entero positivo"));
   }
   const int sz = _curr.as<Int>();
   Type *celltype = Type::get(x->typespec);
   if (celltype == 0) {
      _error(_T("El tipo '%s' no existe", x->typespec->typestr().c_str()));
   }
   // TODO: don't create new Array type every time?
   Type *arraytype = new Array(celltype, sz);
   setenv(x->name, (init.is_null() 
                    ? arraytype->create()
                    : arraytype->convert(init)));
}

void Interpreter::visit_objdecl(ObjDecl *x) {
   Type *type = Type::get(x->typespec);
   if (type != 0) {
      // TODO: Convertir la construcción en una llamada al método constructor
      vector<Value> args;
      for (int i = 0; i < x->args.size(); i++) {
         x->args[i]->accept(this);
         args.push_back(_curr);
      }
      setenv(x->name, type->construct(args));
      return;
   }
   _error(_T("The type '%s' is not implemented in MiniCC", 
             x->typespec->typestr().c_str()));
}

void Interpreter::visit_declstmt(DeclStmt* x) {
   for (DeclStmt::Item& item : x->items) {
      if (item.init) {
         item.init->accept(this);
      } else {
         _curr = Value::null;
      }
      item.decl->accept(this);
   }
}

void Interpreter::visit_exprstmt(ExprStmt* x) {
   x->expr->accept(this);
   if (x->is_return) {
      _ret = _curr;
   }
}

void Interpreter::visit_ifstmt(IfStmt *x) {
   x->cond->accept(this);
   if (!_curr.is<Bool>()) {
      _error(_T("An if's condition needs to be a bool value"));
   }
   if (_curr.as<Bool>()) {
      x->then->accept(this);
   } else {
      if (x->els != 0) {
         x->els->accept(this);
      }
   }
}

void Interpreter::visit_iterstmt(IterStmt *x) {
   pushenv("");
   if (x->init) {
      x->init->accept(this);
   }
   while (true) {
      x->cond->accept(this);
      if (!_curr.is<Bool>()) {
         _error(_T("La condición de un '%s' debe ser un valor de tipo bool.",
                   (x->is_for() ? "for" : "while")));
      }
      if (!_curr.as<Bool>()) {
         break;
      }
      x->substmt->accept(this);
      if (x->post) {
         x->post->accept(this);
      }
   }
   popenv();
}

void Interpreter::invoke_user_func(FuncDecl *decl, const vector<Value>& args) {
   pushenv(decl->funcname());
   invoke_func_prepare(decl, args);
   decl->block->accept(this);
   popenv();
}

void Interpreter::visit_callexpr_getfunc(CallExpr *x) {
   x->func->accept(this);
   _curr = Reference::deref(_curr);
   if (!_curr.is<Function>()) {
      _error(_T("Calling something other than a function."));
   }
}
void Interpreter::visit_callexpr(CallExpr *x) {
   visit_callexpr_getfunc(x);
   Value func = _curr;

   // Eval arguments
   vector<Value> args;
   for (int i = 0; i < x->args.size(); i++) {
      x->args[i]->accept(this);
      args.push_back(_curr);
   }

   // Check types
   const Function *func_type = func.type()->as<Function>();
   for (int i = 0; i < args.size(); i++) {
      string t1 = func_type->param(i)->typestr();
      Value arg_i = args[i];
      if (!func_type->param(i)->is<Reference>()) {
         arg_i = Reference::deref(arg_i);
      } else if (!arg_i.type()->is<Reference>()) {
         _error(_T("En el parámetro %d se requiere una variable.", i+1));
      }
      string t2 = arg_i.type()->typestr();
      if (t1 != t2) {
         _error(_T("El argumento %d no es compatible con el tipo del parámetro "
                   "(%s vs %s)", i+1, t1.c_str(), t2.c_str()));
      }
   }
   
   // Invoke
   func.as<Function>().invoke(this, args);
   if (_ret == Value::null && !func.type()->as<Function>()->is_void()) {
      Type *return_type = func.type()->as<Function>()->return_type();
      _error(_T("La función '%s' debería devolver un '%s'", 
                func.as<Function>().name.c_str(),
                return_type->typestr().c_str()));
   }
   _curr = _ret;
}

void Interpreter::visit_indexexpr(IndexExpr *x) {
   x->base->accept(this);
   _curr = Reference::deref(_curr);
   if (!_curr.is<Array>() and !_curr.is<Vector>()) {
      _error(_T("Las expresiones de índice deben usarse sobre tablas o vectores"));
   }
   vector<Value>& vals = (_curr.is<Array>() ? _curr.as<Array>() : _curr.as<Vector>());
   x->index->accept(this);
   _curr = Reference::deref(_curr);
   if (!_curr.is<Int>()) {
      // FIXME: maps!
      _error(_T("El índice en un acceso a tabla debe ser un entero"));
   }
   const int i = _curr.as<Int>();
   if (i < 0 || i >= vals.size()) {
      _error(_T("La casilla %d no existe", i));
   }
   _curr = Reference::mkref(vals[i]);
}

void Interpreter::visit_fieldexpr(FieldExpr *x) {
   x->base->accept(this);
   _curr = Reference::deref(_curr);
   if (_curr.is<Struct>()) {
      SimpleTable<Value>& fields = _curr.as<Struct>();
      Value v;
      if (!fields.get(x->field->name, v)) {
         _error(_T("No existe el campo '%s'", x->field->name.c_str()));
      }
      _curr = Reference::mkref(v);
      return;
   }
   pair<Type *, Type::Method> method;
   if (_curr.type()->get_method(x->field->name, method)) {
      Function *ft = dynamic_cast<Function*>(method.first);
      _curr = ft->mkvalue(x->field->name, new BoundMethod(method.second, _curr.data()));
      return;
   }
   _error(_T("Este objeto no tiene un campo '%s'", x->field->name.c_str()));
}

void Interpreter::visit_condexpr(CondExpr *x) {
   x->cond->accept(this);
   if (!_curr.is<Bool>()) {
      _error(_T("Una expresión condicional debe tener valor "
                "de tipo 'bool' antes del interrogante"));
   }
   if (_curr.as<Bool>()) {
      x->then->accept(this);
   } else {
      if (x->els != 0) {
         x->els->accept(this);
      }
   }
}

void Interpreter::visit_exprlist(ExprList *x) {
   Value v = VectorValue::make();
   vector<Value>& vals = v.as<VectorValue>();
   for (Expr *e : x->exprs) {
      e->accept(this);
      vals.push_back(_curr);
   }
   _curr = v;
}

void Interpreter::visit_signexpr(SignExpr *x) {
   x->expr->accept(this);
   if (x->kind == SignExpr::Positive) {
      return;
   }
   _curr = Reference::deref(_curr);
   if (_curr.is<Int>()) {
      _curr.as<Int>() = -_curr.as<Int>();
   } else if (_curr.is<Float>()) {
      _curr.as<Float>() = -_curr.as<Float>();
   } else if (_curr.is<Double>()) {
      _curr.as<Double>() = -_curr.as<Double>();
   } else {
      _error(_T("El cambio de signo para '%s' no tiene sentido",
                _curr.type_name().c_str()));
   }
}

void Interpreter::visit_increxpr(IncrExpr *x) {
   x->expr->accept(this);
   if (!_curr.is<Reference>()) {
      _error(_T("Hay que incrementar una variable, no un valor"));
   }
   Value after  = Reference::deref(_curr);
   Value before = after.clone();
   if (after.is<Int>()) {
      if (x->kind == IncrExpr::Positive) {
         after.as<Int>()++;
      } else {
         after.as<Int>()--;
      }
   } else {
      _error(_T("Estás incrementando un valor de tipo '%s'", 
                after.type_name().c_str()));
   }
   _curr = (x->preincr ? before : after);
}

void Interpreter::visit_negexpr(NegExpr *x) {
   x->expr->accept(this);
   if (!_curr.is<Bool>()) {
      _error(_T("Para negar una expresión ésta debe ser de tipo 'bool'"));
   }
   _curr.as<Bool>() = !_curr.as<Bool>();
}
