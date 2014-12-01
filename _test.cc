#include <iostream>

using namespace std;

typedef int myint;

int f(int a) {
	a++;
	return 0;
}

int f(int &b) {
	b++;
	return 0;
}

enum b {a , ba };
int main() {
	typedef float myint;
	myint i = 0.5;
	cout << i << endl;
	while (true) {
		typedef char myint;
		myint j = 'a';
		cout << j << endl;
		break;
	}
	int a = 3;
	int b = f(a);
	cout << a << endl;
}