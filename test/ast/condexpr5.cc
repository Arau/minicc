void f() {
   a=b?x||y||z:z;
}
[[out]]--------------------------------------------------
Program{
   FuncDecl("f", Type(id:'void'), Params = {}, {
      Block({
         ExprStmt(=(id:'a', CondExpr(id:'b', ||(||(id:'x', id:'y'), id:'z'), id:'z')))
      })
   })
}
[[err]]--------------------------------------------------
