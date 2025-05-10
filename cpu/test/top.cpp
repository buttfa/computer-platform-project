#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vtop.h"
#include <iostream>
#include <filesystem>
#include <string>
using namespace std;
namespace fs = filesystem;

#define SIM_TIME_MAX 200
vluint64_t sim_time = 0;

int main(int argc, char **argv)
{
    Vtop *top = new Vtop;

    Verilated::traceEverOn(true);                                                            // 开启波形跟踪
    VerilatedVcdC *m_trace = new VerilatedVcdC;                                              // 实例化波形跟踪对象
    top->trace(m_trace, 5);                                                                  // 将波形跟踪对象与仿真模型关联
    m_trace->open((string("./sim/") + fs::path(__FILE__).stem().string() + ".vcd").c_str()); // 打开波形文件

    top->rst = 1;
    for (sim_time = 0; sim_time < SIM_TIME_MAX; sim_time++)
    {
        top->clk ^= 1;           // 切换时钟信号 1 ^ 0 = 1 1 ^ 1 = 0 0 ^ 0= 0
        top->eval();             // 更新电路状态
        m_trace->dump(sim_time); // 将当前状态写入波形文件
        printf("tick=%d, f=%d\n", sim_time, top->f);
    }

    m_trace->close();
    return 0;
}