#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "term_menu.h"

//compatibility for Unix systems
#if defined(__unix__)||defined(__APPLE__)

	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>

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

//compatibility for Windows systems
#elif defined(_WIN32)||defined(_WIN64)

    #include <Windows.h>

    #define OS 1

	//to create a message box
    void MsgBox(char *contenuto, char *finestra, int tipo){
        MessageBox(0, contenuto, finestra, tipo);
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
    int health;             //0 -> 100
    int h;
    int b;
    int dead_time;          //in ms, time when the player died
    char name[15];          //player name
    char ascii;             //ascii character for the player
    char asciidead;         //ascii character for the dead player
    int direction;          //which way is he facing? 0-top 1-right 2-bottom 3-left
    int k;                  //kills
    int d;                  //deaths
    char top;               //go-top key
    char bottom;            //go-bottom key
    char left;              //go-left key
    char right;             //go-right key
    char fire;              //fire key
    char change_weapon;
    int weapon;
    int fire_time;
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
    int damage;
    int is;         //is it active?
}entity_projectile;

//struct for weapons
typedef struct{
    int firerate;
    int damage;
    char bullet;
    int speed;
}item_weapon;

//matrix "world" --> contains the background
#define HEIGHT 30
#define WIDTH 120

#define DEBUG_HEIGHT 11

//array "entity_other" --> contains entities that aren't projectiles or players
#define OTHER_ENTITY_MAX 10

//array "player" --> contains the players
#define PLAYER_MAX 10

//array "projectile" --> contains the projectiles
#define PROJECTILE_MAX 20

//weapon number
#define WEAPON_NUMBER 2

//global variables
int islog=0;
int isrender=1;
int other_number=0;
int player_number=2;
int projectile_number=0;
int setting_respawn=1;          //0=not respawn 1=respawn
int setting_respawn_mode=0;     //0=random  1=spawns
int setting_respawn_delay=2000; //delay in ms before respawning
char stemp[500];
int selected_player=1;
FILE *logfile;
FILE *logofile;
entity_other other[OTHER_ENTITY_MAX];
entity_player player[PLAYER_MAX];
entity_projectile projectile[PROJECTILE_MAX];
item_weapon weapon[WEAPON_NUMBER];


//prototypes
void Render(int debug,int fps,int fps_time,int delay, char world[HEIGHT][WIDTH], entity_other other[], int other_number, entity_player player[], int player_number, entity_projectile projectile[], int projectile_number);
void winizializza(char x, char world[HEIGHT][WIDTH]);
int menu(int game_settings[], int *exit);
int getday();
int getmonth();
int getyear();
int gethour();
int getmin();
int getsec();
void MsgBoxv(char mex[1024],char dato,char nome[1024],int tipo);
void printlog(char a[]);
void tick(int *isPlaying);
int checkaction(char ch, int p);
void fire(int p);
void physics();
void hitreg();
void respawn_dead_players();
void sort_projectiles();
void atp(int *cont, char toPrint[], char c[]);
int playg(int game_settings[]);
void player_add();
void player_remove(int player);
void player_edit(int selected_player);
void sread(char *string);

void player_add(){
    player_number++;
    player[player_number-1].pid=player_number-1;
    player[player_number-1].health=100;
    player[player_number-1].is=1;
    strcpy(player[player_number-1].name,"player");
    player[player_number-1].ascii='H';
    player[player_number-1].asciidead=206;
    player[player_number-1].h=5;
    player[player_number-1].b=6;
    player[player_number-1].fire='y';
    player[player_number-1].top='t';
    player[player_number-1].bottom='g';
    player[player_number-1].right='h';
    player[player_number-1].left='f';
    player[player_number-1].change_weapon='r';
    player[player_number-1].weapon=0;
}

