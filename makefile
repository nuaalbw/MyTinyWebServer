CC = g++
SRCS = $(wildcard *.cpp)
TARGET = WebServer
CFLAGS = -Wall -g -lpthread -lmysqlclient

ALL:$(TARGET)
$(TARGET):$(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $@ 

clean:
	-rm -rf $(TARGET)

.PHONY:clean ALL
