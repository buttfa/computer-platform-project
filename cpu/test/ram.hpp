#ifndef __RAM_HPP__
#define __RAM_HPP__

#include "Vram.h"

using namespace std;

namespace ram {

/**
 * @brief 向RAM指定地址写入64位的数据
 *
 * @param ram 需要写入的RAM
 * @param addr 需要写入的地址
 * @param data 需要写入的数据
 */
inline void write_64bits(Vram* ram, uint64_t addr, uint64_t data) {
    /* 初始化时钟信号 */
    ram->clk = 0;
    ram->eval();

    /* 设置写入标志位  */
    ram->cs = 1;
    ram->we = 1;
    ram->oe = 0;

    /* 设置写入的地址和数据 */
    ram->addr = addr; // 目标地址值
    ram->data = data; // 要写入的数据

    /* 产生上升时钟沿 */
    ram->clk = 1;
    ram->eval();
    ram->clk = 0;
    ram->eval();

    /* 禁用所有标志位 */
    ram->cs = 0;
    ram->we = 0;
    ram->oe = 0;
}

/**
 * @brief 从RAM指定地址中读取数据
 *
 * @param ram 需要读的RAM
 * @param addr 需要读取的地址
 * @return uint64_t 读取的数据
 */
inline uint64_t read_64bits(Vram* ram, uint32_t addr) {
    /* 初始化时钟信号 */
    ram->clk = 0;
    ram->eval();

    /* 设置读取标志位  */
    ram->cs = 1;
    ram->we = 0; // 写标志位禁用
    ram->oe = 1; // 读标志位使能

    /* 设置读取的地址 */
    ram->addr = addr;

    /* 产生上升时钟沿 */
    ram->clk = 1;
    ram->eval();
    ram->clk = 0;
    ram->eval();

    /* 读取数据 */
    uint64_t data = ram->data;

    /* 禁用所有标志位 */
    ram->cs = 0;
    ram->we = 0;
    ram->oe = 0;

    return data;
}

} // namespace ram

#endif