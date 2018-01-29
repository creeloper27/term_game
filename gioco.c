#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

//compatibility for Unix systems
#if defined(__unix__)||defined(__APPLE__)

	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>

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
	void msleep(int ms){
		char c[20];
		sprintf(c,"sleep %f",(float)ms/1000.0);
		system(c);
	}

	//to create a message box
    	void MsgBox(char *contenuto, char *finestra, int tipo){
        char cmd[1024];
        sprintf(cmd, "xmessage -center \"%s\"", contenuto);
        if(fork()==0){
            close(1); close(2);
            system(cmd);
            exit(0);
        }
    }

    //to get the pressed key
    char getchar2(){
        char c;
        c=getchar();
        printf("\b");
        return c;
    }

    //to clear
    char CLEAR[]="clear";

    //to get the pressed key
    #define getcharacter getchar2();

    //to resize the window
    void resize(int h, int b){
        char command[50];
        sprintf(command,"resize -s %d %d",b,h);
        system(command);
    }

//compatibility for Windows systems
#elif defined(_WIN32)||defined(_WIN64)

    #include <Windows.h>

    //to sleep
	void msleep(int ms){
		Sleep(ms);
	}

	//to create a message box
    void MsgBox(char *contenuto, char *finestra, int tipo){
        MessageBox(0, contenuto, finestra, tipo);
    }

    //to clear
    const char CLEAR[]="cls";

    //to get the pressed key
    #define getcharacter getch();

    //to resize the window
    void resize(int h, int b){
        char command[50];
        sprintf(command,"mode con:cols=%d lines=%d",b,h);
        //start debug
        printf("\n\n%s\n\n",command);
        system("pause");
        //end debug
        system(command);
    }

#endif

//matrix "world" --> contains the background
#define HEIGHT 40
#define WIDTH 88

//array "entity_other" --> contains entities that aren't projectiles or players
#define OTHER_ENTITY_MAX 10

//array "player" --> contains the players
#define PLAYER_MAX 10

//array "projectile" --> contains the projectiles
#define PROJECTILE_MAX 10

//struct entity_other (contains all types of infos, it is used for other types of entities)
typedef struct{
    int is;         //is it active?
    int h;
    int b;
    char ascii;     //ascii character
    int dim;        //size on projectile
    int direction;  //direction
    int speed;      //speed
    int points;     //points earned

    //name
    char name[10];  //name

    //controls, -1 if it hasn't one
    char top;       //go-top key
    char bottom;    //go-bottom key
    char left;      //go-left key
    char right;     //go-right key
    char fire;      //fire key

}entity_other;

//struct players contains just needed infos
typedef struct{
    int h;
    int b;
    char name[15];          //player name
    char ascii;             //ascii character
    char ascii_projectile;  //ascii of the projectile
    int direction;          //which way is he facing? 0-top 1-right 2-bottom 3-left
    char top;               //go-top key
    char bottom;            //go-bottom key
    char left;              //go-left key
    char right;             //go-right key
    char fire;              //fire key
    int is;                 //is it active?
}entity_player;

//struct players contains just needed infos
typedef struct{
    int h;
    int b;
    char ascii;     //ascii character
    int dim;        //size on projectile
    int direction;  //direction
    int speed;      //speed
    int is;         //is it active?
}entity_projectile;

//global variables
int points1=0;
int points2=0;
int player1=88;
int player2=79;
int islog=1;
int isrender=1;
int other_number=0;
int player_number=2;
int projectile_number=0;
char stemp[150];
FILE *logfile;
entity_other other[OTHER_ENTITY_MAX];
entity_player player[PLAYER_MAX];
entity_projectile projectile[PROJECTILE_MAX];


