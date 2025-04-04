##############################################################################
# Spectravideo SVI-328 PicoROM
# (c) 2025 Markus Rautopuro
# 

ifeq (, $(shell which z88dk-z80asm))
	$(error "No z88dk-z80asm in PATH, z88dk (https://github.com/z88dk/z88dk) is required to compile")
endif
ifeq (, $(shell which /Applications/openMSX.app/Contents/MacOS/openmsx))
	$(error "openMSX not found from default location /Applications/openMSX.app, please install https://openmsx.org")
endif
ifeq (, $(shell which node))
	$(error "No node in PATH, Node.js (https://nodejs.org/en) version $(shell cat .nvmrc) is required to compile. You can use 'nvm install' (https://github.com/nvm-sh/nvm) to install the correct version")
endif
ifneq ($(shell node --version), v$(shell cat .nvmrc))
	$(error "Node.js found in path, but it's the wrong version ($(shell node --version)), version $(shell cat .nvmrc) is required to compile. Maybe run 'nvm use'?")
endif
ifeq (, $(shell which zx0))
	$(error "No zx0 in PATH, please install it https://github.com/einar-saukas/ZX0.")
endif
ifeq (, $(shell which m4))
	$(error "No m4 in PATH, please install it https://www.gnu.org/software/m4")
endif

main:
	@echo "Spectravideo SVI-328 PicoROM"
	@echo
	@echo "Please use either 'make production' to compile a production ROM or use 'make emulator' and 'make run'."

build:
	mkdir build

release:
	mkdir release

roms:
	mkdir roms

directories: build release

emulator: directories
	@echo "Compiling emulator version..."

	z88dk-z80asm -o=release/cartsim.rom -DEMULATOR=1 -m -l -b asm/rom.asm
	@mv asm/rom.lis build/rom.lis
	@rm asm/rom.o
	@scripts/synthetic_opcodes.sh build/rom.lis

	@mv release/cartsim.map build/cartsim.map

	@echo "Emulator version (release/cartsim.rom) compiled, use 'make run' to launch the emulator"

production: directories 
	@echo "Compiling a production ROM..."

	z88dk-z80asm -o=release/cart.rom -DEMULATOR=1 -m -l -b asm/rom.asm
	@mv asm/rom.lis build/rom.lis
	@rm asm/rom.o
	@scripts/synthetic_opcodes.sh build/rom.lis

	@mv release/cart.map build/cart.map
	xxd -n ROM -i release/cart.rom > release/rom.h

	@echo "${ROM_VERSION} kB version compiled (release/cart${ROM_VERSION}.rom)"

run:
	@echo "Launching emulator version in openMSX..."
	scripts/extract_breakpoints.sh > build/setbreakpoints.tcl
	scripts/extract_symbols.sh > build/symbols.lst
	@echo "debug symbols load build/symbols.lst" >> build/setbreakpoints.tcl
	/Applications/openMSX.app/Contents/MacOS/openmsx -machine "Spectravideo_SVI-328" -cart "release/cartsim.rom" -script "build/setbreakpoints.tcl"

clean: build
	rm build/*
	rm release/*