#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define job1 "(1)Killer\n"
#define job2 "(2)Swindler\n"
#define job3 "(3)Noble\n"
#define job4 "(4)Merchant\n"
#define job5 "(5)Warlord\n"

int money;
int hand[2];
int rlgBuild[2];
int cmcBuild[2];
int role;
bool dead;
bool IncomeCardIsRlg;
char SkillResult[20]="+";
char buffer[256];
bool GameOver = false;
int rank = 0;
int score = 0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int KeyPadInput()
{
    int c;
    printf("Input:");
    scanf("%d",&c);
    return c;
}

void ReNewMoney()
{
    printf("\n----- Money:   %d    -----\n\n",money);
}


void ReNewBuild()
{
    printf("----- BuildingS -----\n");
    printf("Religious Building:%d\n",rlgBuild[0]);
    printf("Advenced Religious Building:%d\n",rlgBuild[1]);
    printf("Commercial Building:%d\n",cmcBuild[0]);
    printf("Advenced Commercial Building:%d\n",cmcBuild[1]);
    printf("----- BuildingS -----\n\n");
}


void ReNewLCD(char* mess)
{
    printf(" ----- MessageS -----\n");
    printf("%s\n",mess);
    printf("Religious Card:%d\n",hand[0]);
    printf("Commercial Card:%d\n",hand[1]);
    printf("°----- MessageS -----\n\n");
}

void JobCardPhase(char* data, int sockfd)
{
    bool cards[5]={0,0,0,0,0};
    char Job_buffer[20];
    strcpy(Job_buffer,data);
    char *elem=strtok(Job_buffer,":");
    
    char hint[80]="Pick one Job.\n";

    while(elem!=NULL)
    {
        if(!strcmp(elem,"1"))
            {
                cards[0]=true;
                strcat(hint,job1);
            }
        else if (!strcmp(elem,"2"))
            {
                cards[1]=true;
                strcat(hint,job2);
            }
        else if (!strcmp(elem,"3"))
            {
                cards[2]=true;
                strcat(hint,job3);
            }
        else if (!strcmp(elem,"4")) 
            {
                cards[3]=true;
                strcat(hint,job4);
            }
        else if (!strcmp(elem,"5"))
            {
                cards[4]=true;
                strcat(hint,job5);
            }   
        elem=strtok(NULL,":");
    }

    printf("%s",hint);

    int picked=0;
    while(picked==0)
    {
        int choice;
        printf("choose:");
        scanf("%d",&choice);

        if(choice<1 || choice>5)
            printf("Invaid.\n");
        else
        {
            if(cards[choice-1]) 
                {picked=choice;cards[choice-1]=false;} 
            else
                printf("It's empty.\n");
        }
    }
    role=picked;
    printf("You pick :%d\n\n",picked);
    
    char message[40]="$:";
    int i;
    char bf[5];
    for(i=0;i<5;i++)
    {
        // printf("hi\n");
        if(cards[i])
        {
            sprintf(bf,"%d",i+1);
            strcat(message,bf);
            strcat(message,":");
            // test here!!
            // printf("%s",message);
        }
    }
    strcat(message,"*");
    sprintf(bf,"%d",picked);
    strcat(message,bf);
    
    printf("yo\n");
    // printf("%s",message);

    // write back!
    write(sockfd, message, strlen(message));
}

