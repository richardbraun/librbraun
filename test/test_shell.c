/*
 * Copyright (c) 2015 Richard Braun.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

        if (ret) {
            error_die(ret);
        }
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
