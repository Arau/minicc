void xxx(string s) {
   while (a = true)
   while (a = true) a = a + 2;
}
[[out]]--------------------------------------------------
Program{
   FuncDecl("xxx", Type(void), Params = {"s": Type(string)}, {
      Block({
         Stmt(while, =(id:'a', lit:'true'), {
            Stmt(while, =(id:'a', lit:'true'), {
               Stmt(expr, =(id:'a', +(id:'a', lit:'2')))
            })
         })
      })
   })
}
[[err]]--------------------------------------------------
