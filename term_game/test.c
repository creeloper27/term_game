#include <stdio.h>
#include <windows.h>
int main(){
    int r;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for(int k = 1; k < 255; k++){
        SetConsoleTextAttribute(hConsole, k);
        printf("\n%d: ciao");
    }

    return 0;
}
