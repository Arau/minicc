void f() {
   ++/*asdf*/a;
}
[[out]]--------------------------------------------------
void f() {
   ++ /*asdf*/ a;
}
