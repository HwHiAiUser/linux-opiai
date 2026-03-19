#ifndef _LINUX_HISI_SECUREC_H
#define _LINUX_HISI_SECUREC_H

#include <linux/kernel.h>
#include <linux/minmax.h>
#include <linux/securectype.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/stdarg.h>

#define memset_s __opiai_memset_s
static inline errno_t __opiai_memset_s(void *dest, size_t destMax, int c, size_t count)
{
	if (!dest || count > destMax)
		return ERANGE;
	memset(dest, c, count);
	return EOK;
}

#define memmove_s __opiai_memmove_s
static inline errno_t __opiai_memmove_s(void *dest, size_t destMax, const void *src, size_t count)
{
	if (!dest || !src || count > destMax)
		return ERANGE;
	memmove(dest, src, count);
	return EOK;
}

#define memcpy_s __opiai_memcpy_s
static inline errno_t __opiai_memcpy_s(void *dest, size_t destMax, const void *src, size_t count)
{
	if (!dest || !src || count > destMax)
		return ERANGE;
	memcpy(dest, src, count);
	return EOK;
}

#define strcpy_s __opiai_strcpy_s
static inline errno_t __opiai_strcpy_s(char *dest, size_t destMax, const char *src)
{
	size_t len;

	if (!dest || !src || !destMax)
		return EINVAL;
	len = strnlen(src, destMax);
	if (len >= destMax) {
		dest[0] = '\0';
		return ERANGE;
	}
	memcpy(dest, src, len + 1);
	return EOK;
}

#define strncpy_s __opiai_strncpy_s
static inline errno_t __opiai_strncpy_s(char *dest, size_t destMax, const char *src, size_t count)
{
	size_t copy;

	if (!dest || !src || !destMax)
		return EINVAL;
	copy = min(count, destMax - 1);
	memcpy(dest, src, copy);
	dest[copy] = '\0';
	return EOK;
}

#define strcat_s __opiai_strcat_s
static inline errno_t __opiai_strcat_s(char *dest, size_t destMax, const char *src)
{
	size_t dst_len;
	size_t src_len;

	if (!dest || !src || !destMax)
		return EINVAL;
	dst_len = strnlen(dest, destMax);
	src_len = strlen(src);
	if (dst_len >= destMax || src_len > destMax - dst_len - 1)
		return ERANGE;
	memcpy(dest + dst_len, src, src_len + 1);
	return EOK;
}

#define strncat_s __opiai_strncat_s
static inline errno_t __opiai_strncat_s(char *dest, size_t destMax, const char *src, size_t count)
{
	size_t dst_len;
	size_t src_len;
	size_t copy;

	if (!dest || !src || !destMax)
		return EINVAL;
	dst_len = strnlen(dest, destMax);
	src_len = strlen(src);
	copy = min(count, src_len);
	if (dst_len >= destMax || copy > destMax - dst_len - 1)
		return ERANGE;
	memcpy(dest + dst_len, src, copy);
	dest[dst_len + copy] = '\0';
	return EOK;
}

#define vsprintf_s __opiai_vsprintf_s
static inline int __opiai_vsprintf_s(char *dest, size_t destMax, const char *fmt, va_list args)
{
	int ret;

	if (!dest || !fmt || !destMax)
		return -1;
	ret = vsnprintf(dest, destMax, fmt, args);
	return ret >= destMax ? -1 : ret;
}

#define sprintf_s __opiai_sprintf_s
static inline int __opiai_sprintf_s(char *dest, size_t destMax, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = __opiai_vsprintf_s(dest, destMax, fmt, args);
	va_end(args);
	return ret;
}

#define vsnprintf_s __opiai_vsnprintf_s
static inline int __opiai_vsnprintf_s(char *dest, size_t destMax, size_t count, const char *fmt, va_list args)
{
	size_t limit;
	int ret;

	if (!dest || !fmt || !destMax)
		return -1;
	limit = min(destMax, count + 1);
	ret = vsnprintf(dest, limit, fmt, args);
	return ret >= limit ? -1 : ret;
}

#define snprintf_s __opiai_snprintf_s
static inline int __opiai_snprintf_s(char *dest, size_t destMax, size_t count, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = __opiai_vsnprintf_s(dest, destMax, count, fmt, args);
	va_end(args);
	return ret;
}

#define sscanf_s __opiai_sscanf_s
static inline int __opiai_sscanf_s(const char *buf, const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = vsscanf(buf, fmt, args);
	va_end(args);
	return ret;
}

#define vsscanf_s __opiai_vsscanf_s
static inline int __opiai_vsscanf_s(const char *buf, const char *fmt, va_list args)
{
	return vsscanf(buf, fmt, args);
}

#define strtok_s __opiai_strtok_s
static inline char *__opiai_strtok_s(char *str, const char *delim, char **context)
{
	char *cursor;
	char *token;

	if (!delim || !context)
		return NULL;

	cursor = str ? str : *context;
	if (!cursor)
		return NULL;

	token = strsep(&cursor, delim);
	*context = cursor;
	return token;
}

#endif
