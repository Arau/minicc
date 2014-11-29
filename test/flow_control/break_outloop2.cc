int main() {
   break;
   continue;
   int i = 0;
   if (i < 0) {
      break;
      continue;
   }
   else {
      continue;
      break;
   }
   while (i < 0) {
      continue;
   }
   for (int j = 0; j < 0; j++) {
      break;
   }
}
[[err]]--------------------------------------------------
2,4: break statement outside loop
3,4: continue statement outside loop
6,7: break statement outside loop
7,7: continue statement outside loop
10,7: continue statement outside loop
11,7: break statement outside loop
