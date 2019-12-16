#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

int quit = false;

void rl_cb(char* line)
{
    if (NULL == line) {
        quit = true;
        return;
    }

    if (strlen(line) > 0) add_history(line);

    printf("You typed:\n%s\n", line);
    free(line);
}

int main()
{
    struct timeval to;
    const char* prompt = "# ";

    rl_callback_handler_install(prompt, (rl_vcpfunc_t*)&rl_cb);

    to.tv_sec = 1;
    to.tv_usec = 10000;

    rl_callback_read_char();
    while (1) {
        if (quit) break;
        //select(1, NULL, NULL, NULL, &to);
        usleep(1000000);
        rl_callback_read_char();
    };
    rl_callback_handler_remove();

    return 0;
}


//#include <sys/ioctl.h>
//#include <stdio.h>
//#include <unistd.h>
//
//int main(int argc, char** argv)
//{
//    char buf[512];
//    
//    //while (true) {
//    //    struct winsize w;
//    //    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
//
//    //    printf("%d %d\n", w.ws_row, w.ws_col);
//    //}
//    return 0;  // make sure your main returns int
//}
