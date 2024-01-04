#include "chat.h"

void readChat(int sock, char *buf);
void writeChat(int sock, char *buf);
void receiveData(int sock);

char name[NAME_SIZE] = "[NULL]";
char buf[BUFFER_SIZE];

void readChat(int sock, char *buf) {//�б�
    char total_message[NAME_SIZE + BUFFER_SIZE];
	int	str_len;
    while(1) {
        str_len=read(sock, total_message, NAME_SIZE + BUFFER_SIZE);
        if(str_len==0){//�޾ƿ� ä�ù��ۿ� �̸��� ���ؼ� ���
            return;
		}
        total_message[str_len]=0;
		printf("%s\n", total_message);
    }
}

void writeChat(int sock, char *buf) {//����
    char total_message[NAME_SIZE + BUFFER_SIZE];
    while(1) {
        fgets(buf, BUFFER_SIZE, stdin);
        if(strcmp(buf,"/exit\n") == 0 || strcmp(buf,"/quit\n")==0) {// /exit��/quit�� �Է��ϸ� ��������ǰ�
            shutdown(sock, SHUT_WR);
            printf("��������\n");
            break;
        }
        sprintf(total_message, "%s %s",name, buf);
        write(sock, total_message, strlen(total_message));
    }
}

void receiveData(int sock) {//���̺����� �޾ƿ���
    int bytes_received;
    char buffer[1024];
    char end[256];

    while(1){
    memset(buffer, 0, 1024);
    memset(end, 0, 256);
        bytes_received = read(sock, buffer, sizeof(buffer) -1);
        if(strcmp(buffer,"��\n")== 0){//"��"�� ������ ����
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
		printf("���̵� �Է����ּ��� : ");
		fgets(username, BUFFER_SIZE, stdin);
        username[strcspn(username, "\n")] = '\0';
        if(strchr(username, ' ') != NULL) {
			printf("���̵� �����̽��ٰ� ���ԵǾ��ֽ��ϴ�.\n");
            valid = 0;
		} else{
			valid = 1;
		}
	}while(!valid);
    do{
        printf("��й�ȣ�� �Է����ּ��� : ");
        fgets(password, BUFFER_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0';
        if(strchr(password,' ') != NULL) {
            printf("��й�ȣ�� �����̽��ٰ� ���ԵǾ��ֽ��ϴ�.\n");
            valid = 0;
        } else{
            valid = 1;
        }
    }while(!valid);

    snprintf(login_request, sizeof(login_request), "%s %s", username, password);
    write(sock, login_request, strlen(login_request));
    printf("�α���:%s\n", login_request);
    memset(login_response, 0, sizeof(login_response));
	read(sock, login_response, sizeof(login_response));
    printf("read:%s\n", login_response);
    if (strcmp(login_response, "�α��� ����") == 0) {
        printf("�α��� ����\n");
        sprintf(name, "[%s]", username);
        system("clear");
        printf("�޼����� �Է����ּ���\n");
		result= 1;
    }
	else if(strcmp(login_response, "������ �α��� ����") == 0) {
        printf("������ �α��� ����\n");
		sprintf(name, "[%s]", username);
        result= 2;
	} else{
		printf("�α��� ����. �ٽ� �ѹ� ���̵�� ��й�ȣ���Է����ּ���\n");
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
		printf("���̵� �Է����ּ��� : ");
        fgets(username, BUFFER_SIZE, stdin);
        username[strcspn(username, "\n")] = '\0';
        if(strchr(username, ' ') != NULL) {
			printf("���̵� �����̽��ٰ� ���ԵǾ��ֽ��ϴ�.�ٽ� �Է����ּ���.\n");
            valid = 0;
        } else{
            valid = 1;
        }
    }while(!valid);
    do{
		printf("��й�ȣ�� �Է����ּ��� : ");
        fgets(password, BUFFER_SIZE, stdin);
        password[strcspn(password, "\n")] = '\0';
        if(strchr(password,' ') != NULL) {
            printf("��й�ȣ�� �����̽��ٰ� ���ԵǾ��ֽ��ϴ�.�ٽ� �Է����ּ���\n");
            valid = 0;
        } else{
			valid = 1;
        }
    }while(!valid);
    snprintf(create_id, sizeof(create_id), "%s %s", username, password);
    write(sock, create_id, strlen(create_id));//���̵�� ��й�ȣ�� ����
    if (read(sock, Overlap, sizeof(Overlap)) > 0 && strcmp(Overlap, "Overlap") == 0) {//�ߺ�Ȯ�� üũ
    } else {
		printf("������ �ڵ带 �Է����ּ���\n");
		memset(managekey, 0, BUFFER_SIZE);
        fgets(managekey, BUFFER_SIZE, stdin);
        managekey[strcspn(managekey, "\n")] = '\0';
        if(strcmp(managekey, "1803") == 0){
			printf("������ �����߽��ϴ�\n");
            write(sock, managekey, strlen(managekey));
            printf("������ ȸ������ ����:%s\n", create_id);
        } else{
            printf("������ �����߽��ϴ�.\n");
            write(sock, managekey, strlen(managekey));
            printf("�Ϲ� ȸ������ ����:%s\n", create_id);
        }
	}
}

int adminMenu(int sock, char *answer){
	int loopControl = 1;
	char username[BUFFER_SIZE];
	while(loopControl){
		printf("���Ͻô� ���񽺸� �Է����ּ���\n");
		printf("1.ä�� 2.�Ϲ��������̺���ȸ 3.�Ϲ���������\n");
		fgets(answer, BUFFER_SIZE, stdin);
		answer[strcspn(answer, "\n")] = '\0';
		write(sock, answer, strlen(answer));
				
		switch (answer[0]) {//1.ä�� 2.���̺���ȸ 3.��������
			case '1':{
				printf("1��\n");
				printf("�޼����� �Է����ּ���\n");
				loopControl = 0;
				break;//�극��ũ�� �Լ��������� �ٷ� ä������
			}
			case '2':{
				printf("2��\n");
				receiveData(sock);//���̺����� �޾ƿ��� �Լ�
				continue;
			}
			case '3':{
				printf("3��\n");
				printf("������ ���Ͻô� ���̵� �Է����ּ���\n");
				fgets(username, BUFFER_SIZE, stdin);
				username[strcspn(username, "\n")] = '\0';
				write(sock, username, strlen(username));
				printf("[%s] �����Ǿ����ϴ�\n",username);
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
        printf("���ӹ�� : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr(argv[1]);
    server_addr.sin_port=htons(atoi(argv[2]));
	if(connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr))==-1){
        printf("connect()����\n");
	} else{
		printf("���� ����\n");
	}
	while(loopControl){
	    printf("1.�α��� 2.ȸ������\n");
	    printf("�����Ͻ� ���񽺸� �Է����ּ���.\n");

		//�α��κκ�
		fgets(answer, BUFFER_SIZE, stdin);
		answer[strcspn(answer, "\n")] = '\0';
		write(sock, answer, strlen(answer));

		switch (answer[0]) {//1.�α��� 2.ȸ������
			case '1':{
			loginresult = loginProcess(username, password, sock);
			if(loginresult==1){//�Ϲ����� ����1
				loopControl = 0;
				break;
			}
			else if(loginresult==2){//������ ����2
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
		printf("fork����\n");
		close(sock);
		return 0;
	}
    if(pid==0){ //�ڽ����μ���
        writeChat(sock, buf);
	} else{ //�θ����μ���
        readChat(sock, buf);
	}
    close(sock);
    return 0;
}
