#include "chat.h"

sqlite3 *db;
int serv_sock;
void signalHandler(int sig);
int idOverlap(const char *username);
int viewTableData(int clnt_sock);
//db����
int init_db() {
	char *db_path = "chat.db";//db���

	if (sqlite3_open(db_path, &db) == SQLITE_OK) {
		printf("db���� ����\n");
		return 1;
	} else{
		printf("db���� ����\n");
		return 0;
	}
}

void signalHandler(int sig) {
    if (sig == SIGINT) {
        printf("\n��Ʈ��c �Էµ�.������ �����մϴ�.\n");
        close(serv_sock); // ���� ������ �ݰ� ����
        exit(0);
    }
}

//dbȸ������ insert
int registerHandler(const char *username, const char *password,const char *answer) {
	char *db_path = "chat.db";
	const char *query;
	const char *adminquery;
    int result = 0;
    int rc = 0;

	query = "INSERT INTO users (username, password, admin) VALUES(?, ?, 0)";
	
	adminquery = "INSERT INTO users (username, password, admin) VALUES(?, ?, 1)";
	if(strcmp(answer, "1803")==0){//�����ں�й�ȣ
		printf("������\n");
		query = adminquery;//�����ڸ� adminquery�� ����
	}
	sqlite3_stmt *stmt;    
	init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        printf("sqlite �غ� ����\n");
        return 0;
    } else{
        printf("prepare����\n");
    }
	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
	printf("����:%s\n", query);//username,password ���ε�
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result =1;
    } else{
        printf("result= %d\n", result);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

//���̵��ߺ�Ȯ���Լ�
int idOverlap(const char *username) {
    char *db_path = "chat.db";
    const char *query;
    int result = 0;
    int rc = 0;

    query = "SELECT EXISTS (SELECT *FROM users WHERE username=?)";//���̵� �Է¹޾� �̹� �����ϴ��� ã�� ����
    sqlite3_stmt *stmt;
    init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite �غ� ����\n");
        return 0;
    } else{
        printf("prepare����\n");
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);//�Է¹������̵� ���ε�

    printf("����:%s\n", query);
    if (sqlite3_step(stmt) == SQLITE_ROW) { 
		if (sqlite3_column_int(stmt, 0) > 0) {
	        result = 5;//1���̻� ������ 5�� ��ȯ
		}
		printf("stmt=%d\n",(short)stmt);
        printf("result= %d\n", result);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

//db �α��� select
int loginHandler(const char *username, const char *password) {
	char *db_path = "chat.db";
	const char *query;
    int result = 0;
    int rc = 0;
	int admin;
	query = "SELECT admin FROM users WHERE username = ? AND password = ?";//���̵�� ��й�ȣ�� �Է¹޾� admin�� ���� ������
	//db����
	init_db();
    sqlite3_stmt *stmt;
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);    
	printf("rc=%d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite �غ� ����\n");
        return 0;
    } else{
        printf("prepare����\n");
    }
	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    //���̵�� ��й�ȣ ���ε�
    printf("����:%s\n", query);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		admin = sqlite3_column_int(stmt, 0);//Į����ù��°��
		if(admin==1){//admin���� 1�̸� �����ڷ� 5�� ��ȯ
			result =5;
		} else if(admin==0){//admin���� 0�̸� �Ϲ������� 3�� ��ȯ
			result = 3;
		}
    } else{
        printf("result= %d\n", result);
	}
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int viewTableData(int clnt_sock) {
    char *db_path = "chat.db";
	const char *query;
    int rc = 0;
    int num_columns;
    int result = 0;

    query = "SELECT username FROM users WHERE admin = 0;";//admin�� 0�� �Ϲ������� �̸��� �ҷ���
	init_db();
    sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
	printf("rc=%d\n", rc);
	if(rc!=SQLITE_OK) {
		printf("SQLITE �غ� ����\n");
		return -1;
	} else{
		printf("prepare����\n");
	}
    printf("������ ��ȸ ���:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        num_columns = sqlite3_column_count(stmt);//Į���� ������ ���� �Լ�
        for (int i = 0; i < num_columns; i++) {//�ִ�Į����������
            char column_data[1024] = " ";
			snprintf(column_data, sizeof(column_data), "%s: %s\n", sqlite3_column_name(stmt, i), sqlite3_column_text(stmt, i));
            write(clnt_sock, column_data, strlen(column_data));
            printf("%s", column_data);
        }
    }
    usleep(100000);//0.1�� ���
	write(clnt_sock, "��\n", strlen("��\n"));

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int loginProcess(char *username, char *password, int clnt_sock) {
	char temp[BUFFER_SIZE];
    char *token;
	int loginresult;
	memset(temp, 0, sizeof(temp));
	printf("temp1:%s\n",temp);//�Է¹��� ���̵�� ��й�ȣ��
	read(clnt_sock, temp, sizeof(temp));
	printf("temp2:%s\n",temp);
	token = strtok(temp, " ");//�����̽��ٸ� ����������
	username = token;

	if (token != NULL) {
		token = strtok(NULL, " ");//�ڸ�
	}
	password = token;
	loginresult = loginHandler(username, password);
		if (loginresult==3) {//�α����Լ����� 3�� ��ȯ�޾� �Ϲ�����
			write(clnt_sock, "�α��� ����", strlen("�α��� ����"));
            printf("�α��� ����\n");
			printf("�α����� ���̵�:%s, ��й�ȣ:%s\n", username, password);//�α��� �����ϸ� "�α��� ����"�� write�ϰ� 1�� ��ȯ
            memset(temp, 0, sizeof(temp));
			return 1;
		}
		else if (loginresult==5) {//�α����Լ����� 5�� ��ȯ�޾� ������
			write(clnt_sock, "������ �α��� ����", strlen("������ �α��� ����"));
            printf("������ �α��� ����\n");
            printf("�α����� ���̵�:%s, ��й�ȣ:%s\n", username, password);//�����ڰ� �α��ο� �����ϸ� "������ �α��� ����"�� write�ϰ� 2�� ��ȯ
            memset(temp, 0, sizeof(temp));
            return 2;
		} else{
			write(clnt_sock, "�α��� ����", strlen("�α��� ����"));
		    printf("�α��� ����\n");//�α��� ���н� "�α��� ����"�� write�ϰ� 0�� ��ȯ
		    memset(temp, 0, sizeof(temp));
			return 0;
		}
}

int adminMenu(char *answer, int clnt_sock){
	int loopControl = 1;
	char username[BUFFER_SIZE];
	while(loopControl){
		memset(answer, 0 , sizeof(answer));
		read(clnt_sock, answer, sizeof(answer));
		if(strcmp(answer, "1") == 0){//1�� �Է¹����� �ٷ� ä������
			loopControl = 0;
			break;
		} else if(strcmp(answer, "2") == 0) {//2�� �Է¹����� ���̺���ȸ
			system("clear");
			viewTableData(clnt_sock);
			continue;
		} else if(strcmp(answer, "3") == 0) {//3�� �Է¹����� ���������Լ�
			read(clnt_sock, username, sizeof(username));
			printf("%s", username);
			deleteUsers(username);
			continue;	
		}
	}
}		

int deleteUsers(char *username){
    char *db_path = "chat.db";
    const char *query;
    int result = 0;
    int rc = 0;

    query = "DELETE FROM users WHERE username=? AND admin=0";//�Ϲ����� admin=0�� ���� ���̵� �Է¹޾� ����
    sqlite3_stmt *stmt;
    init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite �غ� ����\n");
        return 0;
    } else{
        printf("prepare����\n");
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);//�Է¹��� ���̵� ���ε�

    printf("����:%s\n", query);
    if (sqlite3_step(stmt) == SQLITE_DONE) {//delete�� done
		result = 1;
        printf("stmt=%d\n",(int)stmt);
        printf("result= %d\n", result);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main(int argc, char *argv[]) {
    int clnt_sock;
    int str_len;
    char *token;
    char *username;
	char *password;
	int login = 0;
	int loginresult;
	int loopControl = 1;
    char buf[BUFFER_SIZE];
	char answer[BUFFER_SIZE];
	char temp[BUFFER_SIZE];
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_size;
    pid_t pid;
    
	if(argc != 2) {
        printf("���ӹ�� : %s <��Ʈ>\n", argv[0]);
        exit(1);
    }
	signal(SIGINT, signalHandler);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1){
        printf("bind() error\n");
		return 0;
	} else{
		printf("bind ����\n");
	}
    if(listen(serv_sock, 5) == -1) {
        printf("listen() error\n");
		return 0;
	} else{
		printf("listen ����\n");
		printf("Ŭ���̾�Ʈ ���� �����...\n");
	}
	init_db();

    while(1) {
        adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_size);
        if(clnt_sock == -1){
			printf("���� ����\n");
            continue;
		} else{
            printf("���ο� Ŭ���̾�Ʈ ���ӵ�\n");
		}

        pid = fork();

        if(pid == -1) {
			printf("fork����\n");
            close(clnt_sock);
            continue;
        }
		else if(pid == 0) { // �ڽ� ���μ���
            close(serv_sock);
		while(loopControl){
			memset(answer, 0 , sizeof(answer));
			read(clnt_sock, answer, sizeof(answer));
			printf("answer:%s\n",answer);
			if(strcmp(answer,"1")==0){//�α���
			printf("if:answer:%s\n",answer);
			do{
			loginresult = loginProcess(username, password, clnt_sock);
			if(loginresult==1){
				login =1;
				loopControl =0;
				}
			else if(loginresult==2){
				login =1;
				loopControl=0;
				adminMenu(answer, clnt_sock);
				}
			}while(!login);
		}

			else if(strcmp(answer,"2")==0){//ȸ������
				memset(answer, 0, sizeof(answer));
                memset(temp, 0, sizeof(temp));
                read(clnt_sock, temp, sizeof(temp));
                printf("temp:%s\n",temp);
				token = strtok(temp, " ");
                username = token;
				//���̵� �ߺ�Ȯ��
				if(idOverlap(username)==5){//5�� ���Ϲ����� �ߺ�o
					printf("�ߺ��Ȱ��� �ֽ��ϴ�\n");
					write(clnt_sock, "Overlap", strlen("Overlap"));
					continue;
				} else{
				printf("���̵��ߺ��� �����ϴ�\n");
				printf("ȸ�������� �����մϴ�\n");
				write(clnt_sock, "NoOverlap",strlen("NoOverlap"));
				}//�ߺ��̾��ٴ°� write����
                if (token != NULL) {
                    token = strtok(NULL, " ");
                }
                password = token;
				read(clnt_sock, answer, sizeof(answer));//�����ں�й�ȣ read�κ�
                registerHandler(username, password, answer);
                memset(temp, 0, sizeof(temp));
				continue;		
			}
		}		
//�α��� ���� ��
		printf("�ڽ� PID:%d\n",(long)getpid());
        while((str_len = read(clnt_sock, buf, BUFFER_SIZE)) != 0) {
			buf[str_len] = 0;
            write(clnt_sock, buf, str_len);
			printf("%s\n",buf);
		}
        close(clnt_sock);
        printf("PID:%d Ŭ���̾�Ʈ ��������\n", (long)getpid());
        return 0;
        } else{ // �θ� ���μ���
            close(clnt_sock);
        }
    }
    close(serv_sock);
	sqlite3_close(db);
    return 0;
}
