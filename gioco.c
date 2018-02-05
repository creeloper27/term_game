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

	#define OS 0

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
            close(1);
            close(2);
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

    #define OS 1

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

}entity_other;

//struct players contains just needed infos
typedef struct{
    int pid;                //player ID
    int status;             //0=death   1=alive
    int h;
    int b;
    int dead_time;          //in ms, time when the player died
    char name[15];          //player name
    char ascii;             //ascii character for the player
    char asciidead;         //ascii character for the dead player
    char ascii_projectile;  //ascii of the projectile
    int direction;          //which way is he facing? 0-top 1-right 2-bottom 3-left
    int k;                  //kills
    int d;                  //deaths
    char top;               //go-top key
    char bottom;            //go-bottom key
    char left;              //go-left key
    char right;             //go-right key
    char fire;              //fire key
    int is;                 //is it active?
}entity_player;

//struct players contains just needed infos
typedef struct{
    int pid;        //player ID
    int h;
    int b;
    char ascii;     //ascii character
    int dim;        //size on projectile
    int direction;  //direction
    int speed;      //speed
    int is;         //is it active?
}entity_projectile;

//matrix "world" --> contains the background
#define HEIGHT 35
#define WIDTH 88

//array "entity_other" --> contains entities that aren't projectiles or players
#define OTHER_ENTITY_MAX 10

//array "player" --> contains the players
#define PLAYER_MAX 10

//array "projectile" --> contains the projectiles
#define PROJECTILE_MAX 10

//global variables
int points1=0;
int points2=0;
int player1=88;
int player2=79;
int islog=0;
int isrender=1;
int other_number=0;
int player_number=2;
int projectile_number=0;
int setting_respawn=1;          //0=not respawn 1=respawn
int setting_respawn_mode=0;     //0=random  1=spawns
int setting_respawn_delay=2000; //delay in ms before respawning
char stemp[500];
FILE *logfile;
FILE *logofile;
entity_other other[OTHER_ENTITY_MAX];
entity_player player[PLAYER_MAX];
entity_projectile projectile[PROJECTILE_MAX];


//prototypes
void clear(){system(CLEAR);}
void Render(int debug,int fps,int fps_time,int delay, char world[HEIGHT][WIDTH], entity_other other[], int other_number, entity_player player[], int player_number, entity_projectile projectile[], int projectile_number);
void winizializza(char x, char world[HEIGHT][WIDTH]);
void menu(int game_settings[], int *exit);
int create_menu(char file[], int menu_width, int menu_height, int menu_slots);
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
void hitreg();
void respawn_dead_players();
void sort_projectiles();
void atp(int *cont, char toPrint[], char c[]);
int character(int ch, int phase, int selected);
int play(int s_m, int l_o, int gamemode);

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

//I use extended ascii characters so in character() i return the extended ascii character for windows and another character for linux
int character(int ch, int phase, int selected){
    //▒ 177 //ESC 27  //# 35 //< 60 //> 62  //░176
    switch(ch){
    case 48 ... 57:   //0
        if(ch==selected){
            if(phase)
                return 60;  //<
            else
                return 62;  //>
        }else{
            if(OS)
                return 176; //░
            return 35;      //#
        }
    case ']':   //]
        if(OS)
            return 177;     //▒
        return 35;          //#
    }
    return ch;
}

void menu(int game_settings[], int *exit){
    int r;
    //s_m           0-single_player         1-multi_player
    //l_o           0-local(same computer)  1-online
    //gamemode      0-deathmatch            1-elimination
    while(*exit==0){
        r=create_menu("menu.txt",33,28,3);
        switch(r){
        case 1:     //m_exit==1
            *exit=1;
            break;
        case 48:    //0
            r=create_menu("menu_gamemode.txt",33,25,3);
            break;
        case 49:    //1
            r=create_menu("menu_multi.txt",33,25,3);
            if(r==50)
                break;
            else if(r==48){
                r=create_menu("menu_gamemode.txt",33,25,3);
                if(r==48)
                    game_settings[2]=0;
                else if(r==49)
                    game_settings[2]=1;
            }
            else if(r==49){
                //online
            }
            break;
        case 50:    //2
            r=create_menu("menu_options.txt",33,25,1);
        }
    }
}
//main menu
int create_menu(char file[], int menu_width, int menu_height, int menu_slots){
    char toPrint2[100000];
    char string_menu[100000];
    int cont=0, cont2=0, i=0, select=0;
    int ch,m_exit=0,phase=0,selected=48;
    int offset_top=((WIDTH-33)/2-(33/2))/2;

    //                                 33 spazzi
    //66 di larghezza totale


    FILE *menufile;
    menufile = fopen(file, "r+");
    if(menufile){
        while ((ch = fgetc(menufile)) != EOF){
            string_menu[cont2]=ch;
            cont2++;
            if(ch=='\n'){
                for(i=0;i<(WIDTH-menu_width)/2-(menu_width/2);i++){
                string_menu[cont2]=' ';
                cont2++;
                }
            }
        }
        fclose(menufile);
    }
    //usare %c,7 per suoni menu

    while(m_exit==0){
        if(kbhit()){
            //take the key from the buffer and put it in "ch"
            ch = getcharacter;
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
                case 13:
                    select=1;
                    break;
            }
            if(selected<48)
                selected=48+menu_slots-1;
            if(selected>48+menu_slots-1)
                selected=48;

            if(select)
                return selected;
        }

        for(i=0;i<offset_top;i++){
            toPrint2[cont]='\n';
            cont++;
        }
        for(i=0;i<(WIDTH-menu_width)/2-(menu_width/2);i++){
            toPrint2[cont]=' ';
            cont++;
        }
        for(i=0;i<cont2;i++){
            toPrint2[cont]=character(string_menu[i],phase%2,selected);    //I use extended ascii characters so in character() i return the extended ascii character for windows and another character for linux
            cont++;
        }

        for(i=0;i<HEIGHT-offset_top-menu_height;i++){
            toPrint2[cont]='\n';
            cont++;
        }

        printf("%s",toPrint2);
        cont=0;
        phase++;
        if(phase>=100)
            phase=0;

        msleep(16);
    }

    return m_exit;
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
            if(player[i].status==1){
                if(checkmovement(ch, i)){
                    flag=0;    //exit if there is a mach
                }
            }
        }
    }
    physics();
    hitreg();
    if(setting_respawn)
        respawn_dead_players();
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

    projectile[projectile_number-1].pid=player[p].pid;
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

