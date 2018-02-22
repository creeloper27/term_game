#include <stdio.h>
#include "term_menu.h"

int main(){
    int r;

    resize(100,200);
    while(r=create_menu("menu.txt",66,31,4,177,178,176,'<','>')){
        printf("\nr: %d",r);
        system("pause");
    }

    return 0;
}
