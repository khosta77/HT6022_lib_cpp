TARGET=./main.out
CC=g++
SRCS=\
	./*.cpp
LIBS=\
	-I /opt/homebrew/Cellar/libusb/1.0.26/include -L /opt/homebrew/Cellar/libusb/1.0.26/lib -lusb-1.0

STD=-std=c++17

all: clean $(TARGET)

$(TARGET): 
	$(CC) $(STD) $(LIBS) -lm -o $(TARGET) $(SRCS)

build: $(TARGET)

clean:
	rm -rf $(TARGET)
