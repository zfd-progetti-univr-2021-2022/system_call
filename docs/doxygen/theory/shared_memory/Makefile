
build: clean build/client_shm build/server_shm

build/client_shm:
	mkdir -p build/
	gcc client_shm.c common.c -o build/client_shm

build/server_shm:
	mkdir -p build/
	gcc server_shm.c common.c -o build/server_shm

clean:
	rm -rf build/

.PHONY: build clean
