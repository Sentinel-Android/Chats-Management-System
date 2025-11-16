#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<ctype.h>
#include<stdbool.h>
#include<sys/time.h>

#define MaximumLineLength 256
#define MaximumLineOfInput 34
#define MaximumFilteredWords 50    
#define MaximumLengthOfWord    20  
#define MaximumInputLength   256  
#define TimeoutTimeInSec 5

void toLowerCase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

long getCurrentTime(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec;
}
//////////////////////////////////////////////////////////////////////////////
int FindingRow(int GroupNumber, int UserNumber, int Array[][3]) {
    for (int i = 0; i < 1000; i++) {
        if (Array[i][0] == GroupNumber && Array[i][1] == UserNumber) {
            return i;
        }
    }
    return -1;
}

bool ModifyData(int GroupNumber, int UserNumber, int violations, int Array[][3]) {
    int IndexOfRow = FindingRow(GroupNumber, UserNumber, Array);

    if (IndexOfRow != -1) {
        Array[IndexOfRow][2]+=violations;
        return true;
    } else {
        for (int i = 0; i < 1000; i++) {
            if (Array[i][0] == -1) { 
                Array[i][0] = GroupNumber;
                Array[i][1] = UserNumber;
                Array[i][2] = violations;
                return true;
            }
        }
        return false; 
    }
}
////////////////////////////////////////////////////////////////////////////////

struct chats
{
    long mtype;
    int user_id;
    int group_id;
    int timestamp;
    int violationNumber;
    bool banned;
    char text_message[MaximumLineLength];
};

int main(int argc, char*argv[])
{
    int x;
    char PathOfFile[100];
    sscanf(argv[1],"%d",&x);
    char buffer[MaximumLineOfInput][MaximumLineLength];
    snprintf(PathOfFile,sizeof(PathOfFile),"testcase_%d/%s",x,"input.txt");
    FILE*file=fopen(PathOfFile,"r");
    if(file==NULL)
    {
        perror("Error, file not found!\n");
        return 1;
    }
    int row=0;
    while(fgets(buffer[row],sizeof(buffer),file))
    {
        buffer[row][strcspn(buffer[row],"\n")]=0;
        row++;
    }
    fclose(file);
    int key=atoi(buffer[3]);
    int ThresholdOfViolations=atoi(buffer[4]);
    char PathOfFile1[100];
    char buffer1[MaximumFilteredWords][MaximumLengthOfWord];
    snprintf(PathOfFile1,sizeof(PathOfFile1),"testcase_%d/%s",x,"filtered_words.txt");
    FILE*file1=fopen(PathOfFile1,"r");
    if(file1==NULL)
    {
        perror("Error, file not found!\n");
        return 1;
    }
    int row1=0;
    while(fgets(buffer1[row1],sizeof(buffer1[row1]),file1))
    {
        buffer1[row1][strcspn(buffer1[row1],"\n")]=0;
        row1++;
    }
    fclose(file1);
    int countOFfilteredWords = 0;
    for (int i = 0; i < MaximumFilteredWords; i++) {
        if (strlen(buffer1[i]) > 0) { 
            countOFfilteredWords++;
        }
    }
    int PairOfGrupandUser[1000][3];
    struct chats message_new;
    message_new.mtype=1;
    int msgid=msgget(key,0666|IPC_CREAT);
        if (msgid == -1)
        {
            perror("Error in msgget");
            return 1;
        }
        ///////////////////////////////////////////////////////////////////
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 3; j++) {
                PairOfGrupandUser[i][j] = -1; 
            }
        }
        long startTime = getCurrentTime();
        //////////////////////////////////////////////////////////////
        while(1)
        {
            msgrcv(msgid, &message_new, sizeof(message_new) - sizeof(message_new.mtype), 100, 0);
            printf("%d %d %d\n",message_new.timestamp,message_new.user_id,message_new.group_id);
            startTime = getCurrentTime();
            char inputLower[MaximumInputLength];
            strcpy(inputLower, message_new.text_message);
            toLowerCase(inputLower);
            int threshold = 0;
            int found[MaximumFilteredWords] = {0};
            for (int i = 0; i < countOFfilteredWords; i++) {
                if (strstr(inputLower, buffer1[i]) != NULL) {  
                    if (!found[i]) { 
                        threshold++;
                        found[i] = 1;
                    }
                }
            }
            //////////////////////////////////////////////////////////////
            ModifyData(message_new.group_id, message_new.user_id,threshold,PairOfGrupandUser);
            int GettingRow=FindingRow(message_new.group_id, message_new.user_id,PairOfGrupandUser);
            if(PairOfGrupandUser[GettingRow][2]>=ThresholdOfViolations)
            {
                message_new.banned=1;
            }
            message_new.violationNumber=PairOfGrupandUser[GettingRow][2];
            message_new.mtype=message_new.group_id;
             //////////////////////////////////////////////////////////////////
            /*message_new.violationNumber+=threshold;
            if(message_new.violationNumber>=ThresholdOfViolations)
            {
                message_new.banned=true;
            }
            message_new.mtype=2;*/
            msgsnd(msgid, &message_new, sizeof(message_new) - sizeof(message_new.mtype), 0);
            if (getCurrentTime() - startTime >= TimeoutTimeInSec) {
                break;
            }
            sleep(1);
        }
        msgctl(msgid, IPC_RMID, NULL);
    return 0;
}




