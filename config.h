#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined(__unix__) && !defined(__APPLE__) && !defined(__ANDROID__)
#define USE_X11
#endif

#if defined(__ANDROID__)
#define USE_GL_ES
#endif

#if !defined(__ANDROID__) && !defined(__psp__)
#define USE_BONJOUR
#endif

#endif /* !_CONFIG_H_ */
