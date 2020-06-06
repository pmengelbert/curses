CFLAGS=-Wall -g -lncurses

all: snake

install:
	mv ./snake /home/pme/bin

clean:
	rm -rf ./snake
