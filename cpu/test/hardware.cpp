#include "hardware.hpp"
#include "Vhardware.h"
#include <verilated_vcd_c.h>

int main() {
    Verilated::traceEverOn(true); // 开启波形跟踪
    Vhardware hardware;
    VerilatedVcdC trace;       // 实例化波形跟踪对象
    hardware.trace(&trace, 5); // 将波形跟踪对象与仿真模型关联
    trace.open("./sim/hardware.vcd"); // 打开波形文件

    hardware::write_64bits(&hardware, 0x00, 0x1111222233334444);
    hardware::write_64bits(&hardware, 0x08, 0x5555666677778888);

    hardware.clk = 1;
    for (int i = 0; i < 100; i++) {
        hardware.clk = !hardware.clk;
        hardware.eval();
        trace.dump(i);
    }

    trace.close();
}