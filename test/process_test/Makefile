# 01_fork 测试
fork: 01_fork.c
	gcc -o fork 01_fork.c
	./fork
	rm -f fork

# 02_other 测试
other: 02_other.c
	-gcc $^ -o $@

# 02_execve 测试
execve: 02_execve.c other
	-gcc $< -o $@
	-./$@
	-rm $@ $(word 2, $^)
