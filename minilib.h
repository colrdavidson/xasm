#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <sys/types.h>

#pragma once

typedef unsigned long long u64;
typedef uint32_t           u32;
typedef uint16_t           u16;
typedef uint8_t             u8;

typedef long long          i64;
typedef int32_t            i32;
typedef int16_t            i16;
typedef int8_t              i8;

static __inline long __syscall0(long n)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall1(long n, long a1)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall2(long n, long a1, long a2)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
						  : "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall3(long n, long a1, long a2, long a3)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3) : "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10): "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
	return ret;
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	register long r9 __asm__("r9") = a6;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
	return ret;
}

#define __scc(X) ((long) (X))
#define __syscall1(n,a) __syscall1(n,__scc(a))
#define __syscall2(n,a,b) __syscall2(n,__scc(a),__scc(b))
#define __syscall3(n,a,b,c) __syscall3(n,__scc(a),__scc(b),__scc(c))
#define __syscall4(n,a,b,c,d) __syscall4(n,__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall5(n,a,b,c,d,e) __syscall5(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __syscall6(n,a,b,c,d,e,f) __syscall6(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __syscall7(n,a,b,c,d,e,f,g) (__syscall)(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __syscall(...) __SYSCALL_DISP(__syscall,__VA_ARGS__)
#define syscall(...) __syscall_ret(__syscall(__VA_ARGS__))

#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4
#define MAP_FILE      0
#define MAP_PRIVATE   0x02
#define MAP_FIXED     0x10
#define MAP_ANONYMOUS 0x20
#define MREMAP_MAYMOVE 0x1
#define MREMAP_FIXED   0x2
#define O_RDONLY 0
#define ARCH_SET_GS	0x1001
#define ARCH_SET_FS	0x1002
#define ARCH_GET_FS	0x1003
#define ARCH_GET_GS	0x1004
#define SEEK_SET 0
#define SEEK_END 2

static long errno = 0;

static i64 __syscall_ret(u64 r) {
	if (r > -4096UL) {
		errno = -r;
		return -1;
	}
	return r;
}

static inline void dbgbreak(void) {
	asm("int $3");
}

static ssize_t write(int fd, uint8_t *buf, u64 size) {
	int ret = syscall(SYS_write, fd, buf, size);
	if (ret != size) {
		return -1;
	}
	return ret;
}

__attribute__((noreturn)) static void exit(int code) {
	syscall(SYS_exit, code);
	for (;;) { }
}

void __stack_chk_fail(void) {
	exit(1);
}

static int open(char *pathname, int flags) {
	return syscall(SYS_open, pathname, flags);
}

static int close(int fd) {
	return syscall(SYS_close, fd);
}

static i64 lseek(int fd, i64 offset, u32 flag) {
	return syscall(SYS_lseek, fd, offset, flag);
}

static i64 read(int fd, uint8_t *buf, u64 size) {
	return syscall(SYS_read, fd, buf, size);
}

static int arch_prctl(int code, u8 *addr) {
	u64 addr64;
	void *prctl_arg = &addr64;

	int ret = syscall(SYS_arch_prctl, code, prctl_arg);
	if (!ret) {
		if ((uintptr_t)addr64 != addr64) {
			return -1;
		}

		*addr = (uintptr_t)addr64;
	}

	return ret;
}

static void *mmap(void *start, size_t len, int prot, int flags, int fd, i64 off) {
	return (void *)syscall(SYS_mmap, start, len, prot, flags, fd, off);
}
static void *mremap(void *addr, size_t old_len, size_t new_len, int flags, void *new_addr) {
	return (void *)syscall(SYS_mremap, addr, old_len, new_len, flags, new_addr);
}

static void *malloc(size_t size) {
	size_t len = size + sizeof(size);
	size_t *ptr = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if ((size_t)ptr == (size_t)-1) {
		return NULL;
	}

	*ptr = len;
	return (void *)(ptr + 1);
}

static void *realloc(void *_ptr, size_t size) {
	size_t *ptr = ((size_t *)_ptr) - 1;
	size_t old_len = *ptr;

	size_t new_len = size + sizeof(size);
	size_t *new_ptr = mremap(ptr, old_len, new_len, MREMAP_MAYMOVE, NULL);
	if ((size_t)new_ptr == (size_t)-1) {
		return NULL;
	}

	*new_ptr = new_len;
	return (void *)(new_ptr + 1);
}

static int itoa(uint64_t i, uint8_t base, uint8_t *buf) {
    static const char bchars[] = "0123456789ABCDEF";

    int      pos   = 0;
    int      o_pos = 0;
    int      top   = 0;
    uint8_t tbuf[64];

    if (i == 0 || base > 16) {
        buf[0] = '0';
        buf[1] = '\0';
        return 2;
    }

    while (i != 0) {
        tbuf[pos] = bchars[i % base];
        pos++;
        i /= base;
    }
    top = pos--;

    for (o_pos = 0; o_pos < top; pos--, o_pos++) {
        buf[o_pos] = tbuf[pos];
    }

    buf[o_pos] = 0;
    return o_pos + 1;
}

