/**
 * parse file meta and redirect it to a file
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * File path
 * File size
 * Chunks
 * UID
 * Chunk Hash(s) may more than one rows
 * File Hash
 */

typedef struct file_metadata {
    char file_path[1000];
    size_t file_size;
    int chunk_num;
    int UID;
    char **chunks_hash;     // chunks hash
    int *chunks_size;
    char file_hash[33];     // file hash
} file_metadata;


const char *FILE_PATH = "File path:";
const size_t FILE_PATH_LEN = 10;
const char *FILE_SIZE = "File size:";
const size_t FILE_SIZE_LEN = 10;
const char *CHUNKS = "Chunks:";
const size_t CHUNKS_LEN = 7;
const char *UID = "UID:";
const size_t UID_LEN = 4;
const char *CHUNK_HASH = "CHUNK HASH";
const size_t CHUNK_HASH_LEN = 10;
const char *WHOLE_FILE_HASH = "Whole File Hash:";
const size_t WHOLE_FILE_HASH_LEN = 16;


const char *file_metas_path = "/home/ly/fhw/data_fhw3/fhw3.txt";
const char *save_dir_path = "/home/ly/fhw/data_fhw3/data_meta";      // you should create this directory first!

void init_file_metadata(file_metadata *metadata) {
    assert(metadata != NULL);
    memset(metadata->file_path, 0, sizeof(metadata->file_path));
    metadata->file_size = -1;
    metadata->chunk_num = 0;
    metadata->UID = -1;
    metadata->chunks_hash = NULL;
    metadata->chunks_size = NULL;
    memset(metadata->file_hash, '0', sizeof(metadata->file_hash));
}

void save_metadata(struct file_metadata *metadata, int *file_idx) {
    if (metadata->file_size == -1) {     // use file_size == -1 to check a empty metadata
        return;
    }

    // check whether need to exclude this file
    if(metadata->file_size <= 2*1024UL || // 2KB
        metadata->file_size >= 2*1024*1024*1024UL){ // 2GB
        return;
    }

    // TODO:do save procedure
    char buf[1000];
    memset(buf,0,sizeof(buf));
    sprintf(buf,"%s/%d.meta",save_dir_path,*file_idx);
    (*file_idx)++;
    FILE* save_file = fopen(buf,"w");
    assert(save_file != NULL);

    // write File path field
    sprintf(buf,"File path: %s\n",metadata->file_path);
    fwrite(buf,1,strlen(buf),save_file);
    // write File size
    sprintf(buf,"File size: %ld\n",metadata->file_size);
    fwrite(buf,1,strlen(buf),save_file);
    // write chunk num
    sprintf(buf,"Chunks: %d\n",metadata->chunk_num);
    fwrite(buf,1,strlen(buf),save_file);
    // write uid
    sprintf(buf,"UID: %dabcdefghijk\n",metadata->UID);
    fwrite(buf,1,strlen(buf),save_file);
    // write chunk hash
    sprintf(buf, "Chunk Hash\tChunk Size\n");
    fwrite(buf,1,strlen(buf),save_file);
    int i = 0;
    for(;i<metadata->chunk_num;i++){
        sprintf(buf, "%s\t%d\n",metadata->chunks_hash[i], metadata->chunks_size[i]);
        fwrite(buf,1,strlen(buf),save_file);
    }
    // write file hash
    sprintf(buf,"File Hash: %s\n",metadata->file_hash);
    fwrite(buf,1,strlen(buf),save_file);
    fclose(save_file);
}

void free_metadata(struct file_metadata *metadata) {
    if (metadata->file_size == -1) {
        return;
    }
    // only needs to free chunks_hash
    // free chunks_hash
    int i = 0;
    for (; i < metadata->chunk_num; i++) {
        free(metadata->chunks_hash[i]);
        metadata->chunks_hash[i] = NULL;
    }
    free(metadata->chunks_hash);
    metadata->chunks_hash = NULL;
    // free chunks_size
    free(metadata->chunks_size);
}