void Decomposition(char* receiveStatus)
{
    char *element1,*element2,gameoverhint[10]="0";
    int cal1=1,cal2=1;
    
    element1=strtok(receiveStatus,":");
    while (element1 != NULL)
    {
        switch (cal1)
        {
            case 1:
                if (!(strcmp(element1,"@")))
                {
                    //broadcast
                }
                else if(!(strcmp(element1,"!")))
                {

                }
                else
                {
                    strcpy(gameoverhint, element1);
                    GameOver=true;
                }
                break;
            case 2:
                role = atoi(element1);
                break;
            case 3:
                //©–§l
                rlgBuild[0]=(atoi(element1)/1000);
                rlgBuild[1]=(atoi(element1)%1000)/100;
                cmcBuild[0]=(atoi(element1)%100)/10;
                cmcBuild[1]=(atoi(element1)%10);
                break;
            case 4:
                //money
                money = atoi(element1);
                break;
            case 5:
                hand[0] = atoi(element1);
                break;
            case 6:
                hand[1] = atoi(element1);
                break;
            case 7:
                if(!strcmp(element1,"1"))
                    dead = true;
                break;
            case 8:
                if(!strcmp(element1,"1"))
                    IncomeCardIsRlg=true;
                else
                    IncomeCardIsRlg=false;
                break;
        }
        element1 = strtok (NULL, ":");
        cal1++;
    }
    
    if(GameOver)
    {
        element2=strtok(gameoverhint,"*");
        while (element2 != NULL) 
        {
            switch (cal2) 
            {
                case 1:
                    rank = atoi(element2);
                    printf("my rank :%d\n", rank);
                    break;
                case 2:
                    score = atoi(element2);
                    printf("my score :%d\n", score);
                    break;
                default:
                    break;
            }
            element2 = strtok (NULL, "*");
            cal2++;
        }
    }

    ReNewLCD(" ");
    ReNewBuild();
    ReNewMoney();
}

void IncomePhase()
{
    //Choose income:(1) for two gold (2) for one card.
    ReNewLCD("Choose income:\n(1)for 2 gold.\n(2)for 1 card.\n");

    int choice;
    bool done=false;    
    do{
        choice=KeyPadInput();

        if(choice==1)
        {
            money+=2;
            ReNewMoney();
            ReNewLCD("Income 2 gold.\n");
            sleep(2);
            done=true;
        }
        else if(choice==2)
        {
            if((hand[0]+hand[1])<4)
            {
                if(IncomeCardIsRlg)
                {
                    hand[0]+=1;
                    ReNewLCD("Income a religion card.\n");
                    sleep(2);
                }
                else
                {
                    hand[1]+=1;
                    ReNewLCD("Income a commecial card.\n");
                    sleep(2);
                }
                done=true;
            }
            else
            {
                money+=2;
                ReNewLCD("Cannot draw any card more.\nIncome 2 gold.\n");
                ReNewMoney();
                sleep(3);
                done=true;
            }
        }
        else
        {
            ReNewLCD("Please re-check your choice.\nChoose income:\n(1)for 2 gold.\n(2)for 1 card.\n");
        }
    }while(!done);
    printf("yee\n");
}

//®§¶‚ØSÆÌ¶Ê∞ ∂•¨q
void RoleplayPhase()
{
    void KillerAction();
    void SwindlerAction();
    void NobleAction();
    void MerchantAction();
    void WarlordAction();
    
    strcpy(SkillResult,"+");
    switch(role)
    {
    case 1://±˛§‚ killer 
        KillerAction();
        break;
    case 2://≥N§h swindler 
        SwindlerAction();
        break;
    case 3://∂Q±⁄ noble 
        NobleAction();
        break;
    case 4://∞”§H merchant 
        MerchantAction(); 
        break;
    case 5://≠xª÷ warlord 
        WarlordAction();
        break;
    }
    printf("SkillResult is %s\n", SkillResult);
}

