#ifndef DEGUG_DEF_H
#define DEGUG_DEF_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum debug_color_def
    {
        DEBUG_COLOR_BOLD = 1,
        DEBUG_COLOR_DIM = 2,
        DEBUG_COLOR_UNDERLINE = 4,
        DEBUG_COLOR_BLINK = 5,
        DEBUG_COLOR_REVERSE = 7,
        DEBUG_COLOR_HIDDEN = 8,
        DEBUG_COLOR_STRIKETHROUGH = 9,
        DEBUG_COLOR_DOUBLE_UNDERLINE = 21,

        DEBUG_COLOR_BLACK = 30,
        DEBUG_COLOR_RED = 31,
        DEBUG_COLOR_GREEN = 32,
        DEBUG_COLOR_YELLOW = 33,
        DEBUG_COLOR_BLUE = 34,
        DEBUG_COLOR_MAGENTA = 35,
        DEBUG_COLOR_CYAN = 36,
        DEBUG_COLOR_WHITE = 37,

        DEBUG_COLOR_ORANGE4 = 1058,
        DEBUG_COLOR_ORANGE4_1 = 1094,
        DEBUG_COLOR_DARKORANGE3_1 = 1166,
        DEBUG_COLOR_ORANGE3 = 1172,
        DEBUG_COLOR_ORANGERED1 = 1202,
        DEBUG_COLOR_DARKORANGE = 1208,
        DEBUG_COLOR_ORANGE1 = 1214,

    } debug_color_def_t;



#define debug_clg(color, background, text_decoration, emoji, format, ...) \
    do { \
        printf("%s  ", emoji); \
        if (color < 100) \
            printf("\033[%dm", color); \
        else if (color > 100 && color < 1000) \
            printf("\033[%d;1m", color - 100); \
        else \
            printf("\033[38;5;%dm", color - 1000); \
        if (background) \
        { \
            if (background < 100) \
                printf("\033[%dm", background + 10); \
            else if (background > 100 && background < 1000) \
                printf("\033[%d;1m", background - 100 + 10); \
            else \
                printf("\033[48;5;%dm", background - 1000); \
        } \
        if (text_decoration > DEBUG_COLOR_BOLD && text_decoration < DEBUG_COLOR_DOUBLE_UNDERLINE) \
        { \
            printf("\033[%dm", text_decoration); \
        } \
        printf("[File \"%s\"-> Function Name \"%s\" Line=%d] ",__FILE__, __FUNCTION__, __LINE__); \
        printf(format, ##__VA_ARGS__); \
        printf("\033[0m"); \
        printf("  %s \n", emoji); \
    }  while ( 0 )


#define debug_normal(format, ...) \
    do { \
        debug_clg (DEBUG_COLOR_GREEN, 0, DEBUG_COLOR_BLACK,"✅", format, ##__VA_ARGS__); \
    } while ( 0 );


#define debug_warning(format, ...) \
    do { \
        debug_clg(DEBUG_COLOR_YELLOW, 0, DEBUG_COLOR_DOUBLE_UNDERLINE, "⚠️", format, ##__VA_ARGS__); \
    } while(0);


#define debug_error(format, ...) \
    do { \
        debug_clg(DEBUG_COLOR_RED, 0, DEBUG_COLOR_BLACK, "💥", format, ##__VA_ARGS__); \
    }  while ( 0 );


    const char* debug_get_bool_status (bool _status);

#define print_new_line() printf("\n");

#ifdef __cplusplus
}
#endif

#endif


