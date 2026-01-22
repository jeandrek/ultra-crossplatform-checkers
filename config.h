#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined(__unix__) && !defined(__APPLE__) && !defined(__ANDROID__)
#define USE_X11
#endif

#if defined(__ANDROID__)
#define USE_GL_ES
#endif

#endif /* !_CONFIG_H_ */
