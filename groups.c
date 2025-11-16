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
#define MaximumLinesOfInput 34
#define TimeoutOfSeconds 5
#define MaximumNumberOfGroups 30
#define MaximumNumberOfUsers 50

struct messageBufferOne
{
    long mtype;
    char mtext[MaximumLineLength];
};

struct chats
{
    long mtype;
    int user_id;
    int group_id;
    int timestamp;
    int ViolationsNumber;
    bool banned;
    char text_message[MaximumLineLength];
};

struct Message
{
    long mtype;
    int timestamp;
    int user;
    char mtext[256];
    int modifyingGroup;
};

long getCurrentTime(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec;
}

int ComparingTimestamp(const void *a, const void *b) {
    return ((struct chats *)a)->timestamp - ((struct chats *)b)->timestamp;
}

/*int existsInArray(int arr[], int size, int num) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == num) return 1; 
    }
    return 0;
}*/
int existsInArray(int g[], int size, int number)
{
    int n=0;
while(n<size)
{
    if (g[n] == number) return 1;  
    n++;  
}
return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <line_from_buffer>\n", argv[0]);
        return 1;
    }
    int key=atoi(argv[1]);
    struct messageBufferOne message1;
    message1.mtype=1;
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("msgget falied!");
        return 1;
    }
    char validation_key[MaximumLineLength];
    char moderator_key[MaximumLineLength];
    char group_file_path[MaximumLineLength];
    char test_case[MaximumLineLength];
    char testcase_no[MaximumLineLength];
    for(int i=0;i<4;i++)
    {
        msgrcv(msgid, &message1, sizeof(message1.mtext), 1, 0);
        if(i==0)
        {
            strcpy(validation_key,message1.mtext);
        }
        if(i==1)
        {
            strcpy(moderator_key,message1.mtext);
        }
        if(i==2)
        {
            strcpy(group_file_path,message1.mtext);
        }
        if(i==3)
        {
            message1.mtext[sizeof(message1.mtext) - 1] = '\0'; 

        	strcpy(testcase_no,message1.mtext);
        }
    }
    /*char grp_str[2];
    int count1=0;
    for(int i=13;i<15 && isdigit(group_file_path[i]);i++)
    {
        grp_str[count1++]=group_file_path[i];
    }
    int groupNumber=atoi(grp_str);*/
    int groupNumber;
    
	// Extract the number after "group_" and before ".txt"
	if (sscanf(group_file_path, "groups/group_%d.txt", &groupNumber) == 1) {
    	printf("Extracted group number: %d\n", groupNumber);
	} else {
    	printf("Failed to extract group number!\n");
	}

    struct Message group_created;
    int msgid2 = msgget(atoi(validation_key), 0666 | IPC_CREAT);
    group_created.mtype=1;
    group_created.modifyingGroup=groupNumber;
    msgsnd(msgid2, &group_created, sizeof(group_created) - sizeof(group_created.mtype), 0);
    msgctl(msgid, IPC_RMID, NULL);
    char buffer2[MaximumLinesOfInput][MaximumLineLength];
    char filepath[1000];
    int test_no=atoi(test_case);
    snprintf(filepath,sizeof(filepath),"testcase_%s/%s",testcase_no,group_file_path);
    //printf("Nishtha checking\n");
    FILE*file2=fopen(filepath,"r");
    if(file2==NULL)
    {
        perror("Error in finding file!\n");
        return 1;
    }
    int row2=0;
    while(fgets(buffer2[row2],sizeof(buffer2),file2))
    {
        buffer2[row2][strcspn(buffer2[row2],"\n")]=0;
        row2++;
    }
    int n=atoi(buffer2[0]);
    int fd[2];
    if(pipe(fd)==-1)
    {
        perror("Pipe failed");
        return 1;
    }
    /////////////////////////////////////////////////////////////////
    for(int i=0;i<n;i++)
    {
        int count=0;
        int k;
        for(int j=0;buffer2[i+1][j]!='\0';j++)
        {
            if(buffer2[i+1][j]=='_')
            {
                count++;
                if(count==2)
                {
                    k=j+1;
                    break;
                }
            }
        }
        char num_str[10];
        int index=0;
        for(k;buffer2[i+1][k]!='\0' && isdigit(buffer2[i+1][k]);k++)
        {
            num_str[index++]=buffer2[i+1][k];
        }
        int usr_no=atoi(num_str);
       // printf(" HI     %d\n",usr_no);
        struct Message user_created;
        user_created.mtype=2;
        user_created.modifyingGroup=groupNumber;
        user_created.user=usr_no;
        msgsnd(msgid2, &user_created, sizeof(user_created) - sizeof(user_created.mtype), 0);
        pid_t pid=fork();
        if(pid<0)
        {
            perror("Fork lite! No new process created.\n");
            exit(1);
        }
        else if(pid==0)
        {
            close(fd[0]);
            char buffer4[MaximumLinesOfInput][MaximumLineLength];
            char filepath1[1000];
            snprintf(filepath1,sizeof(filepath1),"testcase_%s/%s",testcase_no,buffer2[i+1]);
            FILE*file3=fopen(filepath1,"r");
            if(file3==NULL)
            {
                perror("Error in findind file!\n");
                return 1;
            }
            int row4=0;
            while(fgets(buffer4[row4],sizeof(buffer4),file3))
            {
                buffer4[row4][strcspn(buffer4[row4],"\n")]=0;
                row4++;
            }
            for (int i = 0; i < row4; i++) {
                int prefixLen = strlen(num_str);
                int lineLen = strlen(buffer4[i]);
        
                memmove(buffer4[i] + prefixLen + 1, buffer4[i], lineLen + 1); 
        
                memcpy(buffer4[i], num_str, prefixLen);
                buffer4[i][prefixLen] = ' ';
            }
            fclose(file3);
            for(int l=0;l<row4;l++)
            {
                printf("User process:%s\n",buffer4[l]);
                write(fd[1], buffer4[l], sizeof(buffer4[l]));
                //close(fd[1]);
            }
            close(fd[1]);
            exit(0);
        }
    }
    ////////////////////////////////////////////////////////////////////////
    close(fd[1]);
    char buffer3[500];
    long startTime = getCurrentTime();
    int no_of_messages=0;
    struct chats chat1;
    struct chats chat_safe[1000]; 
    int current = 0; 
    int msgid1 = msgget(atoi(moderator_key), 0666 | IPC_CREAT);
    if (msgid1 == -1) {
        perror("Error in msgget");
        return 1;
    }
    while (1) {
        memset(buffer3, 0, sizeof(buffer3));
        if ((read(fd[0], buffer3, sizeof(buffer3))) > 0) {
            printf("Group process from user:%s\n",buffer3);
            startTime = getCurrentTime();
            no_of_messages++;
            char str1[50], str2[50], str3[256]; 
            sscanf(buffer3, "%s %s %[^\n]", str1, str2, str3); 
            chat1.user_id = atoi(str1);
            chat1.group_id = groupNumber;
            chat1.timestamp = atoi(str2);
            chat1.ViolationsNumber = 0;
            chat1.banned = 0;
            memset(chat1.text_message,0,sizeof(chat1.text_message));
            strcpy(chat1.text_message, str3);
            chat1.mtype = 100;
            if (current < 1000) {
                chat_safe[current++] = chat1;
            } else {
                printf("Chat buffer full, cannot store more messages.\n");
            }
        }
        if (getCurrentTime() - startTime >= TimeoutOfSeconds) {
            break;
        }
        sleep(1);
    }
    close(fd[0]);  
    qsort(chat_safe, current, sizeof(struct chats), ComparingTimestamp);
    for (int i = 0; i < current; i++) {
        printf("Group process:%d %d %s\n",chat_safe[i].group_id,chat_safe[i].user_id,chat_safe[i].text_message);
        if (msgsnd(msgid1, &chat_safe[i], sizeof(chat_safe[i]) - sizeof(chat_safe[i].mtype), 0) == -1) {
            perror("Error sending message");
        } 
    }
    struct chats new_chat;
    struct Message valid_send;
    int alive_count=n;
    int dead_users[MaximumNumberOfUsers];
    int index=0;
    for(int i=0;i<MaximumNumberOfUsers;i++)
    {
        dead_users[i]=-1;
    }
    while(1)
    {
        msgrcv(msgid1, &new_chat, sizeof(new_chat) - sizeof(new_chat.mtype), groupNumber, 0);
        startTime = getCurrentTime();
        if(new_chat.banned==0)
        {
            valid_send.mtype=MaximumNumberOfGroups+groupNumber;
            valid_send.timestamp=new_chat.timestamp;
            valid_send.user=new_chat.user_id;
            memset(valid_send.mtext,0,sizeof(valid_send.mtext));
            strcpy(valid_send.mtext,new_chat.text_message);
            valid_send.modifyingGroup=groupNumber;
            msgsnd(msgid2, &valid_send, sizeof(valid_send) - sizeof(valid_send.mtype), 0);
        }
        else
        {
            if (!existsInArray(dead_users, index, new_chat.user_id)) {
                valid_send.mtype=MaximumNumberOfGroups+groupNumber;
                valid_send.timestamp=new_chat.timestamp;
                valid_send.user=new_chat.user_id;
                memset(valid_send.mtext,0,sizeof(valid_send.mtext));
                strcpy(valid_send.mtext,new_chat.text_message);
                valid_send.modifyingGroup=groupNumber;
                msgsnd(msgid2, &valid_send, sizeof(valid_send) - sizeof(valid_send.mtype), 0);
                dead_users[index++] = new_chat.user_id;
                alive_count--;
            }
        }
        if(alive_count<2)
        {
            struct Message group_terminated;
            group_terminated.mtype=3;
            group_terminated.modifyingGroup=groupNumber;
            group_terminated.user=index;
            msgsnd(msgid2, &group_terminated, sizeof(group_terminated) - sizeof(group_terminated.mtype), 0);
            struct messageBufferOne message1;
            message1.mtype=1;
            snprintf(message1.mtext,sizeof(message1.mtext),"%d",groupNumber);
            msgsnd(msgid, &message1, sizeof(message1.mtext), 0);
            break;
        }
        if (getCurrentTime() - startTime >= TimeoutOfSeconds) {
            break;
        }
        sleep(1);
    }
    msgctl(msgid, IPC_RMID, NULL);
    msgctl(msgid1, IPC_RMID, NULL);
    msgctl(msgid2, IPC_RMID, NULL);
    return 0;
}




