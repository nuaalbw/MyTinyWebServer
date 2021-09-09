CC=g++
SRCS=$(wildcard *.cpp)
TARGET=WebServer
CFLAGS=-Wall -lmyunp -lpthread -lmysqlclient

ALL:$(TARGET)
$(TARGET):$(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $@ 

clean:
	-rm -rf $(TARGET)

.PHONY:clean ALL
