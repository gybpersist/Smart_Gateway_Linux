#---------------------------
# <目标>: <前置依赖>
# (tab) <需要执行的命令>
# $@ 目标
# $^ 所有前置依赖
# - 忽略错误
# .PHONY: clean
#---------------------------

# main: main.c hello.c
# 	gcc -o main main.c hello.c
# 	./main
# 	rm -f main

# # 变量定义
# obj:= main.c hello.c
# # 目标定义
# main: $(obj)
# 	gcc -o main $(obj)
# 	./main
# 	rm -f main

# 变量定义
obj:= main.c hello.c
# 目标定义
main: $(obj)
	-gcc -o $@ $^
	-./$@
	-rm -f $@

# 伪目标（clean）
.PHONY: clean
clean:
	-rm -f main