int main() {
    FILE *file_metas = fopen(file_metas_path, "r");
    assert(file_metas != NULL);

    char *line = NULL;
    size_t len = 0;
    // read a line
    size_t read_len = 0;
    file_metadata metadata;
    init_file_metadata(&metadata);

    int file_idx = 1;
    while ((read_len = getline(&line, &len, file_metas)) != -1) {
        if (!strncasecmp(FILE_PATH, line, FILE_PATH_LEN)) {
            save_metadata(&metadata,&file_idx);
            // TODO: buyongif(*file_idx == xxx) break;
            free_metadata(&metadata);
            init_file_metadata(&metadata);
            // set file_path
            // remove '\n' character if has one
            if (line[read_len - 1] == '\n') line[read_len - 1] = '\0';
            // get file_path
            char *start = strchr(line, ':') + 1;
            while (isspace(*start)) start++;
            strcpy(metadata.file_path, start);
            printf("file_path:%s\n", metadata.file_path);
        } else if (!strncasecmp(FILE_SIZE, line, FILE_SIZE_LEN)) {
            // set file_size
            // remove '\n' character if has one
            if (line[read_len - 1] == '\n') line[read_len - 1] = '\0';
            // get file_sie
            char *start = strchr(line, ':') + 1;
            while (isspace(*start)) start++;
            int base = 1;
            char *p = start;
            while((*p != 'B' && *p != 'b') &&  *p != '\0'){
                if(*p == 'K' || *p == 'k'){
                    base = 1024; break;
                }else if(*p == 'M' || *p == 'm'){
                    base = 1024 * 1024; break;
                }
                p++;
            }
            *p = '\0';
            metadata.file_size = atoll(start) * base;
            printf("file_size:%ld\n", metadata.file_size);
        } else if (!strncasecmp(CHUNKS, line, CHUNKS_LEN)) {
            // remove '\n' character if has one
            if (line[read_len - 1] == '\n') line[read_len - 1] = '\0';
            // get chunk len
            char *start = strchr(line, ':') + 1;
            while (isspace(*start)) start++;
            metadata.chunk_num = atoi(start);
            printf("chunk_num:%d\n", metadata.chunk_num);
        } else if (!strncasecmp(UID, line, UID_LEN)) {
            // remove '\n' character if has one
            if (line[read_len - 1] == '\n') line[read_len - 1] = '\0';
            // get uid
            char *start = strchr(line, ':') + 1;
            while (isspace(*start)) start++;
            metadata.UID = atoi(start);
            printf("UID:%d\n", metadata.UID);
        } else if (!strncasecmp(CHUNK_HASH, line, CHUNK_HASH_LEN)) {
            // malloc chunk_hash array
            metadata.chunks_hash = malloc(sizeof(char *) * metadata.chunk_num);
            metadata.chunks_size = malloc(sizeof(int) * metadata.chunk_num);

            int i = 0;
            for (; i < metadata.chunk_num; i++) {
                read_len = getline(&line, &len, file_metas);
                assert(read_len != -1);

                metadata.chunks_hash[i] = malloc(33 * sizeof(char));
                memset(metadata.chunks_hash[i],'0',33*sizeof(char));
                metadata.chunks_hash[i][32] = '\0';
                char hash[13];      // because hash has 12 chars
                memset(hash, 0, sizeof(hash));

                char *p1 = metadata.chunks_hash[i] + 20;
                char *p2 = line;
                while (!isspace(*p2)) {
                    if (*p2 != ':') {
                        *p1 = *p2;
                        p1++;
                    }
                    p2++;
                }

                // get chunk size
                while(isspace(*p2)) p2++;
                char *p3 = p2;
                while(isdigit(*p3)) p3++;
                *p3 = '\0';
                metadata.chunks_size[i] = atoi(p2);
                printf("chunk [%d] hash %s\n", i, metadata.chunks_hash[i]);
                printf("chunk [%d] size %d\n", i, metadata.chunks_size[i]);
            }
        } else if (!strncasecmp(WHOLE_FILE_HASH, line, WHOLE_FILE_HASH_LEN)) {
            // get file hash
            // remove '\n' character if has one
            if (line[read_len - 1] == '\n') line[read_len - 1] = '\0';
            // get file hash
            char *start = strchr(line, ':') + 1;
            while (isspace(*start)) start++;
            metadata.file_hash[32] = '\0';
            strcpy(metadata.file_hash+20, start);
            printf("file_hash:%s\n\n", metadata.file_hash);
//            pause();
        }
    }
    // final file
    save_metadata(&metadata,&file_idx);
    free_metadata(&metadata);

    fclose(file_metas);
    return 0;
}
