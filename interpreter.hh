#ifndef INTERPRETER_HH
#define INTERPRETER_HH

#include <assert.h>
#include <iostream>
#include <vector>
#include <map>

#include "ast.hh"
#include "value.hh"
#include "types.hh"

struct EvalError {
   std::string msg;
   EvalError(std::string _msg) : msg(_msg) {}
};

class Interpreter : public AstVisitor, public ReadWriter {
    Value _curr, _ret;

            std::vector<Environment> _env;
    std::map<std::string, FuncDecl*> _funcs;

   bool  is_struct(std::string name) {
      Type *t = Type::find(name);
      return t != 0 and t->is<Struct>();
   }

   void  pushenv(std::string name) { _env.push_back(Environment(name));  }
   void  popenv();
   void  actenv();
   void  setenv(std::string id, Value v, bool hidden = false);
   bool  getenv(std::string id, Value& v);

std::string env2json() const;

    void _error(std::string msg) {
       throw new EvalError(msg);
    }

     Value new_value_from_structdecl(StructDecl *x);

     void  invoke_func_prepare(FuncDecl *x, const std::vector<Value>& args);
     void  invoke_func(const std::vector<Value>&);

     void  visit_program_prepare(Program *x);
     void  visit_program_find_main();
     void  visit_binaryexpr_assignment(Value left, Value right);
     void  visit_binaryexpr_op_assignment(char, Value left, Value right);
     void  visit_callexpr_getfunc(CallExpr *x);

   template<class Op>
     bool  visit_op_assignment(Value left, Value right);

   template<class Op>
     bool  visit_bitop_assignment(Value left, Value right);

   template<class Op>
     bool  visit_sumprod(Value left, Value right);

   template<class Op>
     bool  visit_bitop(Value left, Value right);

   template<class Op>
     bool  visit_comparison(Value left, Value right);

    friend class Stepper;

   void _init();

public:
   Interpreter() { _init(); }
   Interpreter(std::istream *i, std::ostream *o)
      : ReadWriter(i, o) { _init(); }

   ~Interpreter() {}

   void visit_comment(CommentSeq *x);
   void visit_include(Include *x);
   void visit_macro(Macro *x);
   void visit_program(Program *x);
   void visit_using(Using *x);
   void visit_funcdecl(FuncDecl *x);
   void visit_structdecl(StructDecl *x);
   void visit_block(Block *x);
   void visit_ident(Ident *x);
   void visit_binaryexpr(BinaryExpr *x); 
   void visit_vardecl(VarDecl *);
   void visit_arraydecl(ArrayDecl *);
   void visit_declstmt(DeclStmt *x);
   void visit_exprstmt(ExprStmt *x);
   void visit_ifstmt(IfStmt *x);
   void visit_iterstmt(IterStmt *x);
   void visit_callexpr(CallExpr *x);
   void visit_indexexpr(IndexExpr *x);
   void visit_fieldexpr(FieldExpr *x);
   void visit_condexpr(CondExpr *x);
   void visit_exprlist(ExprList *x);
   void visit_signexpr(SignExpr *x);
   void visit_increxpr(IncrExpr *x);
   void visit_negexpr(NegExpr *x);
   void visit_literal(Literal *x);

   void visit_objdecl(ObjDecl *x);
   void visit_objdecl_vector(ObjDecl *x);
};

#endif
