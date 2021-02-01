
# 指令编译器和选项
CC=/usr/bin/gcc
CFLAGS=-Wall -std=gnu99 -g

# 目标文件
TARGET=OBJ
SRCS = main.c test_function.c uart_constants.c uart_log.c uart_method.c uart_parser.c uart_timer.c         



INC = -I./
OBJS = $(SRCS:.c=.o)
$(TARGET):$(OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^


clean:
#	rm -rf $(TARGET) $(OBJS)
	rm -rf $(OBJS)
 
%.o:%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<




