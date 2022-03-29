
build:
	mkdir -p dist
	cd src && make
	cp src/client_0 dist/.
	cp src/server dist/.

clean:
	rm -rf dist/
	cd src && make clean
