TARGET=./main.out
CC=g++
SRCS=\
	./*.cpp
LIBS=\
	 -I /opt/homebrew/Cellar/libusb/1.0.26/include -L /opt/homebrew/Cellar/libusb/1.0.26/lib -lusb-1.0
    #-ljpeg # Добавить  -lmmal -lmmal_core -lmmal_util, если не будет работать

# На mac OS он не подтягивает автоматически библиотеки надо прямо путь указать
#JPEGLIB=-I /opt/homebrew/Cellar/jpeg-turbo/2.1.4/include -L /opt/homebrew/Cellar/jpeg-turbo/2.1.4/lib

STD=-std=c++17
#WER=#-Wall -Wextra -Werror -ansi

all: clean $(TARGET)

$(TARGET): 
	$(CC) $(STD) $(LIBS) -lm -o $(TARGET) $(SRCS)

build: $(TARGET)

clean:
	rm -rf $(TARGET)
#Compilador
#CC     = gcc

#Diretórios de busca
#SHDIR   = -I Driver/Inc

#Flags
#LDFLAGS   = -L /libusb-1.0
#LDFLAGS  += -lusb-1.0 
#LDFLAGS  += -lm 
#CFLAGS    = -ansi
#CFLAGS   = -pedantic
#CFLAGS   += -Wall

#Executável
#EXEC = a.out

# Arquivos fonte
#SRC    = $(wildcard Usr/*.c)
#SRC   += $(wildcard Driver/Src/*.c)

# Arquivos objeto
#COBJ   = $(SRC:.c=.o) 

#all: $(EXEC)

#$(EXEC):$(COBJ)
#	@echo lincando
#	@$(CC) $(COBJ) $(LDFLAGS) -o $(EXEC)

#$(COBJ): %.o : %.c
#	@echo Compilando $<
#	@$(CC) $(CFLAGS) $(SHDIR) -c $< -o $@

#clean:
#	@echo removendo arquivos .o e .out
#	@rm -f $(COBJ)
#	@rm -f $(EXEC)
