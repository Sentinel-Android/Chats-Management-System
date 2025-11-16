#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<ctype.h>
#define MaximumLineLength 256
#define MaximumLinesInput 34


struct MessageBufferOne
{
    long mtype;
    char mtext[MaximumLineLength];
};

void functionProcessError(){
    perror("popen lite! Cant check status of process.");
}

void fileFindingError(){ 
  perror("Can't find file!");
}

void ProcessCreationError(){
    perror("Fork lite! No new process created.");
 }

 void ProcessDefinitionError(){
    perror("execl lite! Process definition did not change.");
}

void messagesendfail(){
    perror("msgsnd failed");
   }

   void errorINmsgget(){
    perror("Error in msgget");
   }

/*int CountForActiveProcesses() {
    int cnt = 0;
    FILE *fp=popen("pgrep -c groups.out","r");
    if(fp==NULL)
    {
        functionProcessError();
        exit(1);
    }
    fscanf(fp,"%d",&cnt);
    pclose(fp);
    return cnt;
}*/


int main(int argc, char*argv[])
{
    int x;
    int n;
    char PathOfFile[100];
    sscanf(argv[1],"%d",&x);
    char buffer[MaximumLinesInput][MaximumLineLength];
   snprintf(PathOfFile,sizeof(PathOfFile),"testcase_%d/%s",x,"input.txt");
   printf("Trying to open this file: %s\n", PathOfFile);
    FILE*file=fopen(PathOfFile,"r");
    if(file==NULL)
    {
        fileFindingError();
        return 1;
    }
    int cnt=0;
    int row=0;
    while(fgets(buffer[row],sizeof(buffer),file))
    {
        buffer[row][strcspn(buffer[row],"\n")]=0;
        if(cnt==0)
        {
            n=atoi(buffer[row]);
            cnt++;
        }
        row++;
    }
    fclose(file);
    int key=atoi(buffer[2]);
    /////////////////////////////////////////////////////////////////////////////
    for(int i=0;i<n;i++)
    {
        pid_t pid=fork();
        if(pid<0)
        {
            ProcessCreationError();
            exit(1);
        }
        else if(pid==0)
        {
            //char id_str[10];
            execl("./groups.out", "groups.out",buffer[2],NULL);
            ProcessDefinitionError();
            exit(1);
        }
        else
        {
            struct MessageBufferOne message;
            message.mtype=1;
            int msgid = msgget(key, 0666 | IPC_CREAT);
            memset(message.mtext, 0, sizeof(message.mtext));  
            strcpy(message.mtext,buffer[1]);
            msgsnd(msgid,&message,sizeof(message.mtext),0);
            memset(message.mtext, 0, sizeof(message.mtext)); 
            strcpy(message.mtext,buffer[3]);
            msgsnd(msgid,&message,sizeof(message.mtext),0);
            memset(message.mtext, 0, sizeof(message.mtext));
            strcpy(message.mtext,buffer[5+i]);
            msgsnd(msgid,&message,sizeof(message.mtext),0);
            
    char stringArray[2]; 
    stringArray[0] = x + '0'; 
    stringArray[1] = '\0'; 
    strcpy(message.mtext, stringArray); 
    if (msgsnd(msgid, &message, sizeof(message.mtext), 0) == -1) {
        messagesendfail();
        exit(1);
    }

    printf("Sending this testcase number: %s\n", message.mtext);
    
            sleep(1);
        }
    }

    /*while(CountForActiveProcesses()>0)
    {
        struct MessageBufferOne message1;
        message1.mtype=1;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        if (msgid == -1)
        {
            errorINmsgget();
            return 1;
        }
        if(msgrcv(msgid, &message1, sizeof(message1.mtext), 1, 0) != -1)
        {
            printf("All users are terminated so exiting group process %s\n",message1.mtext);
        }
        msgctl(msgid, IPC_RMID, NULL);
    }*/
   for(int j=0;j<n;j++)
   {
        wait(NULL);
        struct MessageBufferOne message1;
        message1.mtype=1;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        if (msgid == -1)
        {
            errorINmsgget();
            return 1;
        }
        if(msgrcv(msgid, &message1, sizeof(message1.mtext), 1, 0) != -1)
        {
            printf("All users are terminated so exiting group process %s\n",message1.mtext);
        }
        msgctl(msgid, IPC_RMID, NULL);
   }
    return 0;
}