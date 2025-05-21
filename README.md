## 项目介绍：
基于计算机系统平台课程的学习，结合课程内容，实现一个简单的CPU（支持RISC-V RV64I中的部分指令）和汇编器。

## 项目环境：
    - 操作系统：Ubuntu 24.04.2 LTS
    - Verilator版本：5.020
    - g++版本：13.3.0

## 支持的指令
#### 基于学习的目的，我们只从RV64I中选取部分指令进行实现。
> [!NOTE]
> 为了简化汇编器的实现，我们对这些支持的指令的汇编格式进行了简化，但其含义和功能与RV64I中的指令一致。

|指令|格式|功能|
|:-|:-|:-|
|ld|ld rd rs1 offset|从内存的x[rs1]+sign-extend(offset)地址处读取8个字节，写入x[rd]|
|sd|sd rs2 rs1 offset|将x[rs2]的8字节写入内存的x[rs1]+sign-extend(offset)地址处|
|add|add rd rs1 rs2|将x[rs1]和x[rs2]相加，结果保存在x[rd]中|
|addi|addi rd rs1 imm|将x[rs1]和符号位扩展的imm相加，结果保存在x[rd]中|
|lui|lui rd imm|将符号位扩展的imm左移12位后，写入x[rd]|
|sub|sub rd rs1 rs2|将x[rs1]和x[rs2]相减，结果保存在x[rd]中|
|mul|mul rd rs1 rs2|将x[rs1]和x[rs2]相乘，结果保存在x[rd]中|
|div|div rd rs1 rs2|x[rs1]除以x[rs2]，结果保存在x[rd]中|
|sll|sll rd rs1 rs2|逻辑左移，将x[rs1]左移x[rs2]的结果保存在x[rd]中|
|srl|srl rd rs1 rs2|逻辑右移，将x[rs1]右移x[rs2]的结果保存在x[rd]中|
|and|and rd rs1 rs2|将x[rs1]和x[rs2]按位与，结果保存在x[rd]中|
|or|or rd rs1 rs2|将x[rs1]和x[rs2]按位或，结果保存在x[rd]中|
|not|not rd rs1|x[rs1]按位取反，结果保存在x[rd]中（伪指令，实际被扩展为xori rd rs1 -1）|
|xor|xor rd rs1 rs2|将x[rs1]和x[rs2]按位异或，结果保存在x[rd]中|
|xori|xori rd rs1 imm|将x[rs1]和符号位扩展的imm按位异或，结果保存在x[rd]中|
|beq|beq rs1 rs2 offset|如果x[rs1]和x[rs2]相等，则将pc加上sign-extend(offset)|
|bge|bge rs1 rs2 offset|如果x[rs1]大于等于x[rs2]，则将pc加上sign-extend(offset)|
|jal|jal rd offset|将pc（已经+4）保存在x[rd]中，然后将pc加上sign-extend(offset)|
|jalr|jalr rd rs1 offset|将pc（已经+4）保存在x[rd]中，然后将x[rs1]+sign-extend(offset)的值写入pc中|
|ret|ret|从子过程返回。伪指令，实际被扩展为jalr x0 x1 0|