#include <stdio.h>
#include <time.h>
#include <stdlib.h>

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
	#define resize system("resize -s 89 47");
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
        printf("\n\n%s\n\n",command);
        system("pause");
        system(command);
        system("pause");
    }

#endif

//matrix "w" --> contains the background
#define HEIGHT 40
#define WIDTH 88

//matrix "e" --> contains all the enitities
#define N 10
#define P 7

//NOT USED
//-----------------------------------------------------------------
//numero carattere asci, h, b, type, verso, velocità, dimensione.
//types: 0-objectect 1-player 2-projectile

//matrix "players" --> contains the players infos
#define P_MAX   //max number of players
#define P_ARGS  //arguments for every player
//-----------------------------------------------------------------

//controls
#define FORWARD1 'w'
#define BACKWARD1 's'
#define LEFT1 'a'
#define RIGHT1 'd'
#define FIRE1 'e'

#define FORWARD2 'i'
#define BACKWARD2 'k'
#define LEFT2 'j'
#define RIGHT2 'l'
#define FIRE2 'o'

//global variables
int points1=0;
int points2=0;
int player1=88;
int player2=79;
int islog=1;
int isrender=1;
FILE *logfile;

//prototypes
void clear(){system(CLEAR);}
void Render(int debug,int fps,int delay,int fps_time, char w[HEIGHT][WIDTH], int e[][P]);
void winizializza(char x, char w[HEIGHT][WIDTH]);
void einizializza(int x, int e[N][P]);
int menu();
int getday();
int getmonth();
int getyear();
int gethour();
int getmin();
int getsec();
void MsgBoxv(char mex[1024],char dato,char nome[1024],int tipo);
void printlog(char a[], int b, char c[]);

//message box
void MsgBoxv(char mex[1024],char dato,char nome[1024],int tipo){
    char msg[1024];
    sprintf(msg, "%s%c", mex, dato);
    MsgBox(msg,nome,tipo);
}

//print to log file
void printlog(char a[], int b, char c[]){
    if(islog){
        char mex[30];
        sprintf(mex,"\n[%d:%d:%d] %s%d%s",gethour(), getmin(), getsec(), a, b, c);
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

//initialize matrix "world" (w)
void winizializza(char x, char w[HEIGHT][WIDTH]){
    int i,i2;

    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            w[i][i2]=x;
        }
    }
}

