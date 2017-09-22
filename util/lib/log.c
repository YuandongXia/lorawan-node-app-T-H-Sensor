
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifndef WIN32
#define WIN32
#endif // WIN32
#endif // __MINGW32__

#ifndef WIN32
#include <pthread.h>
#else
#  include <windows.h>
#  include <winerror.h>
#endif // WIN32

#include "log.h"

#ifndef WIN32
/* use mutex to print log */
pthread_mutex_t log_mutex;
#endif // WIN32

int log_init(void)
{
#ifndef WIN32
  int ret = pthread_mutex_init(&log_mutex, NULL);
  if (ret != 0) {
    printf("llcp.log err (%d)\n", ret);
    return -1;
  }
#endif // WIN32
  return 0;
}

int log_fini(void)
{
  return 0;
}

void log_msg(int priority, const char *format, ...)
{
#ifndef WIN32
  pthread_mutex_lock(&log_mutex);
#endif // WIN32

/*Windows doesn't support ANSI escape sequences*/
#ifndef WIN32
  switch (priority) {
    case PRIORITY_FATAL:
      printf("\033[37;41;1m");
      break;
    case PRIORITY_ERROR:
      printf("\033[31;1m");
      break;
    case PRIORITY_WARN:
      printf("\033[33;1m");
      break;
    case PRIORITY_NOTICE:
      printf("\033[34;1m");
      break;
    case PRIORITY_INFO:
      printf("\033[32m");
      break;
    default:
      printf("\033[0m");

  }
#else
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;
  WORD textAttributes;
  /* Save current attributes */
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;

  switch (priority) {
    case PRIORITY_FATAL:
      /* foreground white, background red */
      textAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED;
      break;
    case PRIORITY_ERROR:
      textAttributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
      break;
    case PRIORITY_WARN:
      /* foreground yellow */
      textAttributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
      break;
    case PRIORITY_NOTICE:
      textAttributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
      break;
    case PRIORITY_INFO:
      textAttributes = FOREGROUND_GREEN;// | FOREGROUND_INTENSITY;
      break;
    default:
      textAttributes = saved_attributes;
      break;
  }
  SetConsoleTextAttribute(hConsole, textAttributes);
#endif

  va_list va;
  va_start(va, format);
  vprintf(format, va);

#ifndef WIN32
  printf("\033[0m");
#else
  /* Restore original attributes */
  SetConsoleTextAttribute(hConsole, saved_attributes);
#endif

  printf("\n");
  fflush(stdout);

#ifndef WIN32
  pthread_mutex_unlock(&log_mutex);
#endif // WIN32
}

void log_hex(int priority, const char *buf, int len, const char *prompt, ...)
{
  int i;
#ifndef WIN32
  pthread_mutex_lock(&log_mutex);
#endif // WIN32

  priority = priority;

#ifdef WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;
  WORD textAttributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
  /* Save current attributes */
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, textAttributes);
#else
  printf("\033[1m");
#endif

  va_list va;
  va_start(va, prompt);
  vprintf(prompt, va);

  for(i=0; i<len; i++){
    printf("%02x ", (unsigned char)buf[i]);
  }
  printf("\n");
#ifdef WIN32
  /* Restore original attributes */
  SetConsoleTextAttribute(hConsole, saved_attributes);
#else
  printf("\033[0m");
#endif
#ifndef WIN32
  pthread_mutex_unlock(&log_mutex);
#endif // WIN32
}
