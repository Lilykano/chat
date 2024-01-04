#include "chat.h"

void readChat(int sock, char *buf);
void writeChat(int sock, char *buf);
void receiveData(int sock);

char name[NAME_SIZE] = "[NULL]";
char buf[BUFFER_SIZE];

void readChat(int sock, char *buf) {//읽기
    char total_message[NAME_SIZE + BUFFER_SIZE];
	int	str_len;
    while(1) {
        str_len=read(sock, total_message, NAME_SIZE + BUFFER_SIZE);
        if(str_len==0){//받아온 채팅버퍼에 이름을 더해서 출력
            return;
		}
        total_message[str_len]=0;
		printf("%s\n", total_message);
    }
}

void writeChat(int sock, char *buf) {//쓰기
    char total_message[NAME_SIZE + BUFFER_SIZE];
    while(1) {
        fgets(buf, BUFFER_SIZE, stdin);
        if(strcmp(buf,"/exit\n") == 0 || strcmp(buf,"/quit\n")==0) {// /exit나/quit를 입력하면 접속종료되게
            shutdown(sock, SHUT_WR);
            printf("접속종료\n");
            break;
        }
        sprintf(total_message, "%s %s",name, buf);
        write(sock, total_message, strlen(total_message));
    }
}

void receiveData(int sock) {//테이블데이터 받아오는
    int bytes_received;
    char buffer[1024];
    char end[256];

    while(1){
    memset(buffer, 0, 1024);
    memset(end, 0, 256);
        bytes_received = read(sock, buffer, sizeof(buffer) -1);
        if(strcmp(buffer,"끝\n")== 0){//"끝"을 받으면 종료
            break;
        }
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
}



int loginProcess(char *username, char *password, int sock){
	char buf[BUFFER_SIZE];
	int valid;
	int result;
	char login_request[BUFFER_SIZE];
	char login_response[BUFFER_SIZE];
	do{
		printf("아이디를 입력해주세요 : ");
		fgets(username, BUFFER_SIZE, stdin);
        username[strcspn(username, "\n")] = '\0';
        if(strchr(username, ' ') != NULL) {
			printf("아이디에 스페이스바가 포함되어있습니다.\n");
            valid = 0;
		} else{
			valid = 1;
		}
	}while(!valid);
    do{
        printf("비밀번호를 입력해주세요 : ");
        fgets(password, BUFFER_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0';
        if(strchr(password,' ') != NULL) {
            printf("비밀번호에 스페이스바가 포함되어있습니다.\n");
            valid = 0;
        } else{
            valid = 1;
        }
    }while(!valid);

    snprintf(login_request, sizeof(login_request), "%s %s", username, password);
    write(sock, login_request, strlen(login_request));
    printf("로그인:%s\n", login_request);
    memset(login_response, 0, sizeof(login_response));
	read(sock, login_response, sizeof(login_response));
    printf("read:%s\n", login_response);
    if (strcmp(login_response, "로그인 성공") == 0) {
        printf("로그인 성공\n");
        sprintf(name, "[%s]", username);
        system("clear");
        printf("메세지를 입력해주세요\n");
		result= 1;
    }
	else if(strcmp(login_response, "관리자 로그인 성공") == 0) {
        printf("관리자 로그인 성공\n");
		sprintf(name, "[%s]", username);
        result= 2;
	} else{
		printf("로그인 실패. 다시 한번 아이디와 비밀번호를입력해주세요\n");
		result= -1;
    }
	return result;
}

int registerProcess(char *username, char *password, int sock){
	char managekey[BUFFER_SIZE];
	int valid;
	char create_id[BUFFER_SIZE];
	char Overlap[BUFFER_SIZE];
	do{
		printf("아이디를 입력해주세요 : ");
        fgets(username, BUFFER_SIZE, stdin);
        username[strcspn(username, "\n")] = '\0';
        if(strchr(username, ' ') != NULL) {
			printf("아이디에 스페이스바가 포함되어있습니다.다시 입력해주세요.\n");
            valid = 0;
        } else{
            valid = 1;
        }
    }while(!valid);
    do{
		printf("비밀번호를 입력해주세요 : ");
        fgets(password, BUFFER_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0';
        if(strchr(password,' ') != NULL) {
            printf("비밀번호에 스페이스바가 포함되어있습니다.다시 입력해주세요\n");
            valid = 0;
        } else{
			valid = 1;
        }
    }while(!valid);
    snprintf(create_id, sizeof(create_id), "%s %s", username, password);
    write(sock, create_id, strlen(create_id));//아이디와 비밀번호를 전송
    if (read(sock, Overlap, sizeof(Overlap)) > 0 && strcmp(Overlap, "Overlap") == 0) {//중복확인 체크
    } else {
		printf("관리자 코드를 입력해주세요\n");
		memset(managekey, 0, BUFFER_SIZE);
        fgets(managekey, BUFFER_SIZE, stdin);
        managekey[strcspn(managekey, "\n")] = '\0';
        if(strcmp(managekey, "1803") == 0){
			printf("인증에 성공했습니다\n");
            write(sock, managekey, strlen(managekey));
            printf("관리자 회원가입 성공:%s\n", create_id);
        } else{
            printf("인증에 실패했습니다.\n");
            write(sock, managekey, strlen(managekey));
            printf("일반 회원가입 성공:%s\n", create_id);
        }
	}
}

int adminMenu(int sock, char *answer){
	int loopControl = 1;
	char username[BUFFER_SIZE];
	while(loopControl){
		printf("원하시는 서비스를 입력해주세요\n");
		printf("1.채팅 2.일반유저테이블조회 3.일반유저삭제\n");
		fgets(answer, BUFFER_SIZE, stdin);
		answer[strcspn(answer, "\n")] = '\0';
		write(sock, answer, strlen(answer));
				
		switch (answer[0]) {//1.채팅 2.테이블조회 3.유저삭제
			case '1':{
				printf("1번\n");
				printf("메세지를 입력해주세요\n");
				loopControl = 0;
				break;//브레이크로 함수빠져나가 바로 채팅으로
			}
			case '2':{
				printf("2번\n");
				receiveData(sock);//테이블데이터 받아오는 함수
				continue;
			}
			case '3':{
				printf("3번\n");
				printf("삭제를 원하시는 아이디를 입력해주세요\n");
				fgets(username, BUFFER_SIZE, stdin);
				username[strcspn(username, "\n")] = '\0';
				write(sock, username, strlen(username));
				printf("[%s] 삭제되었습니다\n",username);
				continue;
			}
		}
	}
}

int main(int argc, char *argv[]) {
    int sock;
	int loopControl= 1;
	int loginresult;
    char buf[BUFFER_SIZE];
	char username[BUFFER_SIZE];
	char password[BUFFER_SIZE];
	char answer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    pid_t pid;
	
    if(argc!=3) {
        printf("접속방법 : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr(argv[1]);
    server_addr.sin_port=htons(atoi(argv[2]));
	if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1){
        printf("connect()에러\n");
	} else{
		printf("접속 성공\n");
	}
	while(loopControl){
	    printf("1.로그인 2.회원가입\n");
	    printf("선택하실 서비스를 입력해주세요.\n");

		//로그인부분
		fgets(answer, BUFFER_SIZE, stdin);
		answer[strcspn(answer, "\n")] = '\0';
		write(sock, answer, strlen(answer));

		switch (answer[0]) {//1.로그인 2.회원가입
			case '1':{
			loginresult = loginProcess(username, password, sock);
			if(loginresult==1){//일반유저 리턴1
				loopControl = 0;
				break;
			}
			else if(loginresult==2){//관리자 리턴2
			adminMenu(sock, answer);
				loopControl = 0;
				break;
			}
			}
			case '2':{
				registerProcess(username, password, sock);
			}
		}
	}
    pid=fork();
	if(pid==-1){
		printf("fork에러\n");
		close(sock);
		return 0;
	}
    if(pid==0){ //자식프로세스
        writeChat(sock, buf);
	} else{ //부모프로세스
        readChat(sock, buf);
	}
    close(sock);
    return 0;
}
