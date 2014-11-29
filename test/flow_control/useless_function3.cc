void f(int a, int &b) {
	int c = a + b;
	int d = c * c;
	++d;
	a = 3;
	b = a; //this one is not useless because of reference
}

int main() {

}
[[err]]--------------------------------------------------
