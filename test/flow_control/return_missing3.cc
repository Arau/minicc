int f() {}
float g() {}
bool h() {}
double i() {}
char j() {}
string k() {}
int ff(); //does not apply if there is no function body
float gg();
bool hh();
double ii();
char jj();
string k();
int main() {

}
[[err]]--------------------------------------------------
1,1: function 'f' does not have any return statement
2,1: function 'g' does not have any return statement
3,1: function 'h' does not have any return statement
4,1: function 'i' does not have any return statement
5,1: function 'j' does not have any return statement
6,1: function 'k' does not have any return statement
