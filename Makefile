
build:
	mkdir -p dist
	cd src && $(MAKE)
	cp src/client_0 dist/.
	cp src/server dist/.

docs:
	cd docs/doxygen && doxygen Doxyfile

clean:
	rm -rf dist/
	cd src && make clean

.PHONY: build docs clean
