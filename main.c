#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BITDEPTH 24
#define MAXINTVAL (pow(2, (BITDEPTH - 1)) - 1)

int log_gain(FILE *audio, double multBy) {
    FILE *result;
    result = fopen("result.wav", "wb");
    while (getc(audio) != 'd') {
        fseek(audio, 0, SEEK_CUR);
    }
    fseek(audio, 7, SEEK_CUR);
    char *beforeDATA;
    long sizeBeforeDATA = ftell(audio);
    beforeDATA = (char *) malloc(sizeBeforeDATA * sizeof(char));
    fseek(audio, 0, SEEK_SET);
    fread(beforeDATA, sizeof(char), sizeBeforeDATA, audio);
    fseek(audio, sizeBeforeDATA - 4, SEEK_SET);
    for (size_t i = 0; i < sizeBeforeDATA; ++i) {
        fputc(beforeDATA[i], result);
    }
    free(beforeDATA);
    beforeDATA = NULL;
    long audiosize = getc(audio) + getc(audio) * 0x100 + getc(audio) * 0x10000 + getc(audio) * 0x1000000;
    audiosize += sizeBeforeDATA - 4;
    while(ftell(audio) < audiosize) {
        // LOG start
        int val;
        double floatval;
        double logval;
        char mask;
        short beforelog[3];
        short afterlog[3];
        beforelog[0] = getc(audio); beforelog[1] = getc(audio); beforelog[2] = getc(audio);
        val = beforelog[0] + beforelog[1] * 0x100 + beforelog[2] * 0x10000;
        if ((val / (int)pow(2, 23) % 2)) {
            val ^= 0b11111111000000000000000000000000;
        }
        floatval = (double)val / MAXINTVAL;
        mask = (floatval > 0);
        // g(x)=-ln(1-f(x)) u(f(x))+ln(1+f(x)) u(-f(x))
        logval = mask ? -(log(1 - floatval)) : (log(1 + floatval));
        logval *= multBy;
        // k(x)=(1-ℯ^(-h(x))) u(f(x))+(ℯ^(h(x))-1) u(-f(x))
        floatval = mask ? (1 - exp(-logval)) : (exp(logval) - 1);
        val = (int)(floatval * MAXINTVAL);
        afterlog[0] = val % 0x100;
        afterlog[1] = (val / 0x100) % 0x100;
        afterlog[2] = (val / 0x10000) % 0x100;
        for (size_t i = 0; i < 3; ++i) {
            fputc(afterlog[i], result);
        }
    }

    fclose(result);
    return multBy;
}

int main() {
    FILE *audio;
    double multBy;
    audio = fopen("test.wav", "rb");
    if (audio == NULL) {
        printf("FILE OPEN ERROR!");
        return 1;
    }
    printf("multBy ");
    scanf("%lf", &multBy);
    log_gain(audio, 1.013);

    fclose(audio);
    return 0;
}
