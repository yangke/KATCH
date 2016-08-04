//===-- klee_init_env.c ---------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "klee/klee.h"
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include "fd.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

static void __emit_error(const char *msg) {
  klee_report_error(__FILE__, __LINE__, msg, "user.err");
}

/* Helper function that converts a string to an integer, and
   terminates the program with an error message is the string is not a
   proper number */   
static long int __str_to_int(char *s, const char *error_msg) {
  long int res = 0;
  char c;

  if (!*s) __emit_error(error_msg);

  while ((c = *s++)) {
    if (c == '\0') {
      break;
    } else if (c>='0' && c<='9') {
      res = res*10 + (c - '0');
    } else {
      __emit_error(error_msg);
    }
  }
  return res;
}

static int __isprint(const char c) {
  /* Assume ASCII */
  return (32 <= c && c <= 126);
}

static int __streq(const char *a, const char *b) {
  while (*a == *b) {
    if (!*a)
      return 1;
    a++;
    b++;
  }
  return 0;
}

static char *__get_sym_str(int numChars, char *name) {
  int i;
  char *s = malloc(numChars+1);
  klee_mark_global(s);
  klee_make_symbolic(s, numChars+1, name);

  for (i=0; i<numChars; i++)
    klee_prefer_cex(s, __isprint(s[i]));
  
  s[numChars] = '\0';
  return s;
}

static void __add_arg(int *argc, char **argv, char *arg, int argcMax) {
  if (*argc==argcMax) {
    __emit_error("too many arguments for klee_init_env");
  } else {
    argv[*argc] = arg;
    (*argc)++;
  }
}

/* KATCH argument handling code
 *
 * The getopt* model may add extra arguments to the command line,
 * which need to be treated differently here. They are left concrete
 * to avoid going into an infinite loop. runFunctionAsMain uses
 * a pseudo-argument (argv[1]) to tell this model how many arguments
 * should be left concrete
 *
 * This function makes two changes to the argument list to give KATCH
 * more flexibility:
 * 1. it adds an extra character at the end of each argument (default 0)
 * 2. it adds and extra argument at the end of the argument list (default "YY\0")
 */
void klee_init_env(int* argcPtr, char*** argvPtr) {
  int argc = *argcPtr;
  char** argv = *argvPtr;

  static char extra_arg[] = "YY\0";

  char** ext_argv = (char**) malloc((argc+1) * sizeof(*argv));
  klee_mark_global(ext_argv);

  int q, realq, concrete_args;
  concrete_args = argv[1][0]-'0';
  printf("[klee_init_env] using %d concrete arguments\n", concrete_args);
  for (q = 0; q < argc; ++q) {
    /* skip the pseudo-argument */
    if (1 == q)
      continue;
    realq = q;
    if (q > 1)
      --realq;

    /* leave an extra symbolic character at the end for KATCH */
    size_t len = strlen(argv[q])+2;
    ext_argv[realq] = (char*) malloc(len);
    strcpy(ext_argv[realq], argv[q]);
    printf("[klee_init_env] argv[%d] = %s\n", realq, ext_argv[realq]);
    if (realq > concrete_args) {
      klee_make_symbolic(ext_argv[realq], len, "argv");
      size_t j;
      for (j = 0; j <= len - 2; j++)
        klee_prefer_cex(ext_argv[realq], ext_argv[realq][j] == argv[q][j]);
    }

    /* the terminating null is concrete to avoid spurious overruns */
    ext_argv[realq][len-1] = 0;
  }

  /* update argc after removing the pseudo-argument */
  --argc;
  --(*argcPtr);

  /* add and extra argument for KATCH */
  klee_make_symbolic(extra_arg, sizeof(extra_arg), "extra_argv");
  extra_arg[sizeof(extra_arg)-1] = 0;
  ext_argv[argc] = extra_arg;

  /* terminating NULL for the argument list */
  ext_argv[argc+1] = 0;

  /* argc may be increased by at most 1 by KATCH */
  klee_make_symbolic(argcPtr, sizeof(int), "argc");
  klee_assume(0 < *argcPtr && *argcPtr <= argc+1);

  *argvPtr = ext_argv;

  unsigned sym_files = 0, sym_file_len = 0;

  int sym_stdout_flag = 0;
  int save_all_writes_flag = 0;
  int fd_fail = 0;

  klee_init_fds(sym_files, sym_file_len, 
		sym_stdout_flag, save_all_writes_flag, 
		fd_fail);
}