void player_edit(int selected_player){
    //needs to be converted into a decent menu

    int r=1;

    while(r!=0){
        tclear();
        printf("CHARACTER: \n");
        printf("1 - name: %s\n",player[selected_player].name);
        printf("2 - ascii character(alive): %c\n",player[selected_player].ascii);
        printf("3 - ascii character(dead): %c\n",player[selected_player].asciidead);
        printf("\nCONTROLS: \n");
        printf("4 - top: %c\n",player[selected_player].top);
        printf("5 - bottom: %c\n",player[selected_player].bottom);
        printf("6 - left: %c\n",player[selected_player].left);
        printf("7 - right: %c\n",player[selected_player].right);
        printf("8 - fire: %c\n",player[selected_player].fire);
        printf("9 - change_weapon: %c \n",player[selected_player].change_weapon);
        printf("\n0 - back\n");

        do{
            printf("\n>>edit: ");
            scanf("%d",&r);
        }while(r<0||r>9);

        switch(r){
        case 0:
            break;
        case 1:
            printf(">>new name: ");
            sread(player[selected_player].name);
            break;
        case 2:
            printf(">>new ascii character(alive): ");
            scanf(" %c",&player[selected_player].ascii);
            break;
        case 3:
            printf(">>new ascii character(dead): ");
            scanf(" %c",&player[selected_player].asciidead);
            break;
        case 4:
            printf(">>new top: ");
            scanf(" %c",&player[selected_player].top);
            break;
        case 5:
            printf(">>new bottom: ");
            scanf(" %c",&player[selected_player].bottom);
            break;
        case 6:
            printf(">>new left: ");
            scanf(" %c",&player[selected_player].left);
            break;
        case 7:
            printf(">>new right: ");
            scanf(" %c",&player[selected_player].right);
            break;
        case 8:
            printf(">>new fire: ");
            scanf(" %c",&player[selected_player].fire);
            break;
        case 9:
            printf(">>new change_weapon: ");
            scanf(" %c",&player[selected_player].change_weapon);
            break;
        }
    }

}

void sread(char *string){
    int i=0;
    char c;
    while(c!='\n'&&c!=13&&i<20){
        c=getch();
        putchar(c);
        string[i]=c;
        i++;
    }
    string[i+1]='\0';
}

void player_remove(int s_player){
    int i;
    for(i=s_player;i<player_number-1;i++){
        player[i]=player[i+1];          //possibile errore
    }
    player_number--;
}

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

