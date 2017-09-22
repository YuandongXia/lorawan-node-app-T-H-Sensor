#ifndef _LLC_LOG_H
#define _LLC_LOG_H

#define DEBUG

#ifdef __MINGW32__
#ifndef WIN32
#define WIN32
#endif // WIN32
#endif // __MINGW32__

#define PRIORITY_FATAL   0
#define PRIORITY_ERROR   1
#define PRIORITY_WARN    2
#define PRIORITY_NOTICE  3
#define PRIORITY_INFO    4
#define PRIORITY_PLAIN   5

#ifdef DEBUG

int  log_init(void);
int  log_fini(void);
void log_msg(int priority, const char *format, ...);
void log_hex(int priority, const char *buf, int len, const char *prompt, ...);

#else /* DEBUG */

#define log_init() (0)
#define log_fini() (0)
#define log_msg(priority, format, ...) do {} while (0)
#define log_hex(priority, buf, len, prompt, ...) do {} while(0)

#endif /* DEBUG */

#endif