//hits registration
void hitreg(){
    int i,i2;

    for(i=0;i<player_number;i++){
        for(i2=0;i2<projectile_number;i2++){
            if(player[i].h==projectile[i2].h&&player[i].b==projectile[i2].b){
                if(player[i].status==1){
                    player[i].status=0;
                    player[i].dead_time=clock();
                    player[i].d++;
                    player[projectile[i2].pid].k++;
                }
                projectile[i2].is=0;
            }
        }
    }
}

//respawn dead players
void respawn_dead_players(){
    int i;

    for(i=0;i<player_number;i++){
        if(player[i].status==0&&clock()-player[i].dead_time>=setting_respawn_delay){
            srand(time(NULL)+clock());
            player[i].h=(rand()*2)%(HEIGHT-2);
            srand(time(NULL)+clock());
            player[i].b=(rand()*2)%(WIDTH-2);
            player[i].status=1;
        }
    }

}

//MAIN
int main(int argc, char *argv[]){
    int term_h,term_b;
    int game_settings[3];
    int exit=0;

    argc--; //by default it starts from 1 (0 is the program name) , by decresing it by 1 it starts from 0
    printf("\narguments: %d",argc);



    //resize window
    if(argc>=2){
        printf("\n\nargument_1: %s\nargument_2: %s\n\n",argv[1],argv[2]);
    }

    if(argc==1&&!strcmp(argv[1],"--help")){
        printf("\nterm_game [terminal height] [terminal base]\n");
        return 0;
    }else if(argc==2){
        sscanf(argv[1], "%d", &term_h);
        sscanf(argv[2], "%d", &term_b);
        resize(term_h,term_b);
    }else{
        resize(HEIGHT+8,WIDTH+1);
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

    //menu for setting up players, controls etc
    //to asign strings, use strcpy() because you can't directly asign
    while(exit==0){
        menu(game_settings,&exit);

        player[0].pid=0;
        player[0].status=1;                 //0=dead    1=alive
        player[0].is=1;                     //1=stay in the array   0=needs to be deleted
        strcpy(player[0].name,"player 1");
        player[0].ascii=88;
        player[0].asciidead=206;
        player[0].h=10;
        player[0].b=10;
        player[0].ascii_projectile=43;
        player[0].fire='e';
        player[0].top='w';
        player[0].bottom='s';
        player[0].right='d';
        player[0].left='a';

        player[1].pid=1;
        player[1].status=1;
        player[1].is=1;
        strcpy(player[1].name,"player 2");
        player[1].ascii=79;
        player[1].asciidead=206;
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

        play(game_settings[0],game_settings[1],game_settings[2]);
    }
    if(islog)
        fclose(logfile);
    return 0;
}

int play(int s_m, int l_o, int gamemode){
    //s_m           0-single_player         1-multi_player
    //l_o           0-local(same computer)  1-online
    //gamemode      0-deathmatch            1-elimination
    int isPlaying=1;
    char world[HEIGHT][WIDTH];
    int delay=16;
    int Cstart,i=0,fps=0,fps_time;


    if(s_m==1&&l_o==0){    //multiplayer
        //inizializza background
        if(gamemode==0){
            setting_respawn=1;
            setting_respawn_mode=0;     //0=random  1=spawns
        }else if(gamemode==1){
            setting_respawn=0;
        }else{
            return -1;
        }
        winizializza(' ', world);
        i=0;
        while(isPlaying){

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

                //render the frame
                Render(1,fps ,fps_time ,delay, world, other, other_number, player, player_number, projectile, projectile_number);

                //sleep 13 ms to try and get 60fps
                msleep(delay);
        }
    }
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
        sprintf(temp,"clock: %d\nfps: %d\nfps_time: %d delay: %d\nPOINTS: use percentS and generate it outside according to the players number\nprojectile_number: %d\nplayer_number: %d\nother_number: %d\n",clock(),fps,fps_time,delay,projectile_number,player_number,other_number);
        atp(&cont,toPrint,temp);
        sprintf(temp,"\nP0 K=%d D=%d d_t=%d\tP1 K=%d D=%d d_t=%d\n",player[0].k,player[0].d,player[0].dead_time,player[1].k,player[1].d,player[1].dead_time);
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
                    if(player[i3].status==1){
                        toPrint[cont]=player[i3].ascii;
                        cont++;
                    }else if(player[i3].status==0){
                        toPrint[cont]=player[i3].asciidead;
                        cont++;
                    }

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
