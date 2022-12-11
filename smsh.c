#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

typedef struct Node{
    struct Node *next;
    char text[10000];
}Node;

typedef struct LinkedList{
    int count;
    struct Node *head;
}LinkedList;

void insert(LinkedList* list, char* text);
void delete(LinkedList* list, int position);
void printlist(LinkedList* list);
void deletelist(LinkedList* list);
void execCommand(Node* temp, LinkedList* history);
void insertAt(LinkedList* list, char* text, int position);
char* search(LinkedList* list, int position);

int main(void){
    int pid;
    char* semicolon;
    char* command;
    char* history_check;
    char path[1000];
    LinkedList* history;
    history = (LinkedList*)malloc(sizeof(LinkedList));
    history -> count = 0 ;
    history -> head = NULL;
    
    LinkedList* list;
    list = (LinkedList*)malloc(sizeof(LinkedList));
    list -> count = 0 ;
    list -> head = NULL;
    

    while(1){
        history_check = (char *)malloc(sizeof(char) * 1000);
        semicolon = (char *)malloc(sizeof(char) * 1000);
        
        getcwd(path, 1000);
        printf("[%s]$ ==> ",path);
        scanf("%[^\n]", semicolon);
        
        //종료 커맨드 들오어면 종료
        if(!strcmp(semicolon,"quit") || !strcmp(semicolon,"exit")){
            printf("Bye\n");
            exit(0);
        }
        
        //명령어에 history명령어가 들어오는지 확인
        strcpy(history_check, semicolon);
        history_check = strtok(history_check, " ");
        
        //쉘에 들어온 문자열 받는 부분 ';' 로 구분
        semicolon = strtok(semicolon,";");

        while(semicolon != NULL){
            //링크드리스트에 나눠진 문자열을 넣음
            insert(list,semicolon);
            
            //히스토리에 저장
            if(strstr(history_check, "!") == NULL){
                if(history->count < 20){
                    insertAt(history,semicolon , 1);
                }else{
                    delete(history, 20);
                    insertAt(history,semicolon , 1);
                }
            }
            
            //다음 세미콜론으로 넘어감
            semicolon = strtok(NULL,";");
        }
        getchar();
        
        Node* temp;
        temp = list -> head;
        int j;
        for(j = 0 ; j< list->count; j++){
            execCommand(temp, history);
            temp = temp -> next;
        }
        deletelist(list);
        
        free(semicolon);
    }
    free(list);
	return 0;
}


