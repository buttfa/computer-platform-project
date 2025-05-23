    addi x1 x0 1 ; 将x1的值设为1
    addi x3 x0 10 ; 向x3中写入10

loop:
    addi x2 x2 1 ; x2自增
    mul x1 x1 x2 ; 将x2的值乘到x1中
    beq x2 x3 end ; 如果x2的值等于10，则跳转到end标签处结束运行
    beq x0 x0 loop ; 否则，跳转到loop标签处继续运行

end:
    addi x0 x0 0 ; 空指令