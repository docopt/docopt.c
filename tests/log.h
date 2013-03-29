 /*
  * log.h -- macros for logging and debuging.
  *
  * Examples:
  *
  *     char *s = "foo bar";
  *     logs(s); // will print `[main:24] s == "foo bar"`
  *
  *     int i = NULL;
  *     logi(i); // will print `[main:27] i == 0`
  *
  *     logb(i == NULL); // will print `[main:29] (i == NULL) == true`
  */

#ifndef LOG_H
#define LOG_H

// int
#define logi(value) \
    printf("[%s():%d] " #value " == %i \n", __func__, __LINE__, value)

// int
#define logd(value) \
    printf("[%s():%d] " #value " == %d \n", __func__, __LINE__, value)

// boolean
#define logb(value) \
    printf("[%s():%d] (" #value ") == %s \n", __func__, __LINE__, (value ? "true" : "false"))

// unsigned
#define logu(value) \
    printf("[%s():%d] " #value " == %u \n", __func__, __LINE__, value)

// double, fixed-point notation
#define logf(value) \
    printf("[%s():%d] " #value " == %f \n", __func__, __LINE__, value)

// double, standard form
#define loge(value) \
    printf("[%s():%d] " #value " == %e \n", __func__, __LINE__, value)

// double, normal or exponential notation, whichever is more appropriate
#define logg(value) \
    printf("[%s():%d] " #value " == %g \n", __func__, __LINE__, value)

// unsigned, hexadecimal
#define logx(value) \
    printf("[%s():%d] " #value " == %x \n", __func__, __LINE__, value)

// unsigned, octal
#define logo(value) \
    printf("[%s():%d] " #value " == %o \n", __func__, __LINE__, value)

// string
#define logs(value) \
    printf("[%s():%d] " #value " == \"%s\" \n", __func__, __LINE__, value)

// char
#define logc(value) \
    printf("[%s():%d] " #value " == \'%c\' \n", __func__, __LINE__, value)

// pointer
#define logp(value) \
    printf("[%s():%d] " #value " == %p \n", __func__, __LINE__, value)

 /*
  * I was tempted to make the following definitions,
  * but it would make the value expand if it is a macro,
  * and then logd(var == NULL) will print `var == ((void *)0)`,
  * instead of nice `var == NULL`.
  *
#define log(format, value) \
    printf("\n[%s:%d] " #value " == " format " \n", __func__, __LINE__, value)

#define logd(value) log("%d", value) // int
#define logi(value) log("%i", value) // int
#define logu(value) log("%u", value) // unsigned
#define logf(value) log("%f", value) // double, fixed-point notation
#define loge(value) log("%e", value) // double, standard form
#define logg(value) log("%g", value) // double, normal or exponential notation, whichever is more appropriate
#define logx(value) log("%x", value) // unsigned, hexadecimal
#define logo(value) log("%o", value) // unsigned, octal
#define logs(value) log("%s", value) // null-terminated string
#define logc(value) log("%c", value) // char
#define logp(value) log("%p", value) // pointer
  */


#endif // guard
