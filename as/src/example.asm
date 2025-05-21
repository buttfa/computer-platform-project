
addi x1 x0 10 ; x1 = x0 + 10 → x1 = 10


sll x2 x1 x1 ; x2 = x1 << x1 → 10 << 10 = 10240

srl x3 x2 x1 ; x3 = x2 >> x1 → 10240 >> 10 = 10

div x4 x3 x1 ; x4 = x3 / x1 → 10 / 10 = 1

jalr x5 4(x1) ; x5 = return addr, PC = x1 + 4

jalri 8 ; 等价于 jalr x1, 8(x0)