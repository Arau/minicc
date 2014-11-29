int main() {
   while (true) {
      break;
   }
   while (true) {
      continue;
   }
   while (true) {
      return;
   }
   while (true) {
      goto a;
   }
}
[[err]]--------------------------------------------------
5,4: infinite while loop
12,7: using a goto instruction