int f() {
   if (a == b) {
      cout + x;
   }
}
[[out]]--------------------------------------------------
Program{
   FuncDecl("f", Type(int), Params = {}, {
      Block({
         IfStmt(==(id:'a', id:'b'), Block({
            ExprStmt(+(id:'cout', id:'x'))
         }))
      })
   })
}
[[err]]--------------------------------------------------
