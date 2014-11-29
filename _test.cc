#include <iostream>

using namespace std;

typedef int myint;

enum myenum { hol, a , que , tal };

void f() {
	cout << hol << endl;
}

int main() {
	enum myenum { hol, a , que , tal };
	cout << "hello " << hol << endl;
	f();
	myint m =3;
	int n = 2;
	cout << n+m << endl;
}