int f() {
   {
      int a;
      return a;
      a = 3;
      return a;
   }
   {
      int d;
      d = 3;
   }
   {
      int b;
      return b;
      b = 3;
   }
   {
      int c;
      c = 3;
   }
   return 0;
}

int main() {

}
[[err]]--------------------------------------------------
4,7: unreacheable instructions after return
14,7: unreacheable instructions after return