//prototypes
void clear(){system(CLEAR);}
void Render(int debug,int fps,int fps_time,int delay, char world[HEIGHT][WIDTH], entity_other other[], int other_number, entity_player player[], int player_number, entity_projectile projectile[], int projectile_number);
void winizializza(char x, char world[HEIGHT][WIDTH]);
int menu();
int getday();
int getmonth();
int getyear();
int gethour();
int getmin();
int getsec();
void MsgBoxv(char mex[1024],char dato,char nome[1024],int tipo);
void printlog(char a[]);
void tick();
int checkmovement(char ch, int p);
void fire(int p);
void physics();
void sort_projectiles();
void atp(int *cont, char toPrint[], char c[]);

//message box
void MsgBoxv(char mex[1024],char dato,char nome[1024],int tipo){
    char msg[1024];
    sprintf(msg, "%s%c", mex, dato);
    MsgBox(msg,nome,tipo);
}

//print to log file
void printlog(char a[]){
    if(islog){
        char mex[150];
        sprintf(mex,"\n[%d:%d:%d] %s",gethour(), getmin(), getsec(), a);
        fprintf(logfile,mex);
    }
}

//get the day
int getday(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_mday;
}
//get the month
int getmonth(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_mon + 1;
}
//get the year
int getyear(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_year + 1900;
}
//get the hour
int gethour(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_hour;
}
//get the minute
int getmin(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_min;
}
//get the second
int getsec(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm.tm_sec;
}

//initialize matrix "world"
void winizializza(char x, char world[HEIGHT][WIDTH]){
    int i,i2;

    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            world[i][i2]=x;
        }
    }
}

//main menu
int menu(){
    char daprintare2[1024];
    int cont=0,i=0;

    for(i=0;i<(HEIGHT-5)/2;i++){
        daprintare2[cont]='\n';
        cont++;
    }
    for(i=0;i<(WIDTH-15)/2;i++){
        daprintare2[cont]='\n';
        cont++;
    }
    daprintare2[cont]=218;
    for(i=0;i<13;i++){
        daprintare2[cont]=196;
        cont++;
    }
    daprintare2[cont]=191;
    printf("%s",daprintare2);
    system("pause");
    return 0;
}

//tick
void tick(){
    char ch;
    int i,flag=1;
    //check for controls matches (if keys have been pressed)
    if(kbhit()){
        //take the key from the buffer and put it in "ch"
        ch = getcharacter;
        for(i=0;i<player_number&&flag;i++){
            if(checkmovement(ch, i)){
                    flag=0;    //exit if there is a mach
            }
        }
    }
    physics();
}

//controlla le corrispondenze con i tasti
int checkmovement(char ch, int p){
    if(ch==player[p].top){
        //l'if è su un'altra riga per evitare di controllare tutte le condizioni
        if(player[p].h>1){player[p].h--;player[p].direction=0;}
        return 1;
    }else if(ch==player[p].bottom){
        if(player[p].h<HEIGHT-1) {player[p].h++;player[p].direction=2;}
        return 1;
    }else if(ch==player[p].left){
        if(player[p].b>2) {player[p].b-=2;player[p].direction=3;}
        return 1;
    }else if(ch==player[p].right){
        if(player[p].b<WIDTH-2) {player[p].b+=2;player[p].direction=1;}
        return 1;
    }else if(ch==player[p].fire){
        if(projectile_number<10)
            fire(p);
        return 1;
    }else{
        return 0;
    }
}

//create a projectile with the player infos
void fire(int p){
    projectile_number++;

    projectile[projectile_number-1].h=player[p].h;
    projectile[projectile_number-1].b=player[p].b;
    projectile[projectile_number-1].ascii=player[p].ascii_projectile;
    projectile[projectile_number-1].direction=player[p].direction;
    projectile[projectile_number-1].speed=1;
    projectile[projectile_number-1].is=1;     //when the projectile goes off screen or hit something, .is is set to 0 and the projectile is removed from the projectile array

}