static int putc(int fd, char c) {
	return write(fd, (uint8_t *)&c, 1);
}

#define _PRINT_BUFFER_LEN 128
static void print(int fd, char *c, i64 size) {
	uint8_t obuf[_PRINT_BUFFER_LEN];

	int i = 0;
	for (; *c != 0 && (size == -1 || i < size); i++) {
		if (i > _PRINT_BUFFER_LEN) {
			write(fd, obuf, i);
			i = 0;
		}

		obuf[i] = *c;
		c++;
	}

	if (i > 0) {
		write(fd, obuf, i);
	}
}

static void putn(int fd, u64 i, u8 base) {
	uint8_t tbuf[64];

	if (base > 16) return;

	int size = itoa(i, base, tbuf);
	write(fd, tbuf, size - 1);
}

static void dprintf(int fd, char *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);

	uint8_t obuf[_PRINT_BUFFER_LEN];
	u32 min_len = 0;
	for (char *c = fmt; *c != 0; c++) {
		if (*c != '%') {
			int i = 0;
			for (; *c != 0 && *c != '%'; i++) {
				if (i > _PRINT_BUFFER_LEN) {
					write(fd, obuf, i);
					i = 0;
				}

				obuf[i] = *c;
				c++;
			}

			if (i > 0) {
				write(fd, obuf, i);
			}
			c--;
			continue;
		}

		i64 precision = -1;
consume_moar:
		c++;
		switch (*c) {
		case '.': {
			c++;
			if (*c != '*') {
				continue;
			}

			precision = __builtin_va_arg(args, i64);
			goto consume_moar;
		} break;
		case '0': {
			c++;
			min_len = *c - '0';
			goto consume_moar;
		} break;
		case 's': {
			char *s = __builtin_va_arg(args, char *);
			print(fd, s, precision);
		} break;
		case 'd': {
			i64 i = __builtin_va_arg(args, i64);
			putn(fd, i, 10);
		} break;
		case 'x': {
			u64 i = __builtin_va_arg(args, u64);

			uint8_t tbuf[64];
			int sz = itoa(i, 16, tbuf);

			int pad_sz = min_len - (sz - 1);
			while (pad_sz > 0) {
				putc(fd, '0');
				pad_sz--;
			}

			write(fd, tbuf, sz - 1);
			min_len = 0;
		} break;
		case 'b': {
			u64 i = __builtin_va_arg(args, u64);

			uint8_t tbuf[64];
			int sz = itoa(i, 2, tbuf);

			int pad_sz = min_len - (sz - 1);
			while (pad_sz > 0) {
				putc(fd, '0');
				pad_sz--;
			}

			write(fd, tbuf, sz - 1);
			min_len = 0;
		} break;
		}
	}

	__builtin_va_end(args);
}

#define printf(...) do { dprintf(1, __VA_ARGS__); } while (0)
#define panic(...)  do { dprintf(2, __VA_ARGS__); exit(1); } while (0)
#define array_size(x) sizeof(x) / sizeof(*(x))
#define floor_size(addr, size) ((addr) - ((addr) % (size)))
#define round_size(addr, size) (((addr) + (size)) - ((addr) % (size)))

#define DynArr(type) struct { \
	type *data;               \
	u64 size;                 \
	u64 capacity;             \
}

#define dyn_init(a, c) {                    \
	a.data = malloc(sizeof(a.data[0]) * c); \
	a.size = 0;                             \
	a.capacity = c;                         \
}

#define dyn_append(a, v) {                                        \
	if (a.size >= a.capacity) {                                   \
		a.capacity <<= 1u;                                        \
		a.data = realloc(a.data, sizeof(a.data[0]) * a.capacity); \
	}                                                             \
	a.size += 1;                                                  \
	a.data[a.size] = v;                                           \
}

static int memeq(const void *dst, const void *src, size_t n) {
	const char *_dst = dst;
	const char *_src = src;
	for (size_t i = 0; i < n; i++) {
		if (_src[i] != _dst[i]) {
			return 0;
		}
	}

	return 1;
}

static void *memcpy(void *dst, const void *src, size_t n) {
	if (n == 0) {
		return dst;
	}

	char *_dst = dst;
	const char *_src = src;

	size_t i = n - 1;
	while (i > 0) {
		_dst[i] = _src[i];
		i--;
	}
	_dst[0] = _src[0];

	return dst;
}

static void *memset(void *dst, int val, unsigned long n) {
	char *_dst = dst;
	for (size_t i = 0; i < n; i++) {
		_dst[i] = val;
	}

	return dst;
}

int main(int, char **);

void __main(int argc, char **argv) {
	int ret = main(argc, argv);
	exit(ret);
}
