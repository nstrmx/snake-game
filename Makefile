CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = -I/usr/x86_64-w64-mingw32/sys-root/mingw/include/SDL2/
LIBS = -lSDL2
SRCS = snake.c
OBJS = $(SRCS:.c=.o)
MAIN = snake

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) *.o *~ $(MAIN)