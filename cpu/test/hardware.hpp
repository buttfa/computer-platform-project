#ifndef __HARDWARE_HPP__
#define __HARDWARE_HPP__

#include "Vhardware.h"

using namespace std;

namespace hardware {

/**
 * @brief 向RAM指定地址写入64位的数据
 *
 * @param hardware 需要写入的RAM
 * @param addr 需要写入的地址
 * @param data 需要写入的数据
 */
inline void write_64bits(Vhardware* hardware, uint64_t addr, uint64_t data) {
    /* 初始化使能信号 */
    hardware->test_en = 1;
    hardware->test_cs = 0;
    hardware->eval();

    /* 设置写入标志位  */
    hardware->test_we = 1;
    hardware->test_oe = 0;

    /* 设置写入的地址和数据 */
    hardware->test_addr = addr; // 目标地址值
    hardware->test_data = data; // 要写入的数据

    /* 产生上升使能沿 */
    hardware->test_cs = 0;
    hardware->eval();
    hardware->test_cs = 1;
    hardware->eval();

    /* 禁用所有标志位 */
    hardware->test_en = 0;
    hardware->test_cs = 0;
    hardware->test_we = 0;
    hardware->test_oe = 0;
}

/**
 * @brief 从RAM指定地址中读取数据
 *
 * @param hardware 需要读的RAM
 * @param addr 需要读取的地址
 * @return uint64_t 读取的数据
 */
inline uint64_t read_64bits(Vhardware* hardware, uint32_t addr) {
    /* 初始化使能信号 */
    hardware->test_en = 1;
    hardware->test_cs = 0;
    hardware->eval();

    /* 设置读取标志位  */
    hardware->test_we = 0; // 写标志位禁用
    hardware->test_oe = 1; // 读标志位使能

    /* 设置读取的地址 */
    hardware->test_addr = addr;

    /* 产生上升时钟沿 */
    hardware->test_cs = 0;
    hardware->eval();
    hardware->test_cs = 1;
    hardware->eval();

    /* 读取数据 */
    uint64_t data = hardware->test_data;

    /* 禁用所有标志位 */
    hardware->test_en = 0;
    hardware->test_cs = 0;
    hardware->test_we = 0;
    hardware->test_oe = 0;

    return data;
}

} // namespace hardware

#endif