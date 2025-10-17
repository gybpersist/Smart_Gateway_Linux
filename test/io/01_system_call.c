#include <stdio.h>   // perror()
#include <fcntl.h>   // open()
#include <stdlib.h>  // exit()
#include <unistd.h>  // read()/write()/close()

/**
 * 需求：利用系统调用实现love.txt的一个备份love2.txt，并在控制台输出文件内容,
 *      且多次执行在love2.txt再累加一份内容
 */

int main(int argc, char const *argv[])
{
    // 打开读的文件 open     file discription
    int fd = open("./love.txt", O_RDONLY);
    if (fd == -1)
    {
        // 输出错误信息
        perror("打开读文件失败");
        // 失败退出 => 相当于return 1
        exit(EXIT_FAILURE);
    }

    // 打开写的文件
    int fd2 = open("love2.txt", O_CREAT | O_WRONLY | O_APPEND, 0664);
    if (fd2 == -1)
    {
        // 输出错误信息
        perror("打开写文件失败");
        exit(EXIT_FAILURE);
    }

    // 读取文件内容到内存缓存区 read
    char buf[1024];
    ssize_t read_bytes;
    while ((read_bytes = read(fd, buf, sizeof(buf))) > 0)
    {
        // 将缓存区写到标准输出 write
        write(STDOUT_FILENO, buf, read_bytes);
        // 将缓存区写到另一个文件 write
        write(fd2, buf, read_bytes);
    }

    // 如果read_bytes是-1,提示失败，关闭文件流并退出
    if (read_bytes == -1)
    {
        perror("读取文件失败");
        close(fd);
        close(fd2);
        exit(EXIT_FAILURE);
    }

    // 关闭文件 close
    close(fd);
    close(fd2);
    return 0;
}