void KillerAction()
{
    int rival;
    char bf[5];
    ReNewLCD("Choose who to kill:\n(2,3,4,5)\n(1)for abort\n");
    //LCD®Í∑s≈„•‹hint
    do{
        rival=KeyPadInput();
    }while(rival<1 || rival>5);

    if(rival!=1)
    {
        money+=1;
        ReNewMoney();
        ReNewLCD("Killer guild give you 1 gold for reward.\n");
    }
    else
        ReNewLCD("Peace is good.\n");
    sleep(3);

    //πw≠p¶^∂« 1+•ÿº–+d+0000+0
    strcat(SkillResult,"1+");
    if(rival!=1)//¶≥±˛§H 
    {
        sprintf(bf,"%d",rival);
        strcat(SkillResult,bf);
        strcat(SkillResult,"+d");
    }
    else
    {
        strcat(SkillResult,"nobody");
        strcat(SkillResult,"+0");
    } 
    strcat(SkillResult,"+0000");
    strcat(SkillResult,"+0");
    
}
void SwindlerAction()
{
    int rival;
    char bf[5];
    char hint_s[80]="target:";//±`∫A¥£•‹ 
    char hint_d[200];//∞ ∫A¥£•‹ 
    //ßÛ∑sLCD°AøÔæ‹ 
    strcpy(hint_d,"Choose who to cheat:\n(1,3,4,5)\n(2)for abort\n");
    ReNewLCD(hint_d);
    
    do{
        rival=KeyPadInput(); 
    }while(rival<1 || rival>5);

    //¿R∫A¥£•‹ target:n 
    sprintf(bf,"%d",rival);
    strcat(hint_s,bf);
    strcat(hint_s,"\n");
    
    //•H∞ ∫A¥£•‹ßÛ∑sLCD target:n+øÔæ‹ 
    strcpy(hint_d,hint_s);
    strcat(hint_d,"Choose what you cost:\n(1)Religious Building\n(2)Commercial Building\n");
    ReNewLCD(hint_d);
    
    int cost=0;
    bool pass=false;
    while(!pass && rival!=2)
    {
        cost=KeyPadInput();
        if((rlgBuild[0]+rlgBuild[1]+cmcBuild[0]+cmcBuild[1])==0)
            {cost=0;pass=true;strcat(hint_s,"What A Loser!\n");}
            
        if(cost==1 && rlgBuild[0]>0)
            {pass=true;rlgBuild[0]-=1;}
        else if(cost==1 && rlgBuild[1]>0)
            {pass=true;rlgBuild[1]-=1;}
            
        if(cost==2 && cmcBuild[0]>0)
            {pass=true;cmcBuild[0]-=1;}
        else if(cost==2 && cmcBuild[1]>0)
            {pass=true;cmcBuild[1]-=1;}     
            
        if(!pass)
        {
            strcpy(hint_d,hint_s);
            strcat(hint_d,"\nChooseAnother!\n\ncost:\n(1)Religious Building\n(2)Commercial Building\n");
            ReNewLCD(hint_d);
        }
    }
    
    //¿R∫A¥£•‹ target:n cost:nnnn
    strcat(hint_s,"cost:");
    if(cost==1)
        strcat(hint_s,"rlg Build\n");
    else if (cost==2)
        strcat(hint_s,"cmc Build\n");
    else if (cost==0)
        strcat(hint_s,"none\n");
        
    //∞ ∫A¥£•‹ßÛ∑sLCD  target:n cost:nnnn+øÔæ‹ 
    strcpy(hint_d,hint_s);
    strcat(hint_d,"Choose building you want:\n(1)Religious Build\n(2)Commercial Build\n");
    ReNewLCD(hint_d); 
     
    int target=0;
    while((target<1 || target>2) && rival!=2)
    {
        target=KeyPadInput();   
    }   
    
    //¿R∫A¥£•‹ target:n cost:nnnnnn want:nnnnn 
    strcat(hint_s,"want:");
    if(target==1)
        strcat(hint_s,"rlgBuild\n");
    else if (target==2)
        strcat(hint_s,"cmcBuild\n");
    else if (target==0)
        strcat(hint_s,"none\n");
    ReNewLCD(hint_s);
    ReNewBuild();
    sleep(3);

    //πw≠p¶^∂« 2+•ÿº–+0+´ÿøv¶r∏s+0
    strcat(SkillResult,"2+");
    if(rival!=2)//¶≥¨I≥N™∫™¨™p 
    {
        sprintf(bf,"%d",rival);
        strcat(SkillResult,bf);
    }
    else//©Ò±ÛßﬁØ‡ 
        strcat(SkillResult,"nobody");
        
    strcat(SkillResult,"+0");//±˛§H∂µ•ÿ¨∞0 
    if(rival!=2)//¶≥¨I≥N™∫™¨™p 
    {
        if(target==1)
            strcat(SkillResult,"+1000");
        if(target==2)
            strcat(SkillResult,"+0010");
        if(target==0)
            strcat(SkillResult,"+0000");
    }
    else//©Ò±ÛßﬁØ‡ 
        strcat(SkillResult,"+0000");
    strcat(SkillResult,"+0");//®S¶≥∑m≠∫Æa 
}
void NobleAction()
{   
    char bf[5];
    char hint[80]="Income gold from religion for:";
    // µ|¶¨
    money+=rlgBuild[0]+rlgBuild[1];
    ReNewMoney();
    sprintf(bf,"%d",rlgBuild[0]+rlgBuild[1]);
    strcat(hint,bf);
    strcat(hint,"\nAnd you get crown!!!\n");
    ReNewLCD(hint);
    sleep(3);   
    //∞™Ø≈©v±–´ÿøvπC¿∏µ≤ßÙ´·¶≥√B•~§¿º∆ 

    //πw≠p¶^∂« 3+0+0+0+..... 
    strcat(SkillResult,"3+");
    strcat(SkillResult,"nobody+0");
    strcat(SkillResult,"+0000");
    strcat(SkillResult,"+f");//∑m≠∫Æa∞ ß@ 
}
void MerchantAction()
{   
    char bf[5];
    char hint[80]="Income gold from commerce for:";
    //µ|¶¨
    money+=cmcBuild[0]+cmcBuild[1];
    //∞™Ø≈∞”∑~´ÿøv¿Ú±o¶h®‚≠øµ|¶¨ 
    money+=cmcBuild[1]*2; 
    ReNewMoney();
    sprintf(bf,"%d",cmcBuild[0]+cmcBuild[1]*3);
    strcat(hint,bf);
    ReNewLCD(hint);
    sleep(3);

    //πw≠p¶^∂« 4+0+0+....... 
    strcat(SkillResult,"4+");
    strcat(SkillResult,"nobody+0");
    strcat(SkillResult,"+0000");
    strcat(SkillResult,"+0");

}
void WarlordAction()
{
    int rival;
    char bf[5];
    char hint_s[80]="target:";
    char hint_d[200];   
    strcpy(hint_d,"Choose who to attack:\n(1,2,3,4)\n(5)for abort\n");
    //LCD®Í∑s≈„•‹ 
    ReNewLCD(hint_d);
    
    do{
        rival=KeyPadInput(); 
    }while(rival<1 || rival>5);
    
    //ßÛ∑s¿R∫A¥£•‹ 
    sprintf(bf,"%d",rival);
    strcat(hint_s,bf);
    strcat(hint_s,"\n");
    
    //LCD≈„•‹∞ ∫A¥£•‹+øÔæ‹ 
    strcpy(hint_d,hint_s); 
    strcat(hint_d,"Choose type to strike:\n(1)Religious Build\n(2)Commercial Build\n(else)for abort\n");
    ReNewLCD(hint_d); 
    
    int target=0;
    bool pass=false;    
    while(!pass && rival!=5)
    {
        target=KeyPadInput();   
        if(target==1)
        {
            if(money>=3)
            {
                money-=3;
                pass=true;
            }
            else
            {
                strcpy(hint_d,hint_s);
                strcat(hint_d,"\n Poor money!!\n\nChoose again:\n(1)Religious\n(2)Commercial\n(else)for abort\n");
                ReNewLCD(hint_d);
            }
        } 
        else if(target==2)
        {
            if(money>=4)
            {
                money-=4;
                pass=true;
            }
            else
            {
                strcpy(hint_d,hint_s);
                strcat(hint_d,"\n Poor money!!\n\nChoose again:\n(1)Religious\n(2)Commercial\n(else)for abort\n");
                ReNewLCD(hint_d);           
            }
        } 
        else
        {
            pass=true;
            target=0;
        }
    }
    
    //ßÛ∑s¿R∫A∏Í∞T 
    strcat(hint_s,"tear:");
    if(target==1)
        strcat(hint_s,"rlgBuild\n");
    else if(target==2)
        strcat(hint_s,"cmcBuild\n");
    else
        strcat(hint_s,"Aborted.....\n");
    ReNewLCD(hint_s);
    ReNewMoney();
    sleep(3);

    //πw≠p¶^∂« 5+•ÿº–+0+´ÿøv¶r∏s+0 
    strcat(SkillResult,"5+");
    if(rival!=5 && target!=0)//¶≥©Ó©–§l 
    {
        sprintf(bf,"%d",rival);
        strcat(SkillResult,bf);
    }
    else//©Ò±ÛßﬁØ‡ 
        strcat(SkillResult,"nobody");
    strcat(SkillResult,"+0");//±˛§H∂µ•ÿ¨∞0 
    if(target==1)
        strcat(SkillResult,"+1000");
    else if(target==2)
        strcat(SkillResult,"+0010");
    else
        strcat(SkillResult,"+0000");
    strcat(SkillResult,"+0");

}