//advance projectiles [...]
void physics(){
    int i;
    for(i=0;i<projectile_number;i++){
        if(projectile[i].h>HEIGHT||projectile[i].b>WIDTH||projectile[i].h<0||projectile[i].b<0){
            projectile[i].is=0;
        }
    }
    sort_projectiles();     //delete the projectiles with is=0 from the array
    for(i=0;i<projectile_number;i++){
        switch (projectile[i].direction){
            case 0:
                projectile[i].h--;
                break;
            case 1:
                projectile[i].b+=2;
                break;
            case 2:
                projectile[i].h++;
                break;
            case 3:
                projectile[i].b-=2;
                break;
        }
    }
}

//delete the projectiles with is=0 from the array
void sort_projectiles(){
    int sortered=1,i=0,i2=0,i3=0,flag=1;

    for(i=0;i<projectile_number&&flag;i++){
        if(projectile[i].is==0){
            sortered=0;
            flag=0;
        }
    }
    sprintf(stemp,"i=%d",i);
    printlog(stemp);


    //sort the array
    while(sortered==0){

        if(projectile_number==1){
            projectile_number--;
        }else{
            for(i2=i-1;i2<projectile_number-1;i2++){
                projectile[i2]=projectile[i2+1];

            }
            projectile_number--;
        }

        sortered=1;
        for(i=0;i<projectile_number;i++){
            if(projectile[i].is==0){
                sortered=0;
            }
        }
    }
}

//MAIN
int main(int argc, char *argv[]){
    char world[HEIGHT][WIDTH];
    int delay=16;
    int Cstart,i=0,fps=0,fps_time;
    int term_h,term_b;

    //menu for setting up players, controls etc
    //to asign strings, use strcpy() because you can't directly asign
    player[0].is=1;
    strcpy(player[0].name,"player 1");
    player[0].ascii=88;
    player[0].h=10;
    player[0].b=10;
    player[0].ascii_projectile=43;
    player[0].fire='e';
    player[0].top='w';
    player[0].bottom='s';
    player[0].right='d';
    player[0].left='a';

    player[1].is=1;
    strcpy(player[1].name,"player 2");
    player[1].ascii=79;
    player[1].h=HEIGHT-2;
    player[1].b=WIDTH-4;
    player[1].ascii_projectile=45;
    player[1].fire='o';
    player[1].top='i';
    player[1].bottom='k';
    player[1].right='l';
    player[1].left='j';

    //entity conterr� entit� che non sono ne giocatori ne proiettili, verr� utilizzato nel futuro
    //every time a game starts verryte entity and projectiles arrays and keep player array(just reset positions)
    //every time a player shoots create a struct on the projectile array
    //create function moveleft() moveright() fire().... etc



    argc--; //by default it starts from 1 (0 is the program name) , by decresing it by 1 it starts from 0
    printf("\narguments: %d",argc);

    //resize window
    if(argc>=2){
        printf("\n\n%s\n%s\n\n",argv[1],argv[2]);
    }

    if(argc==1&&!strcmp(argv[1],"--help")){
        printf("\nterm_game [terminal height] [terminal base]\n");
        return 0;
    }else if(argc==2){
        sscanf(argv[1], "%d", &term_h);
        sscanf(argv[2], "%d", &term_b);
        resize(term_h,term_b);
    }else{
        resize(HEIGHT+7,WIDTH+1);
    }


    //create log file
    if(islog){
        int numlogfile=0;
        char nomelogfile[15];

        if(!fopen("log_last.txt", "r+")){
            FILE *pFile1 = fopen("log_last.txt", "w");
            fprintf(pFile1,"0");
            fclose(pFile1);
        }else{
            FILE *pFile2 = fopen("log_last.txt", "r+");
            fscanf(pFile2, "%d", &numlogfile);
            numlogfile++;
            fclose(pFile2);

            FILE *pFile3 = fopen("log_last.txt", "w");
            fprintf(pFile3, "%d",numlogfile);
            fclose(pFile3);
        }

        sprintf(nomelogfile,"log_%d.txt",numlogfile);
        logfile = fopen(nomelogfile, "a+");
        fprintf(logfile, "date: %d-%d-%d", getyear(), getmonth(), getday());
    }

    //inizializza background
    winizializza(' ', world);
    i=0;

    while(points1<10&&points2<10){

        if(i==0){
            Cstart=clock();
            i++;
        }else if(i==1){
            i=0;
            //fps_time=delay + time for fps
            fps_time=clock()-Cstart;
            fps=1000/fps_time;
            //fps_time=time for fps
            fps_time-=delay;
        }

        tick();

        //struttura
        /*
        dentro tick() si controlla se ci sono proiettili e se il tasto premuto fa qualcosa, usare funzioni per muovere il player
        movetop(player number) -> controlla se pu� muversi in quella direzione o no
        */

        //VECCHIA STRUTTURA
/*
        //death manager
        if((h==ph2&&b==pb2)||(h==ph2&&b+1==pb2)){
            h=2;
            b=4;
            points2++;
            isProjectile2=0;
            ph2=-1;
            pb2=-1;
        }
        if((h2==ph&&b2==pb)||(h2==ph&&b-1==pb)){
            h2=HEIGHT-3;
            b2=WIDTH-6;
            points1++;
            isProjectile1=0;
            ph=-1;
            pb=-1;
        }

        //check for a winner
        if(points1>=10){
            Wc=player1;
            W=1;
        }
        if(points2>=10){
            Wc=player2;
            W=2;
        }
        */

        //render the frame
        Render(1,fps ,fps_time ,delay, world, other, other_number, player, player_number, projectile, projectile_number);

        //sleep 13 ms to try and get 60fps
        msleep(delay);
    }

    //print the winner in the log
    //printlog("game-over ",W," vince!");
    if(islog)
        fclose(logfile);

    //print the winner in the messagebox
    //msgBoxv("iL VINCITORE è: ",Wc,"GAME OVER",1);
    return 0;
}

