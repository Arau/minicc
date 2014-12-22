#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include <assert.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include "ast.hh"

/* Missing support for array and object types */

struct Type {
   virtual Type();
   virtual Type(TypeSpec* x);
   virtual Type(Type t);
   virtual std::string to_string() = 0;
   virtual std::string class_str() = 0;   
};

struct Basic : Type {
   char basic_types[][80] = {"int", "float", "char", "double", "string", "bool", ""};
   std::string name;
};

struct Struct : Type {
   struct Struct_field {
      Type* type;
      std::string name;
      Struct_field() {};
      Struct_field(Struct_field f);
   };
   std::vector<Struct_field> fields;
};

struct Enum : Type {
   struct Enum_field {
      std::string name;
      int value;
      Enum_field() {};
      Enum_field(Enum_field f) {
         name = f.name;
         value = f.value;
      }
   };
   std::vector<Enum_field> fields;
};

struct Vector : Type {
   Type* content;
};

struct Typedef : Type {
   std::string name;
   Type* real;
};

struct Pointer : Type {
   Type* pointed;
};

struct Variable {
   std::string name;
   Type type;
   //attributes for more advanced analysis, not used for now
   bool initialized;
   bool known_value; 
   bool reference;
   bool is_const;
   bool assignable;
};

struct FuncParam {
   std::string name;
   Type* type;
   bool ref;
};

struct FuncHeader {
   Type* return_type;
   std::vector<FuncParam> params;
   bool equals(FuncHeader* other);
   FuncHeader(FuncDecl* x);
};

struct Scope {
   std::unordered_map<std::string,Value*> ident2variable;
   std::map<std::string,Type*> ident2type; //for typedefs, structs, enums and so on
};

struct SymbolTable {
   SymbolTable();
   void insert(EnumDecl* x);
   void insert(TypedefDecl* x);
   void insert(StructDecl* x);
   void insert(FuncDecl* x);
   void insert(ParamDecl* x);
   bool contains(FuncDecl* x);
   std::vector<Scope> scope_stack;
   std::unordered_map<std::string,std::vector<FuncHeader*> > ident2func;
};

class TypeChecker : public AstVisitor, public ReadWriter {
private:
   SymbolTable symTable;

   Type expr_value; //updated by expressions


public:
   TypeChecker(std::ostream *o = &std::cout)
      : ReadWriter(o) {}

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
   void visit_binaryexpr(BinaryExpr *x);
   void visit_literal(Literal *x);
   void visit_ident(Ident *x);
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
   void visit_paramdecl(ParamDecl *x);

   void visit_errorstmt(Stmt::Error *x);
   void visit_errorexpr(Expr::Error *x);


};


#endif
