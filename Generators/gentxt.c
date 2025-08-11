#include <stdio.h>

int main(int argc, char* argv[]) {
    int n = atoi(argv[1]);
    int nc = atoi(argv[2]);
    FILE* fp = fopen("output.txt", "wb");  
    if (!fp) {
        perror("file open failed");
        return 1;
    }

    int x = 1;
    for (int i = 0; i < n; ++i) {
        float ff = (float)(i % 100);
        fprintf(fp, "%0.4lf ", ff);
        if (x % nc == 0) fprintf(fp, "\n", ff);
        ++x;
    }
}