void ConstructPhase()
{
    int rlgNum=rlgBuild[0]+rlgBuild[1];
    int cmcNum=cmcBuild[0]+cmcBuild[1];

    char hint[100]="Build a card from hand?\n";
    char rlgAllow[]="(1)Religious\n     by 3 gold\n";
    char cmcAllow[]="(2)Commercial\n     by 4 gold\n";
    char quit[]="(3)quit\n";

    if(rlgNum<4 && money>=3 && hand[0]>0)   
        strcat(hint,rlgAllow);
    if(cmcNum<4 && money>=4 && hand[1]>0)
        strcat(hint,cmcAllow);
    strcat(hint,quit);      

    int choice=0;
    int advChoice=0;
    bool doneFlag=false;
    do{
        ReNewLCD(hint); 
        choice=KeyPadInput();
        switch(choice)
        {
        case 1:
            if(rlgNum<4 && money>=3 && hand[0]>0)
            {
                if(hand[0]>=2 && money>=4)
                {
                    ReNewLCD("Upgarde building with 2 rlgCard & 4 gold?\n(1)confirm\n(else)abort\n");
                    advChoice=KeyPadInput();
                    if(advChoice==1)
                    {
                        money-=4;
                        hand[0]-=2;
                        rlgBuild[1]+=1;
                        doneFlag=true;
                        break;
                    }
                    else
                    {
                        money-=3;
                        hand[0]-=1;
                        rlgBuild[0]+=1;
                        doneFlag=true;
                        break;
                    }
                }
                else
                {
                    money-=3;
                    hand[0]-=1;
                    rlgBuild[0]+=1;
                    doneFlag=true;
                    break;
                }

            }   
            else
            {
                break;
            }
        case 2:
            if(cmcNum<4 && money>=4 && hand[1]>0)
            {
                if(hand[1]>=2 && money>=5)
                {
                    ReNewLCD("Upgarde building with 2 cmcCard & 5 gold?\n(1)confirm\n(else)abort\n");
                    advChoice=KeyPadInput();
                    if(advChoice==1)
                    {
                        money-=5;
                        hand[1]-=2;
                        cmcBuild[1]+=1;
                        doneFlag=true;
                        break;
                    }
                    else
                    {
                        money-=4;
                        hand[1]-=1;
                        cmcBuild[0]+=1;
                        doneFlag=true;
                        break;
                    }
                }
                else
                {
                    money-=4;
                    hand[1]-=1;
                    cmcBuild[0]+=1;
                    doneFlag=true;
                    break;
                }

            }   
            else
            {
                break;
            }
        case 3:
            doneFlag=true;
            break;
        }
    }while(doneFlag!=true);
    
    ReNewMoney();
    ReNewBuild();
    char result[80]="You built ";
    if(advChoice==1)
        strcat(result,"advence ");
    if(choice==1)
        strcat(result,"religion ");
    if(choice==2)
        strcat(result,"commercial ");

    if(choice==3)
        strcat(result,"Nothing.");
    else
        strcat(result," Building.\n");
    ReNewLCD(result);
    sleep(3);
    printf("not abort trap\n");
}

