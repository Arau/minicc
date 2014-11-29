int main() {
   goto a;
   int b = 2, c = 3;
   while (b < c) {
      if (b > c) {
         b = 3;
      }
      else {
         for (int i = 0; i < 3; i++) {
            goto b;
         }
      }
   }
}
[[err]]--------------------------------------------------
2,4: using a goto instruction
10,13: using a goto instruction