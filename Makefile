all: build release install clean
.PHONY: build release install clean uninstall

build:
	mkdir -p build

release: build
	meson setup build --buildtype=release
	cd build; ninja

debug: build
	meson setup build --buildtype=debug
	cd build; ninja

install: release
	cd build; ninja install
	@echo "use 'eta' to start the REPL or 'eta <filename>.n' to run a program"

clean:
	rm -rf build

uninstall: install
	cd build; sudo ninja uninstall
