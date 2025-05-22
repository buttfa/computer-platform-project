label1:
addi x1 x1 1 ; x1 = x1 + 1
addi x2 x2 2 ; x2 = x2 + 2
bge x2 x1 label1

;label1:
;addi x1 x1 1 ; x1 = x1 + 1
;addi x2 x2 1 ; x2 = x2 + 1
;beq x2 x1 label1

;addi x1 x0 7 ; x1==7
;addi x2 x0 2 ; x2==2
;sub x3 x1 x2 ; x3==5
;mul x3 x1 x2 ; x3==14

;addi x1 x0 8 ; x1==8
;addi x2 x0 2 ; x2==2
;div x3 x1 x2 ; x3==4
;mul x3 x1 x2 ; x3==14

;addi x1 x0 8 ; x1==8
;addi x2 x0 2 ; x2==2
;add x3 x1 x2 ;x3==10

;lui x1 0x12345;x1==7；结果：12345000

;addi x1 x0 12 ; x1 = 12 (0b1100)
;addi x2 x0 10 ; x2 = 10 (0b1010)
;and x3 x1 x2 ; x3 = x1 & x2 = 8
;or x4 x1 x2 ; x4 = x1 | x2 = 14
;xor x5 x1 x2 ; x5 = x1 ^ x2 = 6

;not x6 x1 ; x6 = ~x1 = -13（补码）这个补码似乎有些问题,-12的显示11

;lui x10 0x0 ; 高 20 位清零
;addi x10 x10 0x100 ; x10 = 0x00000100
;addi x11 x0 123 ; x11 = 123
;sd x11 x10 0; Mem[0x100] = x11
;addi x12 x0 0 ; x12 = 0
;ld x12 x10 0; x12 = Mem[0x100]=123

;addi x1 x0 5 ; x1 = 5
;addi x2 x0 3 ; x2 = 3
;sll x3 x1 x2 ; x3 = x1 << x2 = 40
;srl x4 x3 x2 ; x4 = x3 >> x2 = 5

;问题区处理：
;addi x1 x0 5 ; x1 = 5
;not x2 x1 ; x2 = ~x1 = 0xFFFFFFFA（补码：-6::解决

;jal x1 label1 ; x1 = PC+4，跳转到 label1
;addi x2 x0 111 ; 如果跳转成功，这句不会执行
;label1:
;addi x3 x0 123 ; 跳转落点，x3 = 123

;lui x6 0x0 ; x6 = 0x00000000
;addi x6 x6 0x10 ; x6 = 0x0000001C（label_jalr 的地址）
;jalr x5 x6 0 ; 跳转到 x6 = label_jalr，x5 = 返回地址
;addi x4 x0 222 ; 不应该执行
;label_jalr:
;addi x7 x0 0x200 ; 执行成功：x7 = 200

;addi x1 x0 1
;addi x2 x0 0xAB
;xori x1 x2 0xAB ; x1==0
