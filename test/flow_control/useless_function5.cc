int f(int a, int b) {
	int c = a + b;
	int d = c * c;
	++d;
	a = 3;
	b = a;
	return d;
}

int main() {

}
[[err]]--------------------------------------------------