//initialize matrix "entity" (e)
void einizializza(int x, int e[N][P]){
    int i,i2;

    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            e[i][i2]=x;
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

//MAIN
int main(int argc, char *argv[]){
    char ch;
    char w[HEIGHT][WIDTH];
    int e[N][P];
    int delay=16;
    int Cstart,i=0,fps=0,fps_time;
    int h=2,b=4;
    int ph=-1,pb=-1;
    int W,Wc;
    int i2;
    int h2=HEIGHT-3,b2=WIDTH-6;
    int ph2=-1,pb2=-1;
    int isProjectile1=0;
    int isProjectile2=0;
    int contn=0;
    int term_h,term_b;


    printf("\n\n%s\n\n",argv[1]);
    if(argc>=1&&argv[1]=="--help"){
        printf("\nterm_game [terminal height] [terminal base]\n");
        return 0;
    }else if(argc>=2){
        sscanf(argv[1], "%d", &term_h);
        sscanf(argv[2], "%d", &term_b);
        resize(term_h,term_b);
    }else{
        resize(HEIGHT+2,WIDTH+1) ;
    }

    system("pause");

    winizializza(' ', w);
    einizializza(-1 , e);

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

    //menu();
    while(points1<10&&points2<10){


        if(i==0){
            Cstart=clock();
            i--;
        }
        else if(i==1){
            i=0;
            //fps_time=delay + time for fps
            fps_time=clock()-Cstart;
            fps=1000/fps_time;
            //fps_time=time for fps
            fps_time-=delay;
        }

        //if a key has been pressed
        if(kbhit()){

            //take the key from the buffer and put it in "ch"
            ch = getcharacter;


            if(ch==FORWARD1)
                h--;
            else if(ch==RIGHT1)
                b+=2;
            else if(ch==BACKWARD1)
                h++;
            else if(ch==LEFT1)
                b-=2;
            else if(ch==FIRE1){
                ph=h;
                pb=b;
                isProjectile1=1;
            }

            if(ch==FORWARD2)
                h2--;
            else if(ch==RIGHT2)
                b2+=2;
            else if(ch==BACKWARD2)
                h2++;
            else if(ch==LEFT2)
                b2-=2;
            else if(ch==FIRE2){
                ph2=h2;
                pb2=b2;
                isProjectile2=1;
            }
            //if(ch=='|'){
            //    menu();
            //}

        }

        //projectiles manager
        if(pb>WIDTH)
            isProjectile1=0;

        if(isProjectile1==1)
            pb+=2;

        if(pb2<0)
            isProjectile2=0;

        if(isProjectile2==1)
            pb2-=2;
        if(pb==pb2&&ph==ph2){
            ph=-1;
            pb=-1;
            ph2=-1;
            pb2=-1;
            isProjectile1=0;
            isProjectile2=0;
        }

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

        //transfer data to the entities matrix for the rendering
        e[0][0]=88;
        e[0][1]=h;
        e[0][2]=b;
        e[0][3]=1;

        e[1][0]=45;
        e[1][1]=ph;
        e[1][2]=pb;

        e[2][0]=79;
        e[2][1]=h2;
        e[2][2]=b2;
        e[2][3]=1;

        e[3][0]=43;
        e[3][1]=ph2;
        e[3][2]=pb2;

        //check for a winner
        if(points1>=10){
            Wc=player1;
            W=1;
        }
        if(points2>=10){
            Wc=player2;
            W=2;
        }

        //render the frame
        Render(1,fps ,fps_time ,delay, w, e);

        //sleep 13 ms to try and get 60fps
        msleep(delay);
    }

    //print the winner in the log
    printlog("game-over ",W," vince!");
    if(islog)
        fclose(logfile);

    //print the winner in the messagebox
    MsgBoxv("iL VINCITORE è: ",Wc,"GAME OVER",1);
    return 0;
}

void Render(int debug,int fps,int fps_time,int delay, char w[HEIGHT][WIDTH], int e[][P]){
    char toPrint[10000];
    int c1,c2,c3,c4;
    int a1,a2,a3,a4;
    int d1,d2,d3,d4;
    int i,i2,i3,i4,cont=0;
    int background=0;

    //generate debug infos in the frame

    //fps
    c1=(fps/1000-(fps/10000)*10);
    c2=(fps/100-(fps/1000)*10);
    c3=(fps/10-(fps/100)*10);
    c4=(fps/1-(fps/10)*10);

    //fps_time
    a1=(fps_time/1000-(fps_time/10000)*10);
    a2=(fps_time/100-(fps_time/1000)*10);
    a3=(fps_time/10-(fps_time/100)*10);
    a4=(fps_time/1-(fps_time/10)*10);

    //delay
    d1=(delay/1000-(delay/10000)*10);
    d2=(delay/100-(delay/1000)*10);
    d3=(delay/10-(delay/100)*10);
    d4=(delay/1-(delay/10)*10);

    //fps
    toPrint[cont]='f';
    cont++;
    toPrint[cont]='p';
    cont++;
    toPrint[cont]='s';
    cont++;
    toPrint[cont]=':';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=c1+'0';
    cont++;
    toPrint[cont]=c2+'0';
    cont++;
    toPrint[cont]=c3+'0';
    cont++;
    toPrint[cont]=c4+'0';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=' ';
    cont++;
    //fps_time
    toPrint[cont]='f';
    cont++;
    toPrint[cont]='p';
    cont++;
    toPrint[cont]='s';
    cont++;
    toPrint[cont]='_';
    cont++;
    toPrint[cont]='t';
    cont++;
    toPrint[cont]='i';
    cont++;
    toPrint[cont]='m';
    cont++;
    toPrint[cont]='e';
    cont++;
    toPrint[cont]=':';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=a1+'0';
    cont++;
    toPrint[cont]=a2+'0';
    cont++;
    toPrint[cont]=a3+'0';
    cont++;
    toPrint[cont]=a4+'0';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=' ';
    cont++;
    //delay
    toPrint[cont]='d';
    cont++;
    toPrint[cont]='e';
    cont++;
    toPrint[cont]='l';
    cont++;
    toPrint[cont]='a';
    cont++;
    toPrint[cont]='y';
    cont++;
    toPrint[cont]=':';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=d1+'0';
    cont++;
    toPrint[cont]=d2+'0';
    cont++;
    toPrint[cont]=d3+'0';
    cont++;
    toPrint[cont]=d4+'0';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=' ';
    cont++;
    //points
    toPrint[cont]=player1;
    cont++;
    toPrint[cont]=':';
    cont++;
    toPrint[cont]=points1+'0';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=' ';
    cont++;
    toPrint[cont]=player2;
    cont++;
    toPrint[cont]=':';
    cont++;
    toPrint[cont]=points2+'0';
    cont++;
    toPrint[cont]='\n';
    cont++;

    //generate the frame and put it in toPrint
    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            background=1;

            //check entity table
            for(i3=0;i3<N&&background==1;i3++){
                if(e[i3][1]==i&&e[i3][2]==i2){
                    toPrint[cont]=e[i3][0];
                    cont++;

                    //width 2
                    if(e[i3][3]==1){
                        toPrint[cont]=e[i3][0];
                        cont++;
                        i2++;
                    }
                    background=0;
                }
            }
            if(background){
                toPrint[cont]=w[i][i2];
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