void execCommand(Node* temp, LinkedList* history){
    LinkedList* list2;
    list2 = (LinkedList*)malloc(sizeof(LinkedList));
    list2 -> count = 0 ;
    list2 -> head = NULL;
    
    
    char **argv;
    int pid, status, i, cmd;
    char* command = (char*)malloc(sizeof(char)*1000);
    char* check = (char*)malloc(sizeof(char)*1000);
    command = temp -> text;
    
    check = strstr(command,  "&");
    pid = fork();
    
    if(pid == 0){
        if(command[0] == '!' && command[1] != '!'){
            //history명령어인경우
            //명령어에 적힌 히스토리를 불러옴
                int k;
                int num = atoi(&command[1]);
                if(num != 0 && num<=history -> count){
                    temp = history->head;
                    for(k = 1 ; k < num ; k++){
                        temp = temp -> next;
                    }
                    strcpy(command, temp->text);
                }else{
                    printf("Command not found!\n");
                }
            

            command = strtok(command, "&");
            strcpy(temp->text, command);
            execCommand(temp, history);

            deletelist(list2);
        }else if(command[0] == '!' && command[1] == '!'){
            //명령어에 적힌 히스토리를 불러옴
                int k;
                int num = 1;
                if(num != 0 && num<=history -> count){
                    temp = history->head;
                    for(k = 1 ; k < num ; k++){
                        temp = temp -> next;
                    }
                    strcpy(command, temp->text);
                }else{
                    printf("Command not found!\n");
                }
            
            
            command = strtok(command, "&");
            //            temp -> text = command;
            strcpy(temp->text, command);
            execCommand(temp, history);
            deletelist(list2);
        }else{
        if(strstr(command,"|") != NULL){
            LinkedList* pipe2;
            pipe2 = (LinkedList*)malloc(sizeof(LinkedList));
            pipe2 -> count = 0 ;
            pipe2 -> head = NULL;
            
            
            //파이프와 리다이렉션 둘다 있는경우
            if((strstr(command, ">")!=NULL) || (strstr(command, "<")!=NULL) || (strstr(command, ">>")!=NULL) || (strstr(command, ">!")!=NULL)){
                printf("Does not offer this command (pipe + redirection)!\n");
                
            }else{//파이프만 있는경우
                int n;
                char* left;
                char* right;
                left = (char*)malloc(sizeof(char)*1000);
                right = (char*)malloc(sizeof(char)*1000);
                char* c1 = (char*)malloc(sizeof(char)*1000);
                char* c2 = (char*)malloc(sizeof(char)*1000);
                char* result;
                int mem = 0;
                
                strcpy(c2,command);
                c2 = strtok(c2, "|");
                //|를 기준으로 파싱한뒤 각각의 값들은 리스트에 넣음
                while(c2 != NULL){
                    insert(pipe2,c2);
                    c2 = strtok(NULL, "|");
                }
                
                while(pipe2 -> count !=0){
                    int pfd[2];
                    strcpy(left,search(pipe2,1));

                    pipe(pfd);
                    if((n = fork())==-1){
                        perror("Error in fork");
                        exit(1);
                    }else if(n == 0){
                        dup2(mem,0);
                        if(pipe2->count !=1){
                            dup2(pfd[1],1);
                        }
                        close(pfd[0]);
                        
                        //execvp의 인자로 넣어주기위해 파싱하여 링크드리스트에 넣음.
                        left = strtok(left, " ");
                        strcpy(c1,left);
                        while(left != NULL){
                            insert(list2,left);
                            left = strtok(NULL, " ");
                        }
                        
                        argv = (char**)malloc( sizeof(char) * (list2->count) );
                        for(i = 0 ; i<list2->count ; i++){
                            argv[i] = (char*)malloc(sizeof(char)*1000);
                        }
                        
                        Node* temp = list2 -> head;
                        for(i = 0 ; i<list2->count+1; i++){
                            if(i == list2->count){
                                argv[i] = NULL;
                                break;
                            }
                            argv[i] = temp->text;
                            temp = temp->next;
                        }
                        execvp(c1,argv);
                        deletelist(list2);
                        
                    }else{
                        wait(NULL);
                        close(pfd[1]);
                        mem = pfd[0];
                        delete(pipe2,1);
                    }
                }

                
                deletelist(list2);
                free(left);
                free(right);
                free(c1);
                free(c2);
            }
        }else{
            //파이프는 없지만 리다이렉션이 있는경우
            if((strstr(command, ">")!=NULL) || (strstr(command, "<")!=NULL) || (strstr(command, ">>")!=NULL) || (strstr(command, ">!")!=NULL)){

                int fd, fd2, n;
                char* left;
                char* right;
                left = (char*)malloc(sizeof(char)*1000);
                right = (char*)malloc(sizeof(char)*1000);
                char *c1 = (char*)malloc(sizeof(char)*1000);
                
                //각각의 리다이렉션에 따라 파싱 후 파일 오픈
                if(strstr(command, ">")!=NULL || strstr(command, ">>")!=NULL || strstr(command, ">!")!=NULL){
                    if(strstr(command, ">")!=NULL && strstr(command, ">>")==NULL && strstr(command, ">!")==NULL){
                        command = strtok(command, ">");
                        strcpy(left,command);
                        command = strtok(NULL,"");
                        strcpy(right, command);
                        right = strtok(right, " ");
//                        printf("left = @@%s@@ right = @@%s@@\n",left, right);
                        if( (fd = open(right, O_WRONLY | O_CREAT| O_TRUNC, 0644)) < 0){
                            printf("error in open!");
                            return;
                        }
                        
                    }else if(strstr(command, ">>")!=NULL){
                        command = strtok(command, ">>");
                        strcpy(left,command);
                        command = strtok(NULL,"");
                        strcpy(right, command);
                        right = strtok(right, ">");
                        right = strtok(right, " ");
//                        printf("left = @@%s@@ right = @@%s@@\n",left, right);
                        if( (fd = open(right, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0){
                            printf("error in open!");
                            return;
                        }
                        
                    }else if(strstr(command, ">!")!=NULL){
                        command = strtok(command, ">!");
                        strcpy(left,command);
                        command = strtok(NULL,"");
                        strcpy(right, command);
                        right = strtok(right, "!");
                        right = strtok(right, " ");
//                        printf("left = @@%s@@ right = @@%s@@\n",left, right);
                        if( (fd = open(right, O_WRONLY | O_CREAT, 0644)) < 0){
                            printf("error in open!");
                            return;
                        }
                    }
                    
                    dup2(fd,1);

                    //execvp의 인자로 넣어주기위해 파싱하여 링크드리스트에 넣음.
                    left = strtok(left, " ");
                    strcpy(c1,left);
                    while(left != NULL){
                        insert(list2,left);
                        left = strtok(NULL, " ");
                    }

                    argv = (char**)malloc( sizeof(char) * (list2->count) );
                    for(i = 0 ; i<list2->count ; i++){
                        argv[i] = (char*)malloc(sizeof(char)*1000);
                    }

                    Node* temp = list2 -> head;
                    for(i = 0 ; i<list2->count+1; i++){
                        if(i == list2->count){
                            argv[i] = NULL;
                            break;
                        }
                        argv[i] = temp->text;
                        temp = temp->next;
                    }

                    if(execvp(c1,argv) == -1){
                        fprintf(stderr,"Wrong command!\n");
                        exit(1);
                    }
                    deletelist(list2);

                    close(fd);
                    
                
                }else if(strstr(command, "<")!=NULL){
                    command = strtok(command, "<");
                    strcpy(left,command);
                    command = strtok(NULL,"");
                    strcpy(right, command);
                    right = strtok(right, " ");
//                    printf("left = @@%s@@ right = @@%s@@\n",left, right);
                    if ((fd = open(right, O_RDONLY, 0644)) < 0){
                        printf("error in open!");
                        return;
                    }
                    
                    
                    
                    dup2(fd, 0);
                    
                    //execvp의 인자로 넣어주기위해 파싱하여 링크드리스트에 넣음.
                    left = strtok(left, " ");
                    strcpy(c1,left);
                    while(left != NULL){
                        insert(list2,left);
                        left = strtok(NULL, " ");
                    }
                    
                    argv = (char**)malloc( sizeof(char) * (list2->count) );
                    for(i = 0 ; i<list2->count ; i++){
                        argv[i] = (char*)malloc(sizeof(char)*1000);
                    }
                    
                    Node* temp = list2 -> head;
                    for(i = 0 ; i<list2->count+1; i++){
                        if(i == list2->count){
                            argv[i] = NULL;
                            break;
                        }
                        argv[i] = temp->text;
                        temp = temp->next;
                    }
                    
                    if(execvp(c1,argv) == -1){
                        fprintf(stderr,"Wrong command!\n");
                        exit(1);
                    }
                    deletelist(list2);
                    
                    close(fd);
                    
                }
                
                free(left);
                free(right);
                free(c1);
            }else{//리다이렉션이랑 파이프 없는경우
                Node* temp;
                
                //cd명령어나 history 명령어가 있는경우
                if(strstr(command, "cd") != NULL){
                    char *c1 = (char*)malloc(sizeof(char)*1000);
                    command = strtok(command, " ");
                    strcpy(c1,command);
                    
                    //cd명령어인경우
                    if(!strcmp(c1,"cd")){
                        command = strtok(NULL, " ");
                        
                        if(chdir(command) < 0){
                            printf("Can't change Directory!\n");
                        }
                        
                    }

                    free(c1);
                }else{
                    command = strtok(command, "&");
                    char *c1 = (char*)malloc(sizeof(char)*1000);
                    
                    //execvp의 인자로 넣어주기위해 파싱하여 링크드리스트에 넣음.
                    command = strtok(command, " ");
                    strcpy(c1,command);
                    while(command != NULL){
                        insert(list2,command);
                        command = strtok(NULL, " ");
                    }

                    argv = (char**)malloc( sizeof(char) * (list2->count) );
                    for(i = 0 ; i<list2->count ; i++){
                        argv[i] = (char*)malloc(sizeof(char)*1000);
                    }
                    
                    Node* temp = list2 -> head;
                    for(i = 0 ; i<list2->count+1; i++){
                        if(i == list2->count){
                            argv[i] = NULL;
                            break;
                        }
                        argv[i] = temp->text;
                        temp = temp->next;
                    }

                    execvp(c1,argv);
                    deletelist(list2);
                    free(c1);
                }
            }
        }
        }
    }
    //NULL일경우 자식프로세스 먼저실행 아닐경우 백그라운드
    if(check == NULL){
        wait(&status);
    }
    free(list2);
//    free(command);
//    free(check);
}


void insert(LinkedList* list, char* text){
    int i;
    Node* temp = list -> head;
    
    Node* ins = (Node*)malloc(sizeof(Node));
    
    if(list -> count == 0){
        list -> head = ins;
        ins -> next = NULL;
        strcpy(ins->text, text);
        (list -> count)++;
    }else{
        for(i = 1 ; i < list->count ; i++){
            temp = temp -> next;
        }
        temp -> next = ins;
        ins -> next = NULL;
        strcpy(ins->text, text);
        (list -> count)++;
    }
}

void insertAt(LinkedList* list, char* text, int position){
    int i;
    Node* temp = list -> head;
    
    if(position<1 || position>(list->count)+1){
        printf("position error in insertAt\n");
    }else{
        Node* ins = (Node*)malloc(sizeof(Node));
        
        if(position == 1){
            ins -> next = list -> head;
            list -> head = ins;
            strcpy(ins->text, text);
        }else{
            for(i = 2 ; i < position; i++){
                temp = temp -> next;
            }
            ins -> next = temp -> next;
            temp -> next = ins;
            strcpy(ins->text, text);
        }
        (list -> count)++;
    }
}

void delete(LinkedList* list, int position){
    int i;
    Node* temp = list -> head;
    if(position <1 || position > list -> count){
        printf("position error or linkedlist count = 0 in delete\n");
        
    }else{
        if(position == 1){
            list -> head = temp -> next;
            free(temp);
        }else{
            for(i = 2 ; i < position ; i++){
                temp = temp -> next;
            }
            Node* temp2 = temp -> next;
            temp -> next = (temp -> next)->next;
            free(temp2);
        }
        (list -> count)--;
    }
}

void printlist(LinkedList* list){
    int i;
    Node* temp = list -> head;
    
    for(i = 0 ; i < list->count ; i++){
        printf("%s\n",temp->text);
        temp = temp->next;
    }
}
void deletelist(LinkedList* list){
    int i, count;
    count = list -> count;
    for(i = 0 ; i<count ; i ++){
        delete(list,1);
    }

}

char* search(LinkedList* list, int position){
    int i;
    Node* temp = list -> head;
    
    if(position <= 0){
        return NULL;
    }
    
    for(i = 1 ; i < position ; i++){
        temp = temp->next;
    }
    
    return temp->text;
}
