/*
 * Copyright (c) 2015-2018 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "../error.h"
#include "../macros.h"
#include "../shell.h"

static void
test_exit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* Calling exit directly doesn't reset the terminal */
    raise(SIGINT);
}

static void
test_top(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    system("top");
}

static void
test_add(int argc, char *argv[])
{
    int i, ret, tmp, total;

    if (argc < 3) {
        printf("shell: add: invalid arguments\n");
        return;
    }

    for (total = 0, i = 1; i < argc; i++) {
        ret = sscanf(argv[i], "%d", &tmp);

        if (ret != 1) {
            printf("shell: add: '%s': invalid argument\n", argv[i]);
            return;
        }

        total += tmp;
    }

    printf("%d\n", total);
}

static struct shell_cmd test_shell_cmds[] = {
    SHELL_CMD_INITIALIZER2("add", test_add,
                           "add <i1 i2 [i3 ...]>", "add integers",
                           "The user must pass at least two integers"),
    SHELL_CMD_INITIALIZER("top", test_top,
                          "top", "display system processes"),
    SHELL_CMD_INITIALIZER("exit", test_exit,
                          "exit", "leave the shell"),
};

int
main(int argc, char *argv[])
{
    struct termios termios;
    unsigned int i;
    int ret;

    (void)argc;
    (void)argv;

    for (i = 0; i < ARRAY_SIZE(test_shell_cmds); i++) {
        ret = shell_cmd_register(&test_shell_cmds[i]);
        error_check(ret, "shell_cmd_register");
    }

    setbuf(stdin, NULL);
    ret = tcgetattr(fileno(stdin), &termios);

    if (ret) {
        perror("tcgetattr");
        return EXIT_FAILURE;
    }

    termios.c_lflag &= ~(ICANON | ECHO);
    termios.c_cc[VMIN] = 1;
    ret = tcsetattr(fileno(stdin), TCSANOW, &termios);

    if (ret) {
        perror("tcsetattr");
        return EXIT_FAILURE;
    }

    shell_setup();
    shell_run();
    test_exit(0, NULL);
}
