
build: build/receiver build/transmitter

build/receiver:
	mkdir -p build
	gcc receiver.c -o build/receiver

build/transmitter:
	mkdir -p build
	gcc transmitter.c -o build/transmitter

clean:
	rm -rf build/

.PHONY: build clean
