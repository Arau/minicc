void f(int a, int b) {
	int c = a + b;
	int d = c * c;
	++d;
	a = 3;
	b = a;
	return ; //still useless even with return
}

int main() {

}
[[err]]--------------------------------------------------
1,1: function 'f' does not do anything