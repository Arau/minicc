#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include <assert.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include "ast.hh"

char basic_types[][80] = {"int", "float", "char", "double", "string", "bool", ""};
enum type_class { basic, strct, enm, vec, alias, pointer };

struct Type;

struct Struct_field {
   Type* type;
   std::string name;
};

struct Enum_field {
   std::string name;
   int value;
};

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

   std::string to_string();
   Type(int n) {
      T = basic;
   }
};

struct Value {
   Type* type;
   bool initialized;
   bool reference;
   bool is_const;

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
   int length;
   std::vector<Value> elements;
};


struct LocalScope {
   std::unordered_map<std::string,Value*> ident2type;

   LocalScope() {
      ident2type = std::unordered_map<std::string,Value*>();
   }

   bool containsSymbol(std::string name, std::string& type); //version that also returns the type by ref.
   bool containsSymbol(std::string name);

   //returns whether the symbol was added correctly
   //(i.e. it did not already exist)
   bool addSymbol(std::string name, std::string type);
};

struct FunctionScope {
   std::vector<LocalScope> scopeStack;

   FunctionScope() {
      scopeStack = std::vector<LocalScope> ();
   }

   void pushLocalScope();
   void popLocalScope();
   
   //returns whether the symbol was added correctly
   //(i.e. it did not already exist)
   bool addSymbol(std::string name, std::string type);

   bool containsSymbol(std::string name, std::string& type);
   bool containsSymbol(std::string name);
};

struct SymbolTable {
   FunctionScope* currentFunction;
   
   //empties the scope stack of currentFunction,
   //gives it an empty first stack and puts the
   //function parameters in it
   void initCurrFuncScope(FuncDecl *x); 

};

class TypeChecker : public AstVisitor, public ReadWriter {
private:
   
   std::string get_pos(AstNode*);
   bool is_boolean_expr(AstNode*);
   std::string _curr;
   
   SymbolTable symTable;

   Type* typespec_type; //updated by visit_typespec


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
