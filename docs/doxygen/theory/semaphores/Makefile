
build: build/process_a build/process_b

build/process_a:
	mkdir -p build/
	gcc process_a.c common.c -o build/process_a

build/process_b:
	mkdir -p build/
	gcc process_b.c common.c -o build/process_b

clean:
	rm -rf build/

.PHONY: build clean
