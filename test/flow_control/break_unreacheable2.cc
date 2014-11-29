int main() {
   while (true) {
      while(true) {
         int d = 0;
         continue;
         int e = 0;
         break;
      }
      int a = 0;
      break;
      int c = 0;
      while(false) {
         int b = 0;
         continue;
         int f = 0;
         break;
      }
      break;
   }
}
[[err]]--------------------------------------------------
5,10: unreacheable instructions after continue
10,7: unreacheable instructions after break
12,13: the while condition is always false
14,10: unreacheable instructions after continue