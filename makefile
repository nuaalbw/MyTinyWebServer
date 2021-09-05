CC=g++
SRCS=$(wildcard *.cpp)
TARGET=WebServer
CFLAGS=-Wall -lmyunp -lpthread

ALL:$(TARGET)
$(TARGET):$(SRCS)
	$(CC) $(SRCS) $(CFLAGS) -o $@ 

clean:
	-rm -rf $(TARGET)

.PHONY:clean ALL
