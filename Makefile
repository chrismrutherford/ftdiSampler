CC=g++
CFLAGS= -O2 -Wall -Wextra -L. -lftd2xx -lpthread -ldl -lrt -Wl,-rpath /usr/local/lib
APP = sigAnc

all: $(APP)

$(APP): sigAnc.cpp	
	$(CC) sigAnc.cpp -o $(APP) $(CFLAGS) -I /usr/local/include
	
clean:
	rm -f *.o ; rm $(APP)