//Add To (to)Print
void atp(int *cont, char toPrint[], char c[]){
    int i;
    for(i=0;i<strlen(c)-1;i++){
        toPrint[*cont]=c[i];
        *cont=*cont+1;
    }
}

void Render(int debug,int fps,int fps_time,int delay, char world[HEIGHT][WIDTH], entity_other other[], int other_number, entity_player player[], int player_number, entity_projectile projectile[], int projectile_number){
    char toPrint[100000];
    char temp[200];
    int i,i2,i3,cont=0;
    int background=0;

    //generate debug infos in the frame
    if(debug){
        sprintf(temp,"fps: %d\nfps_time: %d delay: %d\nPOINTS: use percentS and generate it outside according to the players number\nprojectile_number: %d\nplayer_number: %d\nother_number: %d\n",fps,fps_time,delay,projectile_number,player_number,other_number);
        atp(&cont,toPrint,temp);
        //for(i=0;i<projectile_number;i++){
        //    sprintf(temp,"\nprojectile[%d].is=%d ",i,projectile[i].is);
        //    atp(&cont,toPrint,temp);
        //}
    }

    //generate the frame and put it in toPrint
    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            background=1;

            //check the 3 arrays other player projectile

            //players
            for(i3=0;i3<player_number&&background;i3++){
                if(player[i3].h==i&&player[i3].b==i2){
                    toPrint[cont]=player[i3].ascii;
                    cont++;

                    background=0;
                }
            }

            //projectiles
            for(i3=0;i3<projectile_number&&background;i3++){
                if(projectile[i3].h==i&&projectile[i3].b==i2&&projectile[i3].is){
                    toPrint[cont]=projectile[i3].ascii;
                    cont++;

                    background=0;
                }
            }

            //other entities
            for(i3=0;i3<other_number&&background;i3++){
                if(other[i3].h==i&&other[i3].b==i2){
                    toPrint[cont]=other[i3].ascii;
                    cont++;

                    background=0;
                }
            }

            if(background){
                toPrint[cont]=world[i][i2];
                cont++;
            }
        }

        toPrint[cont]='\n';
        cont++;
    }
        //set an end to the array (save time)
        toPrint[cont]='\0';

        //print the frame (toPrint)
        printf("%s",toPrint);

}
