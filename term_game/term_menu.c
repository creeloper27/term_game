//
//
//       ZANON GABRIELE
//           2018
//
//

#include "term_menu.h"

int resized=0;

//compatibility for Unix systems
#if defined(__unix__)||defined(__APPLE__)

    void readdim(int *h, int *b){
		struct winsize w;
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		*h = w.ws_col;
		*b = w.ws_row;
    }

	//to get if a key has been pressed
	int kbhit(void)
	{
	  struct termios oldt, newt;
	  int ch;
	  int oldf;

	  tcgetattr(STDIN_FILENO, &oldt);
	  newt = oldt;
	  newt.c_lflag &= ~(ICANON | ECHO);
	  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	  ch = getchar();

	  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	  fcntl(STDIN_FILENO, F_SETFL, oldf);

	  if(ch != EOF)
	  {
	    ungetc(ch, stdin);
	    return 1;
	  }

	  return 0;
	}

	//to sleep
	void tsleep(int ms){
		char c[20];
		sprintf(c,"sleep %f",(float)ms/1000.0);
		system(c);
	}


    //to get the pressed key
    char getchar2(){
        char c;
        c=getchar();
        printf("\b");
        return c;
    }

    //to clear
    void tclear(){
        system("clear");
    }

    //to get the pressed key
    #define getcharacter getchar2();

    //to resize the window
    void resize(int h, int b){
        char command[50];
        sprintf(command,"resize -s %d %d",b,h);
        system(command);
        resized=1;
    }

//compatibility for Windows systems
#elif defined(_WIN32)||defined(_WIN64)

    #define OS 1

    int gettermbuf(){
		CONSOLE_SCREEN_BUFFER_INFO csbi;

        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        return csbi.dwSize.Y;
    }


    void readdim(int *h, int *b){
		CONSOLE_SCREEN_BUFFER_INFO csbi;

    	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    	*h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    	*b = csbi.srWindow.Right + 1;
    }

    //to sleep
	void tsleep(int ms){
		Sleep(ms);
	}

    //to clear
    void tclear(){
        system("cls");
    }

    //to get the pressed key
    #define getcharacter getch();

    //to resize the window
    void resize(int h, int b){
        char command[50];
        sprintf(command,"mode con:cols=%d lines=%d",b,h);
        //start debug
        //printf("\n\n%s\n\n",command);
        //system("pause");
        //end debug
        system(command);
        resized=1;
    }

#endif

int loadfile(char *string_menu, char file[], int h, int b, int menu_width, int menu_height){
    int cont2=0, i=0;
    char ch;
    FILE *menufile;
    menufile = fopen(file, "r+");
    if(menufile){
        for(i=0;i<(b-menu_width)/2;i++){ //imposta spaziatura di sinistra per la prima riga del file
            string_menu[cont2]=' ';
            cont2++;
        }
        while ((ch = fgetc(menufile)) != EOF){
            string_menu[cont2]=ch;
            cont2++;
            if(ch=='\n'){   //imposta spaziatura di sinistra nel file
                for(i=0;i<(b-menu_width)/2;i++){
                    string_menu[cont2]=' ';
                    cont2++;
                }
            }
        }
        fclose(menufile);
    }
    return cont2;
}

//create a menu
int create_menu(char file[], int menu_width, int menu_height, int menu_slots, int c_title,int c_title2,int c_slots, int c_an1,int c_an2){
    char toPrint2[100000];
    char string_menu[100000];
    int cont=0, cont2=0, select=0,i;
    int ch,ch2,m_exit=0,phase=0,selected=49;
    int h=0,b=0,temp1,temp2,check=0;

    readdim(&h,&b);
    if(resized==0){
        resize(h,b);    //diminuisce il buffer del terminale
        resized=1;
    }

    cont2=loadfile(string_menu,file,h,b,menu_width,menu_height);
    //usare %c,7 per suoni menu

    while(m_exit==0){
        if(kbhit()){
            //take the key from the buffer and put it in "ch"
            ch = getcharacter;
            ch2=' ';
            switch(ch){
                case 27:
                    m_exit=1;
                    break;
                case 'w':
                    selected--;
                    break;
                case 's':
                    selected++;
                    break;
                case 'a':
                    //seleziona a sinistra
                    break;
                case 'd':
                    //seleziona a destra
                    break;
                case 224:       //se è un carattere 244 [...] es, freccette
                    ch2=getcharacter;
                    if(ch2==72)
                        selected--;
                    else if(ch2==80)
                        selected++;
                    else if(ch2==75)
                        printf("");
                        //seleziona a sinistra
                    else if(ch2==77)
                        printf("");
                        //seleziona a destra
                    break;
                case 13:
                    select=1;
                    break;
                case '\n':
                    select=1;
                    break;
            }

            if(selected<48)
                selected=48+menu_slots-1;
            if(selected>48+menu_slots-1)
                selected=48;

            if(select){
                if(selected==48+menu_slots){
                    return 0;
                }else{
                    return (48-selected)*-1;
                }
            }


        }
        for(i=0;i<(h-menu_height)/2;i++){
            toPrint2[cont]='\n';
            cont++;
        }

        for(i=0;i<cont2;i++){
            toPrint2[cont]=character(string_menu[i],phase%2,selected,c_title,c_title2,c_slots,c_an1,c_an2);    //I use extended ascii characters so in character() i return the extended ascii character for windows and another character for linux
            cont++;
        }

        for(i=0;i<(h-menu_height)/2;i++){
            toPrint2[cont]='\n';
            cont++;
        }

        toPrint2[cont]='\0';
        cont++;

        printf("%s",toPrint2);

        for(i=0;i<cont;i++){
            toPrint2[i]=' ';
        }
        cont=0;
        phase++;
        if(phase>=100)
            phase=0;

        if(check>=6){
            temp1=h;
            temp2=b;
            readdim(&h,&b);

            if(temp1!=h||temp2!=b){
                cont2=loadfile(string_menu,file,h,b,menu_width,menu_height);
            }
            check=0;
        }
        check++;

        tsleep(32);
    }

    return 0;
}

//I use extended ascii characters so in character() i return the extended ascii character for windows and another character for linux
int character(int ch, int phase, int selected, int c_title, int c_title2, int c_slots, int c_an1,int c_an2){
    //▒   177
    //░   176
    //ESC 27
    //#   35
    //<   60
    //>   62

    switch(ch){
    case 48 ... 57:
        if(ch==selected){
            if(phase)
                return c_an1;  //<
            else
                return c_an2;  //>
        }else{
            #ifdef OS
                return c_slots;
            #else
                return 35;
            #endif
        }
    case ']':
        #ifdef OS
            return c_title;
        #else
            return 35;
        #endif
    case '[':
        #ifdef OS
            return c_title2;
        #else
            return 35;
        #endif
    }
    return ch;
}
