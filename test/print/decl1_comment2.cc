void f() {
   int a    /* xxxxx */      = 2, b;
}
[[out]]--------------------------------------------------
void f() {
   int a /* xxxxx */ = 2, b;
}
[[err]]--------------------------------------------------
