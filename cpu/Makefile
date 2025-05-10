compile:
	cd src && verilator $(TOP).v ../test/$(TOP).cpp --top-module $(TOP) -Mdir ../build --cc --exe --trace --debug  -CFLAGS "-g -O0" -LDFLAGS "-g"
	make -C build -f V$(TOP).mk V$(TOP)
	cp build/V$(TOP) sim/V$(TOP)

run: compile
	sim/V$(TOP)

sim: run
	gtkwave ./sim/$(TOP).vcd

clean:
	rm -rf build sim/*

module_test: 
	make sim TOP=$(MODULE_NAME)

MODULES=$(shell ls src/*.v)
cpu_test:
