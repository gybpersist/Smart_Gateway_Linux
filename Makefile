# log_test 测试
log := ./thirdparty/log/log.c
log_test: ./test/log_test/log_test.c $(log)
	gcc -o log_test $^ -I thirdparty
	./log_test
	rm log_test

# cJSON_test 测试
cJSON := ./thirdparty/cJSON/cJSON.c
cJSON_test: ./test/cJSON_test/cJSON_test.c $(cJSON) $(log)
	gcc -o cJSON_test $^ -I thirdparty
	./cJSON_test
	rm cJSON_test
