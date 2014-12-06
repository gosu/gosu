#if _MSC_VER != 1200
//#error MSC version unmatch: _MSC_VER: 1200 is expected.
#endif
#define USE_WINSOCK2 1
#define STDC_HEADERS 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_MEMORY_H 1
#define HAVE_OFF_T 1
#define SIZEOF_INT 4
#define SIZEOF_SHORT 2
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 0
#define SIZEOF___INT64 8
#define SIZEOF_OFF_T 4
#define SIZEOF_VOIDP 4
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_TIME_T 4
#define HAVE_PROTOTYPES 1
#define TOKEN_PASTE(x,y) x##y
#define HAVE_STDARG_PROTOTYPES 1
#define NORETURN(x) __declspec(noreturn) x
#define RUBY_EXTERN extern __declspec(dllimport)
#define HAVE_DECL_SYS_NERR 1
#define HAVE_LIMITS_H 1
#define HAVE_FCNTL_H 1
#define HAVE_SYS_UTIME_H 1
#define HAVE_FLOAT_H 1
#define rb_pid_t int
#define rb_gid_t int
#define rb_uid_t int
#define HAVE_STRUCT_STAT_ST_RDEV 1
#define HAVE_ST_RDEV 1
#define GETGROUPS_T int
#define RETSIGTYPE void
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
#define HAVE_VSNPRINTF 1
#define HAVE_ISNAN 1
#define HAVE_FINITE 1
#define HAVE_HYPOT 1
#define HAVE_FMOD 1
#define HAVE_FREXP 1
#define HAVE_MODF 1
#define HAVE_WAITPID 1
#define HAVE_FSYNC 1
#define HAVE_GETCWD 1
#define HAVE_CHSIZE 1
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
#define HAVE_TZNAME 1
#define HAVE_DAYLIGHT 1
#define SETPGRP_VOID 1
#define RSHIFT(x,y) ((x)>>(int)y)
#define FILE_COUNT _cnt
#define FILE_READPTR _ptr
#define RUBY_SETJMP(env) _setjmp(env)
#define RUBY_LONGJMP(env,val) longjmp(env,val)
#define RUBY_JMP_BUF jmp_buf
#define inline __inline
#define NEED_IO_SEEK_BETWEEN_RW 1
#define STACK_GROW_DIRECTION -1
#define DEFAULT_KCODE KCODE_NONE
#define DLEXT ".so"
#define DLEXT2 ".dll"
#define RUBY_LIB "/lib/ruby/1.8"
#define RUBY_SITE_LIB "/lib/ruby/site_ruby"
#define RUBY_SITE_LIB2 "/lib/ruby/site_ruby/1.8"
#define RUBY_VENDOR_LIB "/lib/ruby/vendor_ruby"
#define RUBY_VENDOR_LIB2 "/lib/ruby/vendor_ruby/1.8"
#define RUBY_PLATFORM "i386-mswin32"
#define RUBY_ARCHLIB "/lib/ruby/1.8/i386-mswin32"
#define RUBY_SITE_ARCHLIB "/lib/ruby/site_ruby/1.8/i386-msvcrt"
#define RUBY_VENDOR_ARCHLIB "/lib/ruby/vendor_ruby/1.8/i386-msvcrt"
#define LIBRUBY_SO "msvcrt-ruby18.dll"
#if 0
!if "msvcrt-ruby18"!="$(RUBY_SO_NAME)" || "i386-mswin32"!="$(ARCH)-$(OS)"
config.h: nul
!endif
#endif
