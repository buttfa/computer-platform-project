compile:
	cd src && verilator $(TOP).v ../test/$(TOP).cpp --top-module $(TOP) -Mdir ../build --cc --exe --trace -CFLAGS "-g -O0 $(CFLAGS)" -LDFLAGS "-g"
	make -C build -f V$(TOP).mk V$(TOP) -j
	cp build/V$(TOP) sim/V$(TOP)

run: compile
	sim/V$(TOP)

sim: run
	gtkwave ./sim/$(TOP).vcd

test: 
	mkdir -p sim build
	make sim TOP=$(TOP)

clean:
	rm -rf build sim

hardware:
	mkdir -p sim build
# 生成仿真应用程序
	make compile TOP=hardware CFLAGS=-D__HARDWARE_RELEASE__
# 执行仿真应用程序
	cd .. && ./cpu/sim/Vhardware $(BIN_FILE) $(SIM_TIMES)
# 绘制波形
	gtkwave ./sim/hardware.vcd

.PHONY: compile run sim clean test