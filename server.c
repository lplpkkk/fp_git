#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <stdbool.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#define N 3 // player numbers
#define CardUpperLimit 10;
 
// Game setting 
struct player{
    int job;
    int building[4];
    int money;
    int rCard;
    int bCard;
    bool death;
    int firstOrder;
    int socketfd;
    int ID;
    int score;
    int rank;
    bool drawCard;
};
struct player playerlist[N];
struct player* firstOrderArray[N]={NULL};
struct player* secondOrderArray[N]={NULL};
int i,j,temp;
bool gameOver=0;
int card[5]; //card[0]盜賊 card[1]術士 card[2]貴族 card[3]商人 card[4]軍人
char sendCard[256]={'\0'};
char sendStaus[256]={'\0'};
char receiveCard[256],*temp_str,receiveStatus[256];
char str[4];
char winnerMessage[256]={"WINNER!"};
int tempOrder[3];
int cal;
unsigned seed = 0;
int tempMoney;
int count[N]={0};
int n = 1000; 
int flag_phase1 = 1;
pthread_t sniffer_thread[N];
char message[256] , client_message[2000], client_message2[2000];
int PHASE=0; 
int player_index=0;
int whoesTurn;
int client_sock;

void *test(void *);
void error(char *msg)
{
    perror(msg);
    exit(1);
}
void judgeBuildingAmount(int whoesTurn){
    int buildingAmount=0;
    int j,i;
    int tempScore[3];
    int bi,bj;
    int temp;
    
    for (j=0; j<4; j++) {    //j->第幾種建築
        buildingAmount = buildingAmount + secondOrderArray[whoesTurn]->building[j];
    }
    if (buildingAmount >= 5) {
        gameOver=1;
        secondOrderArray[whoesTurn]->score+=5;
        for (i=0 ; i<N; i++) {  //計算每個人的分數
            playerlist[i].score += playerlist[i].building[0]*3+playerlist[i].building[1]*5+playerlist[i].building[2]*4+playerlist[i].building[3]*4;
            playerlist[i].score += playerlist[i].money/3;
            tempScore[i]=playerlist[i].score;
        }
        //bubble sort
        for (bi = 0; bi < N; bi++)
        {
            for (bj = 0; bj < N; bj++)
            {
                if (tempScore[bj]<tempScore[bi])
                {
                    temp = tempScore[bi];
                    tempScore[bi]=tempScore[bj];
                    tempScore[bj]=temp;
                }
            }
        }
        for (i = 0; i < N; i++)
        {
            for (j = 0; j < N; j++)
            {
                if (playerlist[j].score==tempScore[i])
                {
                    playerlist[j].rank=i+1;
                }
            }
        }
        // printf("score of %d is %d\n", whoesTurn, secondOrderArray[whoesTurn]->score);
    }
}
void FirstOrderToChooseJob(int whoesTurn){
    int j;
    char str[4];
    
    if (whoesTurn==0) {
        for (j=0 ; j<5; j++)    card[j]=1;   //卡片initialization
    }
    bzero(sendCard,256);
    printf("***%s***\n",sendCard);
    /* client端串上job
     strcat(sendCard,"*")
     char str[2];
     sprintf(str, sizeof(str), "%d", 工作);
     strcat(sendCard,str)
     strcat(sendCard,":" ); */
    for (j=0; j<5; j++) { //卡片字串
        if (card[j]==1) {
            snprintf(str, sizeof(str), "%d", j+1);
            strcat(sendCard,str); //傳sendCard給玩家（ex-1:2:4:->盜賊術士商人還在 回傳 *4:1:2: 盜賊術士還在 該玩家為商人
            strcat(sendCard,":" );
        }
    }
    printf("%d player sendCard:%s \n",whoesTurn,sendCard);
    // bzero(sendCard, 256);
}
void FirstOrderToModifyTheCard(int whoesTurn){
    int j;
    char *temp_str;
    for (j=0 ; j<5; j++)    card[j]=0;
    printf("client_message: %s\n", client_message);
    temp_str=strtok(client_message,":");
    while (temp_str != NULL)
    {
        if (!strcmp(temp_str,"1")){
            card[0] = 1;
        }else if(!strcmp(temp_str,"2")){
            card[1] = 1;
        }else if(!strcmp(temp_str,"3")){
            card[2] = 1;
        }else if(!strcmp(temp_str,"4")){
            card[3] = 1;
        }else if(!strcmp(temp_str,"5")){
            card[4] = 1;
        }else if(!strcmp(temp_str,"*1")){
            firstOrderArray[whoesTurn]->job = 1;
        }else if(!strcmp(temp_str,"*2")){
            firstOrderArray[whoesTurn]->job = 2;
        }else if(!strcmp(temp_str,"*3")){
            firstOrderArray[whoesTurn]->job=3;
        }else if(!strcmp(temp_str,"*4")){
            firstOrderArray[whoesTurn]->job=4;
        }else if(!strcmp(temp_str,"*5")){
            firstOrderArray[whoesTurn]->job=5;
        }
        // printf("!!!%s\n", temp_str);
        temp_str = strtok (NULL, ":");
    }
    printf("*** %d player job:%d ***\n",whoesTurn,firstOrderArray[whoesTurn]->job);
    // printf("*** %d player job ***",whoesTurn);
    // bzero(client_message, 256);
}
void DecideTheFirstOrder(){
    int i,j;
    for (i=0 ; i < N; i++) { //firstOrder
        for (j=0; j < N ;j++) { //從三人中找到相合的firstOrder
            if (playerlist[j].firstOrder == i) {
                firstOrderArray[i]=&playerlist[j];
                break;
            }
        }
    }
}
void DecideTheSecondOrder(){
    int i,j,temp;
    int tempOrder[N];
    
    for (i=0; i<N; i++) tempOrder[i]=playerlist[i].job;
    printf("tempOrder: \n");
    
    for (i=0; i<N; i++) {
        printf("%d",tempOrder[i]);
    }
    printf("\n");
    //bouble sort
    for(i=0;i<N;i++){
        for(j=i;j<N;j++){
            if(tempOrder[j]<tempOrder[i]){
                temp = tempOrder[i];
                tempOrder[i] = tempOrder[j];
                tempOrder[j] = temp;
            }
        }
    }
    //printf result
    printf("tempOrder AFTER sort: \n");
    for (i=0; i<N; i++) {
        printf("%d",tempOrder[i]);
    }
    printf("\n");
    //sort the secondOrder
    for (i=0; i<N; i++) {   //secondOrder 低到高
        for (j=0; j<N; j++) {   //over all playerlist[j] to find the tempOder[i]
            if (playerlist[j].job == tempOrder[i]) {
                secondOrderArray[i]=&playerlist[j];
                break;
            }
        }
    }
}

void changeTheFirstOrder(){
    int i;
    for (i=0; i<N; i++) {
        playerlist[i].firstOrder = (playerlist[i].firstOrder+N-1)%3;
    }
}
void Attack(int whoesTurn){
    int cal=1,cal2=1;  //cal-第幾個字 cal2-第幾個攻擊項目
    char* elem=strtok(client_message,":");
    char input2[20];
    char* elem2;
    int target = 0; //playerlist的index 指向攻擊的目標
    int tempBuilding=0;
    
    while(elem!=NULL)
    {
        if(!strcmp(elem,"#"))
            printf("提示，現在正在做狀態技能回報，其實也不用特別做什麼事\n");
        if(cal==2)
            secondOrderArray[whoesTurn]->money = atoi(elem);
        if(cal==3)
            secondOrderArray[whoesTurn]->rCard = atoi(elem);//剩餘宗教手牌
        if(cal==4)
            secondOrderArray[whoesTurn]->bCard = atoi(elem);//剩餘商業手牌
        if(cal==5)
        {
            printf("Buildings:%s\n",elem);
            secondOrderArray[whoesTurn]->building[0]=(atoi(elem)/1000);
            secondOrderArray[whoesTurn]->building[1]=(atoi(elem)%1000)/100;
            secondOrderArray[whoesTurn]->building[2]=(atoi(elem)%100)/10;
            secondOrderArray[whoesTurn]->building[3]=(atoi(elem)%10);
        }
        if(cal==6)
        {
            strcpy(input2,elem);
            elem2=strtok(input2,"+");
            while(elem2!=NULL)
            {
                if(cal2==1)
                    printf("現在是%d角色在行動\n",atoi(elem2));
                if(cal2==2){
                    printf("技能目標是%d角色\n",atoi(elem2));
                    if((strcmp("nobody",elem2))){
                        for (int i=0; i<N; i++) {
                            if (playerlist[i].job== atoi(elem2)) {
                                target = i;
                                break;
                            }
                        }
                    }
                    
                }
                if(cal2==3 && !strcmp(elem2,"d")){
                    printf("Job:%d killed.\n",playerlist[target].job);
                    playerlist[target].death=1;}
                if(cal2==4){
                    if((playerlist[target].building[0]>=atoi(elem2)/1000)){
                        playerlist[target].building[0]-=atoi(elem2)/1000;
                        if(secondOrderArray[whoesTurn]->job==2){
                            for (int buildingIndex=0; buildingIndex<4; buildingIndex++) {
                                tempBuilding = tempBuilding+secondOrderArray[whoesTurn]->building[buildingIndex];
                            }
                            if (tempBuilding<4) {
                                secondOrderArray[whoesTurn]->building[0]++;
                            }
                        }
                    }
                    if((playerlist[target].building[1]>=(atoi(elem2)%1000)/100)){
                        playerlist[target].building[1]-=(atoi(elem2)%1000)/100;
                        if(secondOrderArray[whoesTurn]->job==2){
                            for (int buildingIndex=0; buildingIndex<4; buildingIndex++) {
                                tempBuilding = tempBuilding+secondOrderArray[whoesTurn]->building[buildingIndex];
                            }
                            if (tempBuilding<4) {
                                secondOrderArray[whoesTurn]->building[1]++;
                            }
                        }
                    }
                    if ((playerlist[target].building[2]>=(atoi(elem2)%100)/10)) {
                        playerlist[target].building[2]-=(atoi(elem2)%100)/10;
                        if(secondOrderArray[whoesTurn]->job==2){
                            for (int buildingIndex=0; buildingIndex<4; buildingIndex++) {
                                tempBuilding = tempBuilding+secondOrderArray[whoesTurn]->building[buildingIndex];
                            }
                            if (tempBuilding<4) {
                                secondOrderArray[whoesTurn]->building[2]++;
                            }
                        }
                    }
                    if ((playerlist[target].building[3]>=(atoi(elem2)%10))) {
                        playerlist[target].building[3]-=(atoi(elem2)%10);
                        if(secondOrderArray[whoesTurn]->job==2){
                            for (int buildingIndex=0; buildingIndex<4; buildingIndex++) {
                                tempBuilding = tempBuilding+secondOrderArray[whoesTurn]->building[buildingIndex];
                            }
                            if (tempBuilding<4) {
                                secondOrderArray[whoesTurn]->building[3]++;
                            }
                        }
                    }
                }
                if(cal2==5 && !strcmp(elem2,"f")){
                    int difference = playerlist[target].firstOrder-0;
                    for (int j=0; j<N; j++) {
                        playerlist[j].firstOrder = (playerlist[j].firstOrder-difference)%3;
                        if (playerlist[j].firstOrder < 0) {
                            playerlist[j].firstOrder+=N;
                        }
                    }
                }
                elem2=strtok(NULL,"+");
                cal2++;
            }
        }
        elem=strtok(NULL,":");
        cal++;
    }
}


void SettingSocket(){
    // Socket setting
    int socket_desc , c , *new_sock;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        exit(1);
    }
    puts("Bind done");
    
    //Listen
    listen(socket_desc , 3);
    c = sizeof(struct sockaddr_in);
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
    /* Prepare section.. */
    // 等所有玩家進入遊戲
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
        new_sock = malloc(1);
        *new_sock = client_sock;
        
        if( pthread_create( &sniffer_thread[player_index] , NULL ,  test , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            exit(1);
        }
        
        playerlist[player_index].firstOrder = player_index;
        playerlist[player_index].ID = player_index;
        // 這邊是進來的順序，應該就是永久ID了？
        // play_index is the permanent order
        player_index+=1;
        
        // 人數到齊就break
        if ( player_index== N ) break;
    }
    printf("All players are in the game!\n");
}
void MoneyOrAssets(int newAssetsCard,int whoesTurn){
    printf("newAssetsCard :%d\n",newAssetsCard );
    if (newAssetsCard == 0){
        secondOrderArray[whoesTurn]->drawCard=0;
    } else {
        secondOrderArray[whoesTurn]->drawCard=1;
    }
    secondOrderArray[whoesTurn]->money+=2;
}

void SendStatus(int i,int situation){
    char str[4];
    
    bzero(sendStaus, 256);
    switch (situation) {
        case 1: //進入角色回合 啟動攻擊能力
            strcat(sendStaus,"!");    //驚嘆號個人
            strcat(sendStaus,":" );
            break;
        case 2: //broadcast
            strcat(sendStaus,"@");    //小老鼠廣播
            strcat(sendStaus,":" );
            break;
        case 3: //遊戲結束 廣播名次及分數
            strcat(sendStaus,"*");    //星號結束
            sprintf(str,"%d", secondOrderArray[i]->rank);
            strcat(sendStaus,str);
            strcat(sendStaus,"*");
            sprintf(str,"%d",secondOrderArray[i]->score);
            strcat(sendStaus,str);
            strcat(sendStaus,":" );
            break;
    }
    sprintf(str,"%d", secondOrderArray[i]->job);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str,"%d",secondOrderArray[i]->building[0]);
    strcat(sendStaus,str);
    sprintf(str,"%d",secondOrderArray[i]->building[1]);
    strcat(sendStaus,str);
    sprintf(str,"%d",secondOrderArray[i]->building[2]);
    strcat(sendStaus,str);
    sprintf(str,"%d",secondOrderArray[i]->building[3]);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str,"%d", secondOrderArray[i]->money);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str, "%d",secondOrderArray[i]->rCard);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str, "%d",secondOrderArray[i]->bCard);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str, "%d",secondOrderArray[i]->death);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    sprintf(str, "%d",secondOrderArray[i]->drawCard);
    strcat(sendStaus,str);
    strcat(sendStaus,":" );
    
    printf("sendStatus:%s\n",sendStaus);
    // write(sockfd);
}
int main(int argc , char *argv[])
{
    playerlist[0].job=2;
    playerlist[0].building[0]=2;
    playerlist[0].building[1]=0;
    playerlist[0].building[2]=1;
    playerlist[0].building[3]=0;
    playerlist[0].money=3;
    playerlist[0].rCard=1;
    playerlist[0].bCard=2;
    playerlist[0].death=0;
    playerlist[0].firstOrder=0;
    
    playerlist[1].job=3;
    playerlist[1].building[0]=1;
    playerlist[1].building[1]=1;
    playerlist[1].building[2]=0;
    playerlist[1].building[3]=0;
    playerlist[1].money=2;
    playerlist[1].rCard=0;
    playerlist[1].bCard=1;
    playerlist[1].death=0;
    playerlist[1].firstOrder=1;
    
    playerlist[2].job=1;
    playerlist[2].building[0]=0;
    playerlist[2].building[1]=0;
    playerlist[2].building[2]=1;
    playerlist[2].building[3]=0;
    playerlist[2].money=6;
    playerlist[2].rCard=0;
    playerlist[2].bCard=1;
    playerlist[2].death=0;
    playerlist[2].firstOrder=2;

    SettingSocket();
    while(!gameOver) // If not GameOver
    {
        DecideTheFirstOrder(); 
        /* Phase 1 ... */
        PHASE=1;
        for ( whoesTurn = 0; whoesTurn < N; whoesTurn++)
        {
            FirstOrderToChooseJob(whoesTurn);

            /* Communication! */
            bzero(client_message, 256);
            // Thread function part
            n=whoesTurn+1;
            sleep(1);
            // while(flag_phase1){printf("[from %d] yee\n", whoesTurn);sleep(1);};
            while(flag_phase1);
            flag_phase1=1;
            printf("Finish Communication!!\n");

            FirstOrderToModifyTheCard(whoesTurn);
        }
        n=0;  // reset checking index
        for (i=0;i<N;i++) 
            count[i]=0;
        printf("First Phase done..\n");

        //首家shift
        changeTheFirstOrder();
        //第二回合排序
        DecideTheSecondOrder();

        /* Phase 2 .. */
        PHASE+=1;
        for (whoesTurn=0;whoesTurn < N; whoesTurn++)
        {
            srand(seed);
            seed = (unsigned)time(NULL);
            MoneyOrAssets(rand()%2,whoesTurn); // 兩塊錢or一張卡
            SendStatus(whoesTurn,1);

            /* Communication Part! */
            bzero(client_message, 256);
            n = (secondOrderArray[whoesTurn]->ID)+1;
            sleep(1);
            while(flag_phase1) {sleep(1);}  // busy waiting until this thread function finish..
            flag_phase1=1;
            printf("Finish Communication!!\n");

            Attack(whoesTurn);

            judgeBuildingAmount(whoesTurn);
            if (gameOver) 
            {   
                for (i=0;i<N;i++) 
                    count[i]=0;
                for (int i=0; i<N; i++)
                {
                    SendStatus(i,3);

                    /* Communication Part! */
                    bzero(client_message, 256);
                    printf("n now is:%d\n", n);
                    n = i+1;
                    sleep(1);
                    while(flag_phase1) {sleep(1);}  // busy waiting until this thread function finish..
                    flag_phase1=1;
                    printf("Finish broadcast!!\n");

                } 
                printf("broadcast exit\n");    //broadcast
            } 
            // else 
            // {
            //     for (i=0;i<N;i++) 
            //         count[i]=0;
            //     for (int i; i < N; i++) 
            //     {
            //         SendStatus(i,2);

            //         /* Communication Part! */
            //         bzero(client_message, 256);
            //         n = i+1;
            //         sleep(1);
            //         while(flag_phase1) {sleep(1);}  // busy waiting until this thread function finish..
            //         flag_phase1=1;
            //         printf("Finish broadcast!!\n");
            //     }
            // }
            if (gameOver) 
            {
                break;
            }    
        }
        n=0;
        PHASE=0;
        for (i=0;i<N;i++) 
            count[i]=0;
        printf("Finish this round!\n");
    }

    printf("This game is finished!\n");

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}

void * test(void *socket_desc)
{
    int i, j;
    int sock = *(int*)socket_desc;
    int read_size;
    int player_index_handler = player_index;

    while(1)
    {
        /* Communication */
        if(n==player_index_handler)
        {
            if (count[player_index_handler-1]==1) break;

            if (PHASE==1)
            {
                *message = '$';
                *(message+1) = ':';
            }

            /* strcat phase and data */
            char send_msg[256]="";
            strcat(send_msg,message);
            if (PHASE==1)
                strcat(send_msg,sendCard);
            else if(PHASE==2)
                strcat(send_msg,sendStaus);
            write(sock, send_msg,strlen(send_msg));

            read_size = recv(sock , client_message , 2000 , 0);
            printf("[From player %d] the client_message is:%s\n", player_index_handler, client_message);
            bzero(message,256);
            bzero(sendCard,256);

            // Finish works..
            flag_phase1=0;
            count[player_index_handler-1]=1; // make sure each phase do one time..
            sleep(1);
        }
        else ;
    }
    return 0;
} 