void DeadRolePhase()
{
    strcpy(SkillResult,"+");
    char bf[5];
    sprintf(bf,"%d",role);
    strcat(SkillResult, bf);
    strcat(SkillResult, "+nobody+0+0000+0");
}

void Reporting(int sockfd)
{
    dead=false; 
    char status[80]="#:";
    char bf[5];
    
    sprintf(bf,"%d",money);
    strcat(status,bf);
    printf("$:%d\n",money);
    strcat(status,":");
    sprintf(bf,"%d",hand[0]);
    strcat(status,bf);
    printf("h:%d\n",hand[0]);
    strcat(status,":");
    sprintf(bf,"%d",hand[1]);
    strcat(status,bf);
    printf("h2:%d\n",hand[1]);
    strcat(status,":");
    sprintf(bf,"%d",rlgBuild[0]);
    strcat(status,bf);
    printf("r0:%d\n",rlgBuild[0]);
    sprintf(bf,"%d",rlgBuild[1]);
    strcat(status,bf);
    printf("r1:%d\n",rlgBuild[1]);
    sprintf(bf,"%d",cmcBuild[0]);
    strcat(status,bf);
    printf("c0:%d\n",cmcBuild[0]);
    sprintf(bf,"%d",cmcBuild[1]);
    printf("c1:%d\n",cmcBuild[1]);
    strcat(status,bf);  
    strcat(status,":");
    printf("test if SkillResult fail\n");
    strcat(status,SkillResult); 

    printf("SkillResult is:%s\n", SkillResult);
    printf("status is:%s\n", status);
    write(sockfd, status, strlen(status));
    printf("Reporting finish!\n");
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, k;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 8888;
    /* Create Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    /* Catch Server IP */
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    while(!GameOver)
    {
        int read_size =recv(sockfd , buffer , 256 , 0);
        if (read_size>1) 
        {
            // show hint in client..
            printf("%s\n", buffer);

            // extract the first one
            char p = buffer[0];
            // printf("p is:%c\n", p);
            if (p =='$') 
            {
                // printf("FIRST HINT\n");
                JobCardPhase(buffer, sockfd);
            }
            else if (p =='!')
            {
                Decomposition(buffer);
                if(!dead)
                {
                    IncomePhase();
                    RoleplayPhase();
                    ConstructPhase();
                }
                else 
                {
                    DeadRolePhase();
                }
                Reporting(sockfd);
                sleep(1);
            }
            else if (p=='@')
            {
                printf("broadcasting....\n");
                Decomposition(buffer); 
            } 
            else
            {
                printf("...maybe GameOver or garbage...\n");
                Decomposition(buffer);
                printf("I win, my rank is:%d with %d point.\n", rank,score);
            } 

            bzero(buffer,256);
        }
        else ;
    }

    printf("rank:%d \n", rank);

    //GameOver
    char overmess[100]="Game Over!\nYour RanK:";
    char bf[5];
    sprintf(bf,"%d",rank);
    strcat(overmess,bf);
    strcat(overmess,"\nScore:");
    sprintf(bf,"%d",score);
    strcat(overmess,bf);
    strcat(overmess,"\n");
    ReNewLCD(overmess); 

    return 0;
}
