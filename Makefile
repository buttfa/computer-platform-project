# 测试用汇编文件的默认路径
FILE=./as/src/example.asm
# 默认仿真时间步数
TIMES=200

run:
# 步骤一：编译生成as
	cd as && make
# 步骤二：使用步骤一生成的as，编译汇编代码为二进制文件
	./as/build/as $(FILE).bin < $(FILE) 
# 步骤三：根据步骤二生成的二进制文件，进行仿真
	cd cpu && make hardware BIN_FILE=$(FILE).bin SIM_TIMES=$(TIMES)

clean:
	cd as && make clean
	cd cpu && make clean