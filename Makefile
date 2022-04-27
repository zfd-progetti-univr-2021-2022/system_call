
build:
	mkdir -p dist
	cd src && rm -rf obj/
	cd src && $(MAKE) CFLAGS="-Wall -std=gnu99 -g -D_GLIBCXX_DEBUG -DDEBUG=3"
	cp src/client_0 dist/.
	cp src/server dist/.

docs:
	cd docs/doxygen && doxygen Doxyfile

clean:
	rm -rf dist/
	cd src && make clean

.PHONY: build docs clean
