#include "chat.h"

sqlite3 *db;
int serv_sock;
void signalHandler(int sig);
int idOverlap(const char *username);
int viewTableData(int clnt_sock);
//db접속
int init_db() {
	char *db_path = "chat.db";//db경로

	if (sqlite3_open(db_path, &db) == SQLITE_OK) {
		printf("db연결 성공\n");
		return 1;
	} else{
		printf("db연결 실패\n");
		return 0;
	}
}

void signalHandler(int sig) {
    if (sig == SIGINT) {
        printf("\n컨트롤c 입력됨.서버를 종료합니다.\n");
        close(serv_sock); // 서버 소켓을 닫고 종료
        exit(0);
    }
}

//db회원가입 insert
int registerHandler(const char *username, const char *password,const char *answer) {
	char *db_path = "chat.db";
	const char *query;
	const char *adminquery;
    int result = 0;
    int rc = 0;

	query = "INSERT INTO users (username, password, admin) VALUES(?, ?, 0)";
	
	adminquery = "INSERT INTO users (username, password, admin) VALUES(?, ?, 1)";
	if(strcmp(answer, "1803")==0){//관리자비밀번호
		printf("관리자\n");
		query = adminquery;//관리자면 adminquery로 변경
	}
	sqlite3_stmt *stmt;    
	init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        printf("sqlite 준비 실패\n");
        return 0;
    } else{
        printf("prepare성공\n");
    }
	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
	printf("쿼리:%s\n", query);//username,password 바인딩
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result =1;
    } else{
        printf("result= %d\n", result);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

//아이디중복확인함수
int idOverlap(const char *username) {
    char *db_path = "chat.db";
    const char *query;
    int result = 0;
    int rc = 0;

    query = "SELECT EXISTS (SELECT *FROM users WHERE username=?)";//아이디를 입력받아 이미 존재하는지 찾는 쿼리
    sqlite3_stmt *stmt;
    init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite 준비 실패\n");
        return 0;
    } else{
        printf("prepare성공\n");
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);//입력받은아이디 바인딩

    printf("쿼리:%s\n", query);
    if (sqlite3_step(stmt) == SQLITE_ROW) { 
		if (sqlite3_column_int(stmt, 0) > 0) {
	        result = 5;//1개이상 있으면 5를 반환
		}
		printf("stmt=%d\n",(short)stmt);
        printf("result= %d\n", result);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

//db 로그인 select
int loginHandler(const char *username, const char *password) {
	char *db_path = "chat.db";
	const char *query;
    int result = 0;
    int rc = 0;
	int admin;
	query = "SELECT admin FROM users WHERE username = ? AND password = ?";//아이디와 비밀번호를 입력받아 admin의 값을 가져옴
	//db접속
	init_db();
    sqlite3_stmt *stmt;
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);    
	printf("rc=%d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite 준비 실패\n");
        return 0;
    } else{
        printf("prepare성공\n");
    }
	sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    //아이디와 비밀번호 바인딩
    printf("쿼리:%s\n", query);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		admin = sqlite3_column_int(stmt, 0);//칼럼의첫번째값
		if(admin==1){//admin값이 1이면 관리자로 5를 반환
			result =5;
		} else if(admin==0){//admin값이 0이면 일반유저로 3을 반환
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

    query = "SELECT username FROM users WHERE admin = 0;";//admin이 0인 일반유저의 이름을 불러옴
	init_db();
    sqlite3_stmt *stmt;
	rc = sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
	printf("rc=%d\n", rc);
	if(rc!=SQLITE_OK) {
		printf("SQLITE 준비 실패\n");
		return -1;
	} else{
		printf("prepare성공\n");
	}
    printf("데이터 조회 결과:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        num_columns = sqlite3_column_count(stmt);//칼럼의 갯수를 세는 함수
        for (int i = 0; i < num_columns; i++) {//최대칼럼갯수까지
            char column_data[1024] = " ";
			snprintf(column_data, sizeof(column_data), "%s: %s\n", sqlite3_column_name(stmt, i), sqlite3_column_text(stmt, i));
            write(clnt_sock, column_data, strlen(column_data));
            printf("%s", column_data);
        }
    }
    usleep(100000);//0.1초 대기
	write(clnt_sock, "끝\n", strlen("끝\n"));

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int loginProcess(char *username, char *password, int clnt_sock) {
	char temp[BUFFER_SIZE];
    char *token;
	int loginresult;
	memset(temp, 0, sizeof(temp));
	printf("temp1:%s\n",temp);//입력받은 아이디와 비밀번호를
	read(clnt_sock, temp, sizeof(temp));
	printf("temp2:%s\n",temp);
	token = strtok(temp, " ");//스페이스바를 구분점으로
	username = token;

	if (token != NULL) {
		token = strtok(NULL, " ");//자름
	}
	password = token;
	loginresult = loginHandler(username, password);
		if (loginresult==3) {//로그인함수에서 3을 반환받아 일반유저
			write(clnt_sock, "로그인 성공", strlen("로그인 성공"));
            printf("로그인 성공\n");
			printf("로그인한 아이디:%s, 비밀번호:%s\n", username, password);//로그인 성공하면 "로그인 성공"을 write하고 1을 반환
            memset(temp, 0, sizeof(temp));
			return 1;
		}
		else if (loginresult==5) {//로그인함수에서 5를 반환받아 관리자
			write(clnt_sock, "관리자 로그인 성공", strlen("관리자 로그인 성공"));
            printf("관리자 로그인 성공\n");
            printf("로그인한 아이디:%s, 비밀번호:%s\n", username, password);//관리자가 로그인에 성공하면 "관리자 로그인 성공"을 write하고 2를 반환
            memset(temp, 0, sizeof(temp));
            return 2;
		} else{
			write(clnt_sock, "로그인 실패", strlen("로그인 실패"));
		    printf("로그인 실패\n");//로그인 실패시 "로그인 실패"를 write하고 0을 반환
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
		if(strcmp(answer, "1") == 0){//1을 입력받으면 바로 채팅으로
			loopControl = 0;
			break;
		} else if(strcmp(answer, "2") == 0) {//2를 입력받으면 테이블조회
			system("clear");
			viewTableData(clnt_sock);
			continue;
		} else if(strcmp(answer, "3") == 0) {//3을 입력받으면 유저삭제함수
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

    query = "DELETE FROM users WHERE username=? AND admin=0";//일반유저 admin=0인 유저 아이디를 입력받아 삭제
    sqlite3_stmt *stmt;
    init_db();
    rc=sqlite3_prepare_v2(db, query, strlen(query), &stmt, 0);
    printf("rc=%d\n",rc);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite 준비 실패\n");
        return 0;
    } else{
        printf("prepare성공\n");
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);//입력받은 아이디 바인딩

    printf("쿼리:%s\n", query);
    if (sqlite3_step(stmt) == SQLITE_DONE) {//delete는 done
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
        printf("접속방법 : %s <포트>\n", argv[0]);
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
		printf("bind 성공\n");
	}
    if(listen(serv_sock, 5) == -1) {
        printf("listen() error\n");
		return 0;
	} else{
		printf("listen 성공\n");
		printf("클라이언트 접속 대기중...\n");
	}
	init_db();

    while(1) {
        adr_size = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_size);
        if(clnt_sock == -1){
			printf("소켓 에러\n");
            continue;
		} else{
            printf("새로운 클라이언트 접속됨\n");
		}

        pid = fork();

        if(pid == -1) {
			printf("fork에러\n");
            close(clnt_sock);
            continue;
        }
		else if(pid == 0) { // 자식 프로세스
            close(serv_sock);
		while(loopControl){
			memset(answer, 0 , sizeof(answer));
			read(clnt_sock, answer, sizeof(answer));
			printf("answer:%s\n",answer);
			if(strcmp(answer,"1")==0){//로그인
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

			else if(strcmp(answer,"2")==0){//회원가입
				memset(answer, 0, sizeof(answer));
                memset(temp, 0, sizeof(temp));
                read(clnt_sock, temp, sizeof(temp));
                printf("temp:%s\n",temp);
				token = strtok(temp, " ");
                username = token;
				//아이디 중복확인
				if(idOverlap(username)==5){//5를 리턴받으면 중복o
					printf("중복된값이 있습니다\n");
					write(clnt_sock, "Overlap", strlen("Overlap"));
					continue;
				} else{
				printf("아이디중복이 없습니다\n");
				printf("회원가입을 진행합니다\n");
				write(clnt_sock, "NoOverlap",strlen("NoOverlap"));
				}//중복이없다는걸 write해줌
                if (token != NULL) {
                    token = strtok(NULL, " ");
                }
                password = token;
				read(clnt_sock, answer, sizeof(answer));//관리자비밀번호 read부분
                registerHandler(username, password, answer);
                memset(temp, 0, sizeof(temp));
				continue;		
			}
		}		
//로그인 성공 후
		printf("자식 PID:%d\n",(long)getpid());
        while((str_len = read(clnt_sock, buf, BUFFER_SIZE)) != 0) {
			buf[str_len] = 0;
            write(clnt_sock, buf, str_len);
			printf("%s\n",buf);
		}
        close(clnt_sock);
        printf("PID:%d 클라이언트 접속종료\n", (long)getpid());
        return 0;
        } else{ // 부모 프로세스
            close(clnt_sock);
        }
    }
    close(serv_sock);
	sqlite3_close(db);
    return 0;
}
