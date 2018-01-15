#include <stdio.h>
#include <time.h>

#ifdef __unix__ || __APPLE__

	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <unistd.h>

    void MsgBox(char *contenuto, char *finestra, int tipo){
        char cmd[1024];
        sprintf(cmd, "xmessage -center \"%s\"", s);
        if(fork()==0){
            close(1); close(2);
            system(cmd);
            exit(0);
        }
    }

    const char CLEAR[]="clear";

	#define character 1
	#define getcharacter read();
	#define resize system("resize -s 89 47");

#elif defined(_WIN32) || defined(_WIN64)

    #include <Windows.h>

    void MsgBox(char *contenuto, char *finestra, int tipo){
        MessageBox(0, contenuto, finestra, tipo);
    }

    const char CLEAR[]="cls";

    #define character kbhit();
    #define getcharacter getch();
    #define resize system("mode con:cols=89 lines=47");

#endif

#define H 45
#define B 88
#define N 10
//numero carattere asci, h, b, type, verso, velocità, dimensione.
//types: 0-objectect 1-player 2-projectile
#define P 7

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
FILE *logfile;

//prototypes
void clear(){system(CLEAR);}
char read(){};
void Render(int debug,int fps,int delay,int fps_time, char w[H][B], int e[][P]);
void winizializza(char x, char w[H][B]);
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
void winizializza(char x, char w[H][B]){
    int i,i2;

    for(i=0;i<H;i++){
        for(i2=0;i2<B;i2++){
            w[i][i2]=x;
        }
    }
}

//initialize matrix "entity" (e)
void einizializza(int x, int e[N][P]){
    int i,i2;

    for(i=0;i<H;i++){
        for(i2=0;i2<B;i2++){
            e[i][i2]=x;
        }
    }
}

//main menu
int menu();

//MAIN
int main(){
    char ch;
    char w[H][B];
    int e[N][P];
    int delay=16;
    int Cstart,i=0,fps=0,fps_time;
    int h=2,b=4;
    int ph=-1,pb=-1;
    int W,Wc;
    int i2;
    int h2=H-3,b2=B-6;
    int ph2=-1,pb2=-1;
    int isProjectile1=0;
    int isProjectile2=0;
    int contn=0;

    resize;
    winizializza(' ', w);
    einizializza(-1 , e);

    //log file
    if(islog){
        int numlogfile=0;
        char nomelogfile[6];

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

    while(points1<10&&points2<10){
        if(i==0){
            Cstart=clock();
            i++;
        }
        else if(i==1){
            i=0;
            //fps_time=delay + tempo per fps
            fps_time=clock()-Cstart;
            fps=1000/fps_time;
            //fps_time=tempo per fps
            fps_time-=delay;
        }
        if(kbhit()){

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
        }

        //gestione projectiles
        if(pb>B)
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

        //gestione e rilevamento morti
        if((h==ph2&&b==pb2)||(h==ph2&&b+1==pb2)){
            h=2;
            b=4;
            points2++;
            isProjectile2=0;
            ph2=-1;
            pb2=-1;
        }
        if((h2==ph&&b2==pb)||(h2==ph&&b-1==pb)){
            h2=H-3;
            b2=B-6;
            points1++;
            isProjectile1=0;
            ph=-1;
            pb=-1;
        }

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

        if(points1>=10){
            Wc=player1;
            W=1;
        }
        if(points2>=10){
            Wc=player2;
            W=2;
        }

        Render(1,fps ,fps_time ,delay, w, e);
        Sleep(delay);
    }
    printlog("game-over ",W," vince!");
    if(islog)
        fclose(logfile);
    MsgBoxv("iL VINCITORE è: ",Wc,"GAME OVER",1);
    return 0;
}

void Render(int debug,int fps,int fps_time,int delay, char w[H][B], int e[][P]){
    char daPrintare[10000];
    int c1,c2,c3,c4;
    int a1,a2,a3,a4;
    int d1,d2,d3,d4;
    int i,i2,i3,i4,cont=0;
    int sfondo=0;

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
    daPrintare[cont]='f';
    cont++;
    daPrintare[cont]='p';
    cont++;
    daPrintare[cont]='s';
    cont++;
    daPrintare[cont]=':';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=c1+'0';
    cont++;
    daPrintare[cont]=c2+'0';
    cont++;
    daPrintare[cont]=c3+'0';
    cont++;
    daPrintare[cont]=c4+'0';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    //fps_time
    daPrintare[cont]='f';
    cont++;
    daPrintare[cont]='p';
    cont++;
    daPrintare[cont]='s';
    cont++;
    daPrintare[cont]='_';
    cont++;
    daPrintare[cont]='t';
    cont++;
    daPrintare[cont]='i';
    cont++;
    daPrintare[cont]='m';
    cont++;
    daPrintare[cont]='e';
    cont++;
    daPrintare[cont]=':';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=a1+'0';
    cont++;
    daPrintare[cont]=a2+'0';
    cont++;
    daPrintare[cont]=a3+'0';
    cont++;
    daPrintare[cont]=a4+'0';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    //delay
    daPrintare[cont]='d';
    cont++;
    daPrintare[cont]='e';
    cont++;
    daPrintare[cont]='l';
    cont++;
    daPrintare[cont]='a';
    cont++;
    daPrintare[cont]='y';
    cont++;
    daPrintare[cont]=':';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=d1+'0';
    cont++;
    daPrintare[cont]=d2+'0';
    cont++;
    daPrintare[cont]=d3+'0';
    cont++;
    daPrintare[cont]=d4+'0';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    //punteggi
    daPrintare[cont]=player1;
    cont++;
    daPrintare[cont]=':';
    cont++;
    daPrintare[cont]=points1+'0';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=' ';
    cont++;
    daPrintare[cont]=player2;
    cont++;
    daPrintare[cont]=':';
    cont++;
    daPrintare[cont]=points2+'0';
    cont++;
    daPrintare[cont]='\n';
    cont++;

    for(i=0;i<H;i++){
        for(i2=0;i2<B;i2++){
            sfondo=1;

            //controllo tabella entità
            for(i3=0;i3<N&&sfondo==1;i3++){
                if(e[i3][1]==i&&e[i3][2]==i2){
                    daPrintare[cont]=e[i3][0];
                    cont++;

                    //larghezza 2
                    if(e[i3][3]==1){
                        daPrintare[cont]=e[i3][0];
                        cont++;
                        i2++;
                    }
                    sfondo=0;
                }
            }
            if(sfondo){
                daPrintare[cont]=w[i][i2];
                cont++;
            }
        }
        daPrintare[cont]='\n';
        cont++;
    }
        daPrintare[cont]='\0';
        printf("%s",daPrintare);
}
