compile:
	cd src && verilator $(TOP).v $(SRC) ../test/$(TOP).cpp --top-module $(TOP) -Mdir ../build --cc --exe --trace -CFLAGS "-g -O0" -LDFLAGS "-g"
	make -C build -f V$(TOP).mk V$(TOP)
	cp build/V$(TOP) sim/V$(TOP)

run: compile
	sim/V$(TOP)

sim: run
	gtkwave ./sim/$(TOP).vcd

test: 
	mkdir -p sim build
	make sim TOP=$(TOP) SRC="$(SRC)"

clean:
	rm -rf build sim

.PHONY: compile run sim clean test