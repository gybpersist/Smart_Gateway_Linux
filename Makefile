# -g 开启gdb的debug调试（生成的二进制文件中包含调试信息）
# -O0 禁用优化（确保调试一致性）
# -Wall 显示所有警告
CFLAGS := -g -O0 -Wall
# $(CFLAGS) 开启gdb的debug调试（生成的二进制文件中包含调试信息）

# log_test 测试
log := ./thirdparty/log/log.c
log_test: ./test/log_test/log_test.c $(log)
	gcc -o log_test $^ -I thirdparty
	./log_test
	rm log_test

# cJSON_test 测试
cJSON := ./thirdparty/cJSON/cJSON.c
cJSON_test: ./test/cJSON_test/cJSON_test.c $(cJSON) $(log)
	gcc $(CFLAGS) -o cJSON_test $^ -I thirdparty
	./cJSON_test
	rm cJSON_test

# app_common_test 测试
app_common := ./app/app_common.c
app_common_test: ./test/app_common_test/app_common_test.c $(app_common) $(log)
	gcc -o app_common_test $^ -I thirdparty -I app
	./app_common_test
	rm app_common_test