int menu(int game_settings[], int *exit){
    int r;
    //s_m           0-single_player         1-multi_player
    //l_o           0-local(same computer)  1-online
    //gamemode      0-deathmatch            1-elimination

    while(r=create_menu("menu.txt",66,31,4,176,178,177,'<','>')){
        switch(r){

        case 1:
            game_settings[0]=0;
            while(r=create_menu("menu_gamemode.txt",66,31,3,176,178,177,'<','>')){
                create_menu("not_available.txt",66,31,1,176,178,177,'<','>');
            }
            break;

        case 2:
            game_settings[0]=1;
            while(r=create_menu("menu_multi.txt",66,31,3,176,178,177,'<','>')){
                switch(r){

                case 1:
                    game_settings[1]=0;
                    while(r=create_menu("menu_gamemode.txt",66,31,3,176,178,177,'<','>')){
                        switch(r){

                        case 1:
                            game_settings[2]=0;
                            return 1;
                            break;
                        case 2:
                            game_settings[2]=1;
                            return 1;
                            break;
                        }
                    }
                    break;
                case 2:
                    game_settings[1]=1;
                    create_menu("not_available.txt",66,31,1,176,178,177,'<','>');

                }
            }
            break;
        case 3:
            while(r=create_menu("menu_options.txt",66,31,2,176,178,177,'<','>')){
                switch(r){
                case 1:
                    while(r=create_advmenu("menu_players.txt",&selected_player,player_number,66,31,4,176,178,177,'<','>')){
                        switch(r){
                        case 1:
                            player_add();
                            break;
                        case 2:
                            player_edit(selected_player-1);
                            break;
                        case 3:
                            if(selected_player<player_number)
                                player_remove(selected_player-1);
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    if(r==0)
        *exit=1;
    return 0;
}
    //s_m           0-single_player         1-multi_player
    //l_o           0-local(same computer)  1-online
    //gamemode      0-deathmatch            1-elimination

    //printf("\ng_s[0]=%d\ng_s[1]=%d\ng_s[2]=%d\n",game_settings[0],game_settings[1],game_settings[2]);
    //system("pause");

//tick
void tick(int *isPlaying){
    char ch;
    int i,flag=1;
    //check for controls matches (if keys have been pressed)
    if(kbhit()){
        //take the key from the buffer and put it in "ch"
        ch = getcharacter;
        if(ch==27)
            *isPlaying=0;
        for(i=0;i<player_number&&flag;i++){
            if(player[i].health>0){
                if(checkaction(ch, i)){
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
int checkaction(char ch, int p){
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
        if(projectile_number<PROJECTILE_MAX&&clock()-player[p].fire_time>=weapon[player[p].weapon].firerate){
            player[p].fire_time=clock();
            fire(p);
        }
        return 1;
    }else if(ch==player[p].change_weapon){
        if(projectile_number<10)
            if(player[p].weapon<WEAPON_NUMBER-1)
                player[p].weapon++;
            else
                player[p].weapon=0;
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
    projectile[projectile_number-1].ascii=weapon[player[p].weapon].bullet;
    projectile[projectile_number-1].direction=player[p].direction;
    projectile[projectile_number-1].speed=weapon[player[p].weapon].speed;
    projectile[projectile_number-1].damage=weapon[player[p].weapon].damage;
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
                projectile[i].h-=projectile[i].speed;
                break;
            case 1:
                projectile[i].b+=projectile[i].speed*2;
                break;
            case 2:
                projectile[i].h+=projectile[i].speed;
                break;
            case 3:
                projectile[i].b-=projectile[i].speed*2;
                break;
        }
    }
}

//delete the projectiles with is=0 from the array
void sort_projectiles(){
    int sortered=1,i=0,i2=0,flag=1;

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
                if(player[i].health>0){
                    player[i].health-=projectile[i2].damage;
                    if(player[i].health<=0){
                        player[i].dead_time=clock();
                        player[i].d++;
                        player[projectile[i2].pid].k++;
                    }
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
        if(player[i].health<=0&&clock()-player[i].dead_time>=setting_respawn_delay){
            srand(time(NULL)+clock());
            player[i].h=(rand()*2)%(HEIGHT-2);
            srand(time(NULL)+clock());
            player[i].b=(rand()*2)%(WIDTH-2);
            player[i].health=100;
        }
    }

}

//MAIN
int main(int argc, char *argv[]){
    int term_h,term_b;
    int game_settings[3];
    int exit=0,play=0;

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
        resize(HEIGHT+DEBUG_HEIGHT,WIDTH+1);
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

    weapon[0].firerate=100;
    weapon[0].damage=20;
    weapon[0].speed=1;
    weapon[0].bullet='*';

    weapon[1].firerate=1500;
    weapon[1].damage=90;
    weapon[1].speed=3;
    weapon[1].bullet='-';

    player[0].pid=0;
    player[0].health=100;                 //0=dead    1=alive
    player[0].is=1;                     //1=stay in the array   0=needs to be deleted
    strcpy(player[0].name,"player 1");
    player[0].ascii=88;
    player[0].asciidead=206;
    player[0].h=10;
    player[0].b=10;
    player[0].fire='e';
    player[0].top='w';
    player[0].bottom='s';
    player[0].right='d';
    player[0].left='a';
    player[0].change_weapon='q';
    player[0].weapon=0;

    player[1].pid=1;
    player[1].health=100;
    player[1].is=1;
    strcpy(player[1].name,"player 2");
    player[1].ascii=79;
    player[1].asciidead=206;
    player[1].h=HEIGHT-2;
    player[1].b=WIDTH-4;
    player[1].fire='o';
    player[1].top='i';
    player[1].bottom='k';
    player[1].right='l';
    player[1].left='j';
    player[1].change_weapon='u';
    player[1].weapon=0;

    //menu for setting up players, controls etc
    //to asign strings, use strcpy() because you can't directly asign
    while(exit==0){
        play=menu(game_settings,&exit);
        //in range of the speed based on the player coordinates <- HITREG TO CACH FAST BULLETS

        //entity conterr� entit� che non sono ne giocatori ne proiettili, verr� utilizzato nel futuro
        //every time a game starts verryte entity and projectiles arrays and keep player array(just reset positions)
        //every time a player shoots create a struct on the projectile array
        //create function moveleft() moveright() fire().... etc
        //printf("\ng_s[0]=%d\ng_s[1]=%d\ng_s[2]=%d\n",game_settings[0],game_settings[1],game_settings[2]);
        //system("pause");
        if(play)
            playg(game_settings);
    }
    if(islog)
        fclose(logfile);
    return 0;
}

int playg(int game_settings[]){
    //0           0-single_player         1-multi_player
    //1           0-local(same computer)  1-online
    //2           0-deathmatch            1-elimination

    int isPlaying=1;
    char world[HEIGHT][WIDTH];
    int delay=16;
    int Cstart,i=0,fps=0,fps_time;
    resize(HEIGHT+DEBUG_HEIGHT,WIDTH+1);


    if(game_settings[0]==1&&game_settings[1]==0){    //multiplayer
        //inizializza background
        if(game_settings[2]==0){
            setting_respawn=1;
            setting_respawn_mode=0;     //0=random  1=spawns
        }else if(game_settings[2]==1){
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

                tick(&isPlaying);

                //render the frame
                Render(1,fps ,fps_time ,delay, world, other, other_number, player, player_number, projectile, projectile_number);

                //sleep 13 ms to try and get 60fps
                tsleep(delay);
        }
    }
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
    char temp[500];
    int i,i2,i3,cont=0;
    int background=0;

    //generate debug infos in the frame
    if(debug){
        //points: use percentS and generate it outside according to the players number
        sprintf(temp,"clock: %d\nfps: %d\nfps_time: %d delay: %d\n\nprojectile_number: %d\nplayer_number: %d\nother_number: %d\n",(int)clock(),fps,fps_time,delay,projectile_number,player_number,other_number);
        atp(&cont,toPrint,temp);
        sprintf(temp,"\nP-1 K=%d D=%d d_t=%d HP=%d\nP-2 K=%d D=%d d_t=%d HP=%d\nP-3 K=%d D=%d d_t=%d HP=%d\nP-4 K=%d D=%d d_t=%d HP=%d\n",player[0].k,player[0].d,player[0].dead_time,player[0].health,player[1].k,player[1].d,player[1].dead_time,player[1].health,player[2].k,player[2].d,player[2].dead_time,player[2].health,player[3].k,player[3].d,player[3].dead_time,player[3].health);
        atp(&cont,toPrint,temp);
        }
    //generate the frame and put it in toPrint
    for(i=0;i<HEIGHT;i++){
        for(i2=0;i2<WIDTH;i2++){
            background=1;

            //check the 3 arrays other player projectile

            //players
            for(i3=0;i3<player_number&&background;i3++){
                if(player[i3].h==i&&player[i3].b==i2){
                    if(player[i3].health>0){
                        toPrint[cont]=player[i3].ascii;
                        cont++;
                    }else if(player[i3].health<=0){
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


/*
struttura
    dentro tick() si controlla se ci sono proiettili e se il tasto premuto fa qualcosa, usare funzioni per muovere il player
    movetop(player number) -> controlla se pu� muversi in quella direzione o no
*/
