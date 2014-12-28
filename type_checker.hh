#ifndef TYPE_CHECKER_HH
#define TYPE_CHECKER_HH

#include <assert.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include "ast.hh"

/* Missing subclasses: map, array, class, ... */
struct Var_type {
   Var_type() {};
   virtual std::string to_string() const = 0;
   virtual std::string class_str() const = 0;
   virtual Var_type* copy() const = 0;
   static Var_type* convertTypeSpec(const TypeSpec* t);
   bool equals(Var_type* other) const;
   bool is_int() const;
   bool is_char() const;
   bool is_bool() const;
   bool is_float() const;
   bool is_double() const;
};

//basic_types: "int", "float", "char", "double", "string", "bool"
struct Basic_type : Var_type {
   Basic_type() {}
   Basic_type(std::string name): name(name) {}
   Basic_type(const Basic_type *t);
   std::string name;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Struct_field {
   Var_type* type;
   std::string name;
   Struct_field() {};
   Struct_field(TypeSpec* t, std::string n);
   Struct_field(const Struct_field& f);
};

struct Struct_type : Var_type {
   Struct_type(const Struct_type *t);
   Struct_type(StructDecl *x);
   std::vector<Struct_field> fields;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Enum_field {
   std::string name;
   Enum_field() {};
   Enum_field(std::string name):name(name) {};
   Enum_field(const Enum_field &f): name(f.name) {};
};

struct Enum_type : Var_type {
   Enum_type(const Enum_type *t);
   Enum_type(EnumDecl *);
   std::vector<Enum_field> fields;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Vector_type : Var_type {
   Vector_type(const Vector_type *t);
   Var_type* content;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Typedef_type : Var_type {
   Typedef_type(const Typedef_type *t);
   Typedef_type(TypedefDecl *x);
   std::string name;
   Var_type* real;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Pointer_type : Var_type {
   Pointer_type(const Pointer_type *t);
   Pointer_type(const Var_type *pointed);
   Var_type* pointed;
   std::string to_string() const;
   std::string class_str() const;
   Var_type* copy() const;
};

struct Variable {
   Variable() {};
   Variable(std::string n, Var_type* t);
   Variable(TypeSpec* t, std::string n);

   std::string name;
   Var_type* type;
   //attributes for more advanced analysis, not used for now
   bool initialized;
   bool known_value; 
   bool reference;
   bool is_const;
   bool assignable;
};

struct FuncParam {
   FuncParam(TypeSpec* t, std::string n);
   std::string name;
   Var_type* type;
   bool ref;
};

struct FuncHeader {
   Var_type* return_type;
   std::vector<FuncParam> params;
   bool equals(FuncHeader* other);
   FuncHeader(FuncDecl* x);
};

struct Scope {
   std::unordered_map<std::string,Variable*> ident2variable;
   std::map<std::string,Var_type*> ident2type; //for typedefs, structs, enums and so on
};

struct SymbolTable {
   SymbolTable();
   void insert(EnumDecl* x);
   void insert(TypedefDecl* x);
   void insert(StructDecl* x);
   void insert(FuncDecl* x);
   void insert(ParamDecl* x);
   void insert(std::string id, Var_type* x);
   void insert(std::string id, Variable* x);
   void check_function_clash(std::string id, std::string class_name, AstNode* x);
   void check_type_clash(std::string id, std::string class_name, AstNode* x);
   bool contains(FuncDecl* x);
   bool contains(std::string id);
   Var_type* getType(std::string id);
   std::vector<Scope> scope_stack;
   std::unordered_map<std::string,std::vector<FuncHeader*> > ident2func;
};

class TypeChecker : public AstVisitor, public ReadWriter {
private:
   SymbolTable symTable;

   Var_type* expr_type; //updated by expressions

   void check_bool_type(AstNode* x);
   void visit_binaryexpr_assignment(Var_type* t1, Var_type* t2);
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
