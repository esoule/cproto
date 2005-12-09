/* $Id: popen.c,v 4.2 1998/01/08 00:03:28 cthuang Exp $
 *
 * Imitate a UNIX pipe in MS-DOS.
 */
#ifdef MSDOS
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <io.h>
#include "cproto.h"

static char pipe_name[FILENAME_MAX];	/* name of the temporary file */

/* Open a pipe for reading.
 */
FILE *
popen (cmd, type)
char *cmd, *type;
{
    char *tmpdir, *argv[30], **arg, *cmdline, *s, opt[FILENAME_MAX];
    int ostdout, status;

    /* Set temporary file name. */
    if ((tmpdir = getenv("TMP")) == NULL) {
	pipe_name[0] = '\0';
    } else {
	strcpy(pipe_name, tmpdir);
	trim_path_sep(pipe_name);
	strcat(pipe_name, "/");
    }
    strcat(pipe_name, tmpnam(NULL));

    /* Split the command into an argument array. */
    cmdline = xstrdup(cmd);
    arg = argv;
    s = strtok(cmdline, " ");
    *arg++ = s;
#ifdef TURBO_CPP
    sprintf(opt, "-o%s", pipe_name);
    *arg++ = opt;
#endif
    while ((s = strtok(NULL, " ")) != NULL) {
	*arg++ = s;
    }
    *arg = NULL;
 
    /* Redirect the program's stdout. */
    ostdout = dup(fileno(stdout));
#ifdef TURBO_CPP
    freopen("nul", "w", stdout);
#else
    freopen(pipe_name, "w", stdout);
#endif
 
    /* Run the program. */
    status = spawnvp(P_WAIT, argv[0], argv);
 
    /* Restore stdout. */
    dup2(ostdout, fileno(stdout));
    free(cmdline);

    if (status != 0)
	return NULL;
 
    /* Open the intermediate file and return the stream. */
    return fopen(pipe_name, type) ;
}
 
/* Close the pipe.
 */
int
pclose (f)
FILE *f;
{
    int status;
 
    status = fclose(f);
    unlink(pipe_name);
    return status;
}
#endif	/* MSDOS */
