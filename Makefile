# 默认仿真时间步数
TIMES=800

run:
# 检查FILE变量是否被设置
ifeq ($(FILE),)
	$(error FILE 变量没有被设置。 仿真测试方法: make FILE=<汇编文件路径> [TIMES=<仿真时间步数>])
endif
# 步骤一：编译生成as
	cd as && make
# 步骤二：使用步骤一生成的as，编译汇编代码为二进制文件
	./as/build/as $(FILE).bin < $(FILE) 
# 步骤三：根据步骤二生成的二进制文件，进行仿真
	cd cpu && make hardware BIN_FILE=$(FILE).bin SIM_TIMES=$(TIMES)

clean:
	cd as && make clean
	cd cpu && make clean