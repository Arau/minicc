#include <iostream>
using namespace std;

int main() {
   int a;
   cin >> a;
   if (a == 42) {
      cout << "yes" << endl;
   } else {
      cout << "no" << endl;
   }
}
[[in]]---------------------------------------------------
41
[[out]]--------------------------------------------------
Saltamos a la función 'main'.
5:3-5:9: int a;
Se declara la variable 'a'.
6:3-6:12: cin >> a;
Se lee de la entrada.
7:7-7:14: a == 42
La condición vale 'false', tomamos la segunda rama.
10:6-10:27: cout << "no" << endl;
Se escribe a la salida.
[[err]]--------------------------------------------------