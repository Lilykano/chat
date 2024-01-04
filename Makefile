CC = gcc
LDFLAGS = -pthread
STD = -std=c99
SQL = -lsqlite3

SERVER_TARGET = server
CLIENT_TARGET = client


all : $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): server.c
	$(CC) $(STD) server.c  -g -o $(SERVER_TARGET) $(SQL)

$(CLIENT_TARGET): client.c
	$(CC) $(STD) client.c  -g -o $(CLIENT_TARGET) $(SQL)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
