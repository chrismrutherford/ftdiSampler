CC=g++
CFLAGS= -Ofast -Wall -Wextra -L. -lftd2xx -lpthread -ldl -lrt -Wl,-rpath /usr/local/lib
SAMPLER = sigAnc
REPLAY = replay

all: $(SAMPLER) $(REPLAY)

$(SAMPLER): sigAnc.cpp
	$(CC) sigAnc.cpp -o $(SAMPLER) $(CFLAGS) -I /usr/local/include
	
$(REPLAY): replay.cpp
	$(CC) replay.cpp -o $(REPLAY) $(CFLAGS) -I /usr/local/include
clean:
	rm -f *.o ; rm $(SAMPLER)
