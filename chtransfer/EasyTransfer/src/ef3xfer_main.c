/*
 *
 * (c) 2013 Thomas Giesel
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Thomas Giesel skoe@directbox.com
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ef3xfer.h"
#include "chacolib.h"

static void usage(const char* p_str_prg);
static void log_str(const char* str);
static void log_progress(int percent, int b_gui_only);

/*****************************************************************************/
static int m_argc;
static char** m_argv;
static int m_n_next_arg;


/*****************************************************************************/
static void usage(const char* p_str_prg)
{
    printf("chxfer (%s)", VERSION);
    printf(" - Transfer data to a Chameleon over USB.\n\n");
    printf("Usage: %s <action> [options]\n", p_str_prg);
    printf("Actions:\n"
           "  -h       --help           Print this help and exit\n"
           "  -x FILE  --exec FILE      Send a PRG file and execute it\n"
           "  -w FILE  --write FILE     Write a disk image (d64)\n"
           "           --format         format disk before writing\n"
           "           --verify         verify disk after writing\n"
           "           --usbtest        Perform USB connection test\n"
           "\nOptions to be used with --write-disk:\n"
           "  -d NUM   --drive          Drive number to be used (8)\n"
          );
}


/*****************************************************************************/
static void log_str(const char* str)
{
    fprintf(stderr, "%s", str);
    fflush(stderr);
}


/*****************************************************************************/
/*
 */
static void log_progress(int percent, int b_gui_only)
{
    if (b_gui_only)
        return;

    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;

    fprintf(stderr, "\r%3d%%", percent);
    fflush(stderr);
}

/*****************************************************************************/
static const char* get_next_arg(void)
{
    if (m_n_next_arg < m_argc)
        return m_argv[m_n_next_arg++];
    else
        return NULL;
}

int cleanup(int n) {
    chameleon_close();
    return n;
}
/*****************************************************************************/
int main(int argc, char** argv)
{
    const char* p_write_filename = NULL;
    const char* p_exec_filename = NULL;
    int         do_usb_test = 0;
    int         n_actions = 0;
    int         n_drv = 8;
    int         doformat = 0;
    int         doverify = 0;
    char dummy;
    const char* arg;

    m_argc = argc;
    m_argv = argv;
    m_n_next_arg = 1;

    /* default callback functions for stdout */
    ef3xfer_set_callbacks(log_str, log_progress);
    if (argc == 1)
    {
        usage(argv[0]);
        return 0;
    }

    while ((arg = get_next_arg()) != NULL)
    {
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }
        else if (strcmp(arg, "-x") == 0 || strcmp(arg, "--exec") == 0)
        {
            p_exec_filename = get_next_arg();
            n_actions++;
        }
        else if (strcmp(arg, "-w") == 0 || strcmp(arg, "--write") == 0)
        {
            p_write_filename = get_next_arg();
            n_actions++;
        }
        else if (strcmp(arg, "--format") == 0)
        {
            doformat = 1;
            n_actions++;
        }
        else if (strcmp(arg, "--verify") == 0)
        {
            doverify = 1;
            n_actions++;
        }
        else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--drive") == 0)
        {
            arg = get_next_arg();
            if (!arg)
            {
                fprintf(stderr, "*** Drive number missing\n");
                return 1;
            }
            if (sscanf(arg, "%d%c", &n_drv, &dummy) != 1 ||
                n_drv < 0 || n_drv > 15)
            {
                fprintf(stderr, "*** Bad drive number\n");
                return 1;
            }
        }
        else if (strcmp(arg, "--usbtest") == 0)
        {
            do_usb_test = 1;
            n_actions++;
        }
        else
        {
            fprintf(stderr, "*** Unknown action: %s\n", arg);
            return 1;
        }
    }

    if (n_actions > 1)
    {
        fprintf(stderr, "*** Too many actions, use only one at once.\n");
        return 1;
    }

    if (chameleon_init() < 0) {
        fprintf(stderr, ("initialization failed.\n"));
        exit(cleanup(EXIT_FAILURE));
    }
    if (p_exec_filename)
        return !ef3xfer_transfer_prg(p_exec_filename);
    else if (p_write_filename)
        return !ef3xfer_d64_write(p_write_filename, n_drv, doformat, doverify);
    else if (do_usb_test)
        return ef3xfer_usb_test();

    return EXIT_SUCCESS;
}
