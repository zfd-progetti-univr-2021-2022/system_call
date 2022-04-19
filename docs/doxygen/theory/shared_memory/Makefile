
build: clean build/client build/server

build/client:
	mkdir -p build/
	gcc client.c common.c -o build/client

build/server:
	mkdir -p build/
	gcc server.c common.c -o build/server

clean:
	rm -rf build/

.PHONY: build clean
