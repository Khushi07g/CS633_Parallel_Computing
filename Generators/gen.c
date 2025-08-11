#include <stdio.h>

int main(int argc, char* argv[]) {
    int n = atoi(argv[1]);

    FILE* fp = fopen("output.bin", "wb");  
    if (!fp) {
        perror("file open failed");
        return 1;
    }

    for (int i = 0; i < n; ++i) {
        float ff = (float)(i % 100);
        fwrite(&ff, sizeof(float), 1, fp);
    }
}
