CC=g++
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))
TARGET=WebServer
CFLAGS=-Wall -I../include -L../lib -lmyunp -lpthread

ALL:$(TARGET)
%.o:%.cpp
	$(CC) -c $< $(CFLAGS) -o $@

$(TARGET):$(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $@ 

clean:
	-rm -rf $(TARGET) $(OBJS)

.PHONY:clean ALL
