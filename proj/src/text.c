#include <lcom/lcf.h>
#include <math.h>

#include "bmp.h"
#include "text.h"
#include "RTC.h"

static Bitmap * numbers[10];

void init_numbers()
{
    numbers[0] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_0.bmp");
    numbers[1] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_1.bmp");
    numbers[2] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_2.bmp");
    numbers[3] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_3.bmp");
    numbers[4] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_4.bmp");
    numbers[5] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_5.bmp");
    numbers[6] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_6.bmp");
    numbers[7] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_7.bmp");
    numbers[8] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_8.bmp");
    numbers[9] = loadBitmap("/home/lcom/labs/proj/bmp/Alphabet/Number_9.bmp");
}

void print_int(unsigned num, unsigned x, unsigned y)
{
    unsigned digits;
    char * text = NULL;
    if(num != 0)
    {
        digits = floor(log10(num))+1;
        text = (char *) malloc(sizeof(char)*digits);
        for (unsigned int i = 0; i < digits; i++)
        {
            text[i] = num / (unsigned)pow(10,digits-i-1) + '0';
            num = num % (unsigned)pow(10,digits-i-1);
        }
    }
    else
    {
        digits = 1;
        text = (char *) malloc(sizeof(char));
        text[0] = '0';
    }
    unsigned currX = x;
    for (unsigned int i = 0; i < digits; i++)
    {
        Bitmap * char_bmp;
        switch(text[i])
        {
        case '0':
            char_bmp = numbers[0];
            break;
        case '1':
            char_bmp = numbers[1];
            break;
        case '2':
            char_bmp = numbers[2];
            break;
        case '3':
            char_bmp = numbers[3];
            break;
        case '4':
            char_bmp = numbers[4];
            break;
        case '5':
            char_bmp = numbers[5];
            break;
        case '6':
            char_bmp = numbers[6];
            break;
        case '7':
            char_bmp = numbers[7];
            break;
        case '8':
            char_bmp = numbers[8];
            break;
        case '9':
            char_bmp = numbers[9];
            break;
        default:
            char_bmp = NULL;
        }
        drawBitmap(char_bmp, currX, y, false);
        currX += 52;
    }
    free(text);
}

void free_numbers()
{
    for (unsigned int i = 0; i <= 10; i++)
        free(numbers[i]);
}

void hide_int(unsigned num, unsigned x, unsigned y)
{
    unsigned digits;
    char * text = NULL;
    if(num != 0)
    {
        digits = floor(log10(num))+1;
        text = (char *) malloc(sizeof(char)*digits);
        for (unsigned int i = 0; i < digits; i++)
        {
            text[i] = num / (unsigned)pow(10,digits-i-1) + '0';
            num = num % (unsigned)pow(10,digits-i-1);
        }
    }
    else
    {
        digits = 1;
        text = (char *) malloc(sizeof(char));
        text[0] = '0';
    }
    unsigned currX = x;
    for (unsigned int i = 0; i < digits; i++)
    {
        Bitmap * char_bmp;
        switch(text[i])
        {
        case '0':
            char_bmp = numbers[0];
            break;
        case '1':
            char_bmp = numbers[1];
            break;
        case '2':
            char_bmp = numbers[2];
            break;
        case '3':
            char_bmp = numbers[3];
            break;
        case '4':
            char_bmp = numbers[4];
            break;
        case '5':
            char_bmp = numbers[5];
            break;
        case '6':
            char_bmp = numbers[6];
            break;
        case '7':
            char_bmp = numbers[7];
            break;
        case '8':
            char_bmp = numbers[8];
            break;
        case '9':
            char_bmp = numbers[9];
            break;
        default:
            char_bmp = NULL;
        }
        hideBitmap(char_bmp, currX, y);
        currX += 52;
    }
    free(text);
}

void print_score(int score)
{
    FILE *scoresFile;
	scoresFile = fopen("/home/lcom/labs/proj/scoresFile.txt", "aw+");
	if (scoresFile == NULL)
        return;
    fprintf(scoresFile, "%d/%d/%d, %d:%d:%d - %d POINTS\n", get_day(), get_month(), get_year(), get_hour(), get_min(), get_sec(), score);
    fclose(scoresFile);
}

