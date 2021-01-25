#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>

#define FILE_IDX_START 1


using namespace std;

int random_dev_fd = -1;

void write_file_with_random_data(const string &file_path, size_t bytes)
{
    static const size_t max_bytes_per_time = 1024 * 1024 * 30; // 每次最多写100M
    static char buf[max_bytes_per_time];
    size_t left = bytes;
    size_t cur_bytes = 0;
    FILE* out_file = fopen(file_path.c_str(),"w");
    assert(out_file != NULL);

    while (left) {
        cur_bytes = std::min(left, max_bytes_per_time);
        int ret = read(random_dev_fd, buf, cur_bytes);
        assert(ret == cur_bytes);
        fwrite(buf,1,cur_bytes,out_file);
        left -= cur_bytes;
    }
    fclose(out_file);
}

/**
 * 1 means excluding this file
 * 0 means including this file
 * @param file_bytes
 * @return
 */
int exclude_files(size_t file_bytes){
    /* Condition: if this file size is less than 2KB, we need to exclude this file*/
    if(file_bytes <= 2*1024UL ||    // 2KB
        file_bytes >= 1*1024*1024*1024UL) { // 2GB
        return 1;
    }
    return 0;
}


bool generate_file(const string &file_path, const string &file_size_str)
{
    cout << "now generate:" << file_size_str << "--->" << file_path << '\n';
    assert(file_size_str.size() >= 2);

    char c = file_size_str[file_size_str.size() - 2];
    size_t file_bytes = 0;

    if (isdigit(c)) {    // B
        file_bytes = stoll(file_size_str.substr(0, file_size_str.size() - 1));
    } else if (c == 'K' || c == 'k') { // KB
        file_bytes = stoll(file_size_str.substr(0, file_size_str.size() - 2)) * 1024;
    } else if (c == 'M' || c == 'm') { // MB
        file_bytes = stoll(file_size_str.substr(0, file_size_str.size() - 2)) * 1024 * 1024;
    }

    if(exclude_files(file_bytes)){
        return false;
    }

    write_file_with_random_data(file_path, file_bytes);
    return true;
}

/*******************************************************************
** 函数名:     folder_mkdirs
** 函数描述:   可多级建立文件夹
** 参数:       folder_path:目标文件夹路径
** 返回:       1 - 目标文件夹存在，2 - 创建失败
********************************************************************/
int folder_mkdirs(const char *folder_path)
{
    if (!access(folder_path, F_OK)) {                        /* 判断目标文件夹是否存在 */
        return 1;
    }

    char path[256];                                        /* 目标文件夹路径 */
    char *path_buf;                                        /* 目标文件夹路径指针 */
    char temp_path[256];                                   /* 存放临时文件夹路径 */
    char *temp;                                            /* 单级文件夹名称 */
    int temp_len;                                          /* 单级文件夹名称长度 */

    memset(path, 0, sizeof(path));
    memset(temp_path, 0, sizeof(temp_path));
    strcat(path, folder_path);
    path_buf = path;

    while ((temp = strsep(&path_buf, "/")) != NULL) {        /* 拆分路径 */
        temp_len = strlen(temp);
        if (0 == temp_len) {
            continue;
        }
        strcat(temp_path, "/");
        strcat(temp_path, temp);
        printf("temp_path = %s\n", temp_path);
        if (-1 == access(temp_path, F_OK)) {                 /* 不存在则创建 */
            if (-1 == mkdir(temp_path, 0777)) {
                return 2;
            }
        }
    }
    return 1;
}

int main()
{
    // some vars
    const string file_meta_path = "/home/ly/fhw/data_fhw3/fz3.txt";
    const string output_file_dir = "/home/ly/fhw/data_fhw3/data";   // note: no end "/" character

    // open file meta
    ifstream in(file_meta_path, ios_base::in);
    if (!in.is_open()) return -1;
    // open random dev
    random_dev_fd = open("/dev/urandom", O_RDONLY);
    assert(random_dev_fd != -1);

    // try to create output dir
    folder_mkdirs(output_file_dir.c_str());

    // generate
    string file_size_str;
    int file_idx = 1;
    char file_path[100];
    bool file_valid = true; // true represent omit this file
    while (getline(in, file_size_str)) {
        sprintf(file_path, "%s/%d.data", output_file_dir.c_str(), file_idx++);
        if(file_idx >= FILE_IDX_START){
            file_valid = generate_file(file_path, file_size_str);
            if(!file_valid)
                file_idx--; //if file_size <=2k, >=1G. file_name--
        }
    }

    // free resource
    close(random_dev_fd);
    in.close();
    return 0;
}