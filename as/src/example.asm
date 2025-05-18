label:
    ; 加法操作
    add x2 x1 x1

    ; 存储数据
    sd x2 8(x1)

    ; 跳转条件分支（偏移地址 0x20）
    beq x2 x1 0x20

    ; 无条件跳转（偏移地址 0x24）
    jal 0x24

    ; 返回（等价于 jalr x0, x1, 0）
    ret

    ; 偏移到某地址
    jr 0x30

array:
    1 ; 32位
    2
    3
    4
    5