#Makefile example

CC=g++
CPPFLAGS= -fPIC
LINKFLAGS = -shared
LIBFLAGS = -la -lb -lc

EXE=libexample.so
OBJS=a.o b.o c.o
SRCS=a.cpp b.cpp c.cpp

OBJ_DIR=objs
SRC_DIR=src
OBJ=$(addprefix $(OBJ_DIR)/, $(OBJS))

app: clean dir $(EXE) strip

${EXE}: $(OBJ)
	@echo $(OBJ)
	$(CC) $(CPPFLAGS) $(LINKFLAGS) $^ -o $@ $(LIBFLAGS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

.PHONY = dir clean strip
clean:
	-$(RM) -r $(OBJ_DIR) $(EXE) 
dir: 
	-mkdir $(OBJ_DIR)
strip:
	strip ${EXE}