//
//
//      CATTAI LORENZO
//           2018
//
//

// tsleep([time in milliseconds]);                           ->  waits
// tclear();                                                 ->  clear the terminal
// readdim([height address(*int)],[width address(*int)]);    ->  read the current dimension of the terminal
// resize([lines(height)],[colums(width)]);                  ->  set the dimension of the terminal
// kbhit();                                                  ->  return 1 if there is a key in the buffer
// getcharacter;                                             ->  return the key pressed (you don't nedd to press enter)
// create_menu();
// create_advmenu();

//when using readdim() and resize() remember that a colum is 2 characters wide


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//compatibility for Unix systems
#if defined(__unix__)||defined(__APPLE__)

	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>

	//to get if a key has been pressed22
	int kbhit(void);

    //to get the pressed key
    #define getcharacter getchar2()

//compatibility for Windows systems
#elif defined(_WIN32)||defined(_WIN64)

    #include <Windows.h>

    #define OS 1

    int gettermbuf();

    //to get the pressed key
    #define getcharacter getch()

#endif

int loadfile(char *string_menu, char file[], int h, int b, int menu_width, int menu_height);
void resize(int h, int b);
void tsleep(int ms);
void tclear();
int create_menu(char file[], int menu_width, int menu_height, int menu_slots, int c_title,int c_title2,int c_slots, int c_an1,int c_an2);
int character(int ch, int phase, int selected, int c_title, int c_title2, int c_slots, int c_an1,int c_an2);
void readdim(int *h, int *b);
int create_advmenu(char file[], int *horizontal_selection, int horizontal_slots, int menu_width, int menu_height, int menu_slots, int c_title,int c_title2,int c_slots, int c_an1,int c_an2);

