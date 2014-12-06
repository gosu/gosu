#ifndef INCLUDE_RUBY_CONFIG_H
#define INCLUDE_RUBY_CONFIG_H 1
#if _MSC_VER != 1600
#error MSC version unmatch: _MSC_VER: 1600 is expected.
#endif
#define RUBY_MSVCRT_VERSION 100
#define STDC_HEADERS 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_LONG_LONG 1
#define HAVE_OFF_T 1
#define SIZEOF_INT 4
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF___INT64 8
#define _INTEGRAL_MAX_BITS 64
#define SIZEOF_OFF_T 8
#define SIZEOF_VOIDP 4
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_TIME_T 8
#define TIMET2NUM(v) LL2NUM(v)
#define NUM2TIMET(v) NUM2LL(v)
#define SIZEOF_RLIM_T 0
#define SIZEOF_SIZE_T 4
#define SIZEOF_PTRDIFF_T 4
#define SIZEOF_INTPTR_T 4
#define SIZEOF_UINTPTR_T 4
#define HAVE_PROTOTYPES 1
#define TOKEN_PASTE(x,y) x##y
#define HAVE_STDARG_PROTOTYPES 1
#define NORETURN(x) __declspec(noreturn) x
#define DEPRECATED(x) __declspec(deprecated) x
#define NOINLINE(x) __declspec(noinline) x
#define FUNC_STDCALL(x) __stdcall x
#define FUNC_CDECL(x) __cdecl x
#define FUNC_FASTCALL(x) __fastcall x
#define RUBY_EXTERN extern __declspec(dllimport)
#define HAVE_DECL_SYS_NERR 1
#define HAVE_LIMITS_H 1
#define HAVE_FCNTL_H 1
#define HAVE_SYS_UTIME_H 1
#define HAVE_FLOAT_H 1
#define HAVE_TIME_H 1
#define rb_pid_t int
#define rb_gid_t int
#define rb_uid_t int
#define HAVE_STRUCT_STAT_ST_RDEV 1
#define HAVE_ST_RDEV 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_STDINT_H 1
#define HAVE_INT8_T 1
#define HAVE_UINT8_T 1
#define SIZEOF_INT8_T 1
#define SIZEOF_UINT8_T 1
#define HAVE_INT16_T 1
#define HAVE_UINT16_T 1
#define SIZEOF_INT16_T 2
#define SIZEOF_UINT16_T 2
#define HAVE_INT32_T 1
#define HAVE_UINT32_T 1
#define SIZEOF_INT32_T 4
#define SIZEOF_UINT32_T 4
#define HAVE_INT64_T 1
#define HAVE_UINT64_T 1
#define SIZEOF_INT64_T 8
#define SIZEOF_UINT64_T 8
#define HAVE_INTPTR_T 1
#define HAVE_UINTPTR_T 1
#define HAVE_SSIZE_T 1
#define ssize_t int
#define PRI_LL_PREFIX "I64"
#define PRI_PIDT_PREFIX PRI_INT_PREFIX
#define GETGROUPS_T int
#define RETSIGTYPE void
#define TYPEOF_TIMEVAL_TV_SEC long
#define HAVE_ALLOCA 1
#define HAVE_DUP2 1
#define HAVE_MEMCMP 1
#define HAVE_MEMMOVE 1
#define HAVE_MKDIR 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_STRCHR 1
#define HAVE_STRSTR 1
#define HAVE_STRTOD 1
#define HAVE_STRTOL 1
#define HAVE_STRTOUL 1
#define HAVE_FLOCK 1
#define HAVE_ISNAN 1
#define HAVE_FINITE 1
#define HAVE_HYPOT 1
#define HAVE_FMOD 1
#define HAVE_FREXP 1
#define HAVE_MODF 1
#define HAVE_WAITPID 1
#define HAVE_FSYNC 1
#define HAVE_GETCWD 1
#define HAVE_TRUNCATE 1
#define HAVE_FTRUNCATE 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_TIMES 1
#define HAVE_FCNTL 1
#define HAVE_LINK 1
#define HAVE__SETJMP 1
#define HAVE_TELLDIR 1
#define HAVE_SEEKDIR 1
#define HAVE_MKTIME 1
#define HAVE_COSH 1
#define HAVE_SINH 1
#define HAVE_TANH 1
#define HAVE_SIGNBIT 1
#define HAVE_TZNAME 1
#define HAVE_DAYLIGHT 1
#define HAVE_GMTIME_R 1
#define SETPGRP_VOID 1
#define RSHIFT(x,y) ((x)>>(int)y)
#define HAVE_RB_FD_INIT 1
#define RUBY_SETJMP(env) _setjmp(env)
#define RUBY_LONGJMP(env,val) longjmp(env,val)
#define RUBY_JMP_BUF jmp_buf
#define inline __inline
#define NEED_IO_SEEK_BETWEEN_RW 1
#define STACK_GROW_DIRECTION -1
#define CANONICALIZATION_FOR_MATHN 1
#define DEFAULT_KCODE KCODE_NONE
#define LOAD_RELATIVE 1
#define DLEXT ".so"
#define EXECUTABLE_EXTS ".exe",".com",".cmd",".bat"
#define RUBY_COREDLL "msvcr100"
#define LIBRUBY_SO "msvcr100-ruby200.dll"
#if 0
!if "msvcr100-ruby200"!="$(RUBY_SO_NAME)" || "i386-mswin32_100"!="$(ARCH)-$(PLATFORM)"
config.h: nul
!endif
#endif
#endif /* INCLUDE_RUBY_CONFIG_H */
