#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include <assert.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include "ast.hh"

/* Missing support for array and object types

*/
char basic_types[][80] = {"int", "float", "char", "double", "string", "bool", ""};

struct Type;
struct Struct_field {
   Type* type;
   std::string name;
   Struct_field() {};
   Struct_field(Struct_field f);
};

struct Enum_field {
   std::string name;
   int value;
   Enum_field() {};
   Enum_field(Enum_field f) {
      name = f.name;
      value = f.value;
   }
};

enum type_class { basic, strct, enm, vec, alias, pointer};
struct Type {
   type_class T;
   //basic types:
   std::string basic_name;
   //struct types:
   std::vector<Struct_field> struct_fields;
   //enum types:
   std::vector<Enum_field> enum_fields;
   //vector types:
   Type* content;
   //alias types
   std::string alias_name;
   Type* real;
   //pointer types:
   Type* pointed;
   Type();
   Type(TypeSpec* x);
   Type(Type t);
   std::string to_string();
   std::string class_str();
};

struct Value {
   std::string name;
   Type type;
   bool initialized;
   bool known_value; //for static evaluation, not used for now
   bool reference;
   bool is_const;
   bool assignable;

   //basic types:
   int int_value;
   bool bool_value;
   char char_value;
   std::string string_value;
   float float_value;
   double double_value;
   //struct types
   std::vector<Value> struct_field_values;
   //enum types
   std::string item;
   //vector types
   std::vector<Value> elements;
};

struct FuncParam {
   std::string name;
   Type* type;
   bool ref;
   Value* init;
};

struct FuncHeader {
   Type* return_type;
   std::vector<FuncParam> params;
   bool equals(FuncHeader* other);
   FuncHeader(FuncDecl* x);
};

struct Scope {
   std::unordered_map<std::string,Value*> ident2value;
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
