/*
 * Copyright (c) 2015-2018 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <macros.h>
#include <shell.h>

struct test_iodev {
    FILE *in;
    FILE *out;
};

static void
test_iodev_init(struct test_iodev *iodev, FILE *in, FILE *out)
{
    iodev->in = in;
    iodev->out = out;
}

static int
test_getc(void *io_object)
{
    struct test_iodev *iodev;

    iodev = io_object;
    return getc(iodev->in);
}

static void
test_vfprintf(void *io_object, const char *format, va_list ap)
{
    struct test_iodev *iodev;

    iodev = io_object;
    vfprintf(iodev->out, format, ap);
}

static void
test_exit(void)
{
    /* Calling exit directly doesn't reset the terminal */
    raise(SIGINT);
}

static void
test_shell_exit(struct shell *shell __unused, int argc __unused,
                char *argv[] __unused)
{
    test_exit();
}

static void
test_shell_top(struct shell *shell __unused, int argc __unused,
               char *argv[] __unused)
{
    system("top");
}

static void
test_shell_add(struct shell *shell, int argc, char *argv[])
{
    int i, ret, tmp, total;

    if (argc < 3) {
        shell_printf(shell, "shell: add: invalid arguments\n");
        return;
    }

    for (total = 0, i = 1; i < argc; i++) {
        ret = sscanf(argv[i], "%d", &tmp);

        if (ret != 1) {
            shell_printf(shell, "shell: add: '%s': invalid argument\n",
                         argv[i]);
            return;
        }

        total += tmp;
    }

    shell_printf(shell, "%d\n", total);
}

static struct shell_cmd test_shell_cmds[] = {
    SHELL_CMD_INITIALIZER2("add", test_shell_add,
                           "add <i1 i2 [i3 ...]>", "add integers",
                           "The user must pass at least two integers"),
    SHELL_CMD_INITIALIZER("top", test_shell_top,
                          "top", "display system processes"),
    SHELL_CMD_INITIALIZER("exit", test_shell_exit,
                          "exit", "leave the shell"),
};

int
main(void)
{
    struct test_iodev iodev;
    struct termios termios;
    struct shell_cmd_set cmd_set;
    struct shell shell;
    int ret;

    test_iodev_init(&iodev, stdin, stdout);
    shell_cmd_set_init(&cmd_set);
    SHELL_REGISTER_CMDS(test_shell_cmds, &cmd_set);
    shell_init(&shell, &cmd_set, test_getc, test_vfprintf, &iodev);

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

    shell_run(&shell);
    test_exit();
}
