int f() {
   int c = 0, d = 0;
   while (c < d) {
      while (c < d) {
         if (c < d) {
            c = 0;
            return c;
            d = 0;
            return 0;
         }
         else {
            c = 0;
            return d;
            d = 0;
            return c;
         }
         c = 0;
         return 0;
         d = 0;
         return 0;
      }
   }
   c = 0;
   return c;
   d = 0;
   return 0;
}

int main() {

}
[[err]]--------------------------------------------------
7,13: unreacheable instructions after return
13,13: unreacheable instructions after return
18,10: unreacheable instructions after return
24,4: unreacheable instructions after return