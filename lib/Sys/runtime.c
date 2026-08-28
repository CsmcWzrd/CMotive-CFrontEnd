#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <math.h>
#include <ctype.h>
#include <wchar.h>
#include <time.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

void *CMotive_New(size_t n) { return calloc(1, n ? n : 1); }
void CMotive_Delete(void *p) { free(p); }

/* ---- CMotive standard library runtime helpers ---- */
char *cmotive_strdup_local(const char *s) { size_t n; char *p; if (!s) s = ""; n = strlen(s) + 1u; p = (char*)malloc(n); if (p) memcpy(p, s, n); return p; }
int cmotive_sys_stdio_print(const char* s) { return fputs(s ? s : "", stdout); }
int cmotive_sys_stdio_println(const char* s) { int r = fputs(s ? s : "", stdout); fputc('\n', stdout); return r; }
int cmotive_sys_stdio_puts(const char* s) { return puts(s ? s : ""); }
int cmotive_sys_stdio_putchar(int ch) { return putchar(ch); }
int cmotive_sys_stdio_flush(void) { return fflush(stdout); }

void* cmotive_sys_file_open(const char *p, const char *m) { return (void*)fopen(p ? p : "", m ? m : "rb"); }
int cmotive_sys_file_close(void *h) { if (!h) return 0; return fclose((FILE*)h); }
int64_t cmotive_sys_file_read(void *h, void *b, uint64_t n) { if (!h || !b) return -1; return (int64_t)fread(b, 1u, (size_t)n, (FILE*)h); }
int64_t cmotive_sys_file_write(void *h, void *b, uint64_t n) { if (!h || !b) return -1; return (int64_t)fwrite(b, 1u, (size_t)n, (FILE*)h); }
int cmotive_sys_file_seek(void *h, int64_t off, int origin) { if (!h) return -1; return fseek((FILE*)h, (long)off, origin); }
int64_t cmotive_sys_file_tell(void *h) { if (!h) return -1; return (int64_t)ftell((FILE*)h); }
int cmotive_sys_file_eof(void *h) { if (!h) return 1; return feof((FILE*)h); }
int cmotive_sys_file_flush(void *h) { if (!h) return -1; return fflush((FILE*)h); }
int cmotive_sys_file_remove(const char *p) { return remove(p ? p : ""); }
int cmotive_sys_file_rename(const char *a, const char *b) { return rename(a ? a : "", b ? b : ""); }

int cmotive_sys_filesystem_exists(const char* p) { struct stat st; return (p && stat(p, &st) == 0) ? 1 : 0; }
int cmotive_sys_filesystem_is_file(const char* p) { struct stat st; if (!p || stat(p, &st) != 0) return 0; return (st.st_mode & S_IFMT) == S_IFREG; }
int cmotive_sys_filesystem_is_directory(const char* p) { struct stat st; if (!p || stat(p, &st) != 0) return 0; return (st.st_mode & S_IFMT) == S_IFDIR; }
int cmotive_sys_filesystem_mkdir(const char* p) {
  if (!p) return -1;
#if defined(_WIN32)
  return _mkdir(p);
#else
  return mkdir(p, 0777);
#endif
}
int cmotive_sys_filesystem_remove(const char* p) { return remove(p ? p : ""); }
int cmotive_sys_filesystem_rename(const char *a, const char *b) { return rename(a ? a : "", b ? b : ""); }
int64_t cmotive_sys_filesystem_size(const char* p) { struct stat st; if (!p || stat(p, &st) != 0) return -1; return (int64_t)st.st_size; }
char* cmotive_sys_filesystem_current_path(void) {
  char tmp[4096];
#if defined(_WIN32)
  if (!_getcwd(tmp, sizeof(tmp))) return cmotive_strdup_local("");
#else
  if (!getcwd(tmp, sizeof(tmp))) return cmotive_strdup_local("");
#endif
  return cmotive_strdup_local(tmp);
}

int __cmotive_log_level = 0;
int cmotive_sys_logging_set_level(int level) { __cmotive_log_level = level; return 0; }
int cmotive_sys_logging_emit(int level, const char *name, const char *m) { if (level < __cmotive_log_level) return 0; return fprintf((level >= 4) ? stderr : stdout, "[%s] %s\n", name, m ? m : ""); }
int cmotive_sys_logging_trace(const char* m) { return cmotive_sys_logging_emit(0, "TRACE", m); }
int cmotive_sys_logging_debug(const char* m) { return cmotive_sys_logging_emit(1, "DEBUG", m); }
int cmotive_sys_logging_info(const char* m) { return cmotive_sys_logging_emit(2, "INFO", m); }
int cmotive_sys_logging_warn(const char* m) { return cmotive_sys_logging_emit(3, "WARN", m); }
int cmotive_sys_logging_error(const char* m) { return cmotive_sys_logging_emit(4, "ERROR", m); }
int cmotive_sys_logging_fatal(const char* m) { return cmotive_sys_logging_emit(5, "FATAL", m); }

#define CMOTIVE_MATH1(name) double cmotive_sys_math_##name(double x) { return name(x); }
#define CMOTIVE_MATH2(name) double cmotive_sys_math_##name(double x, double y) { return name(x, y); }
CMOTIVE_MATH1(sin) CMOTIVE_MATH1(cos) CMOTIVE_MATH1(tan) CMOTIVE_MATH1(asin) CMOTIVE_MATH1(acos) CMOTIVE_MATH1(atan)
CMOTIVE_MATH2(atan2) CMOTIVE_MATH1(sinh) CMOTIVE_MATH1(cosh) CMOTIVE_MATH1(tanh) CMOTIVE_MATH1(exp) CMOTIVE_MATH1(exp2)
CMOTIVE_MATH1(expm1) CMOTIVE_MATH1(log) CMOTIVE_MATH1(log10) CMOTIVE_MATH1(log2) CMOTIVE_MATH1(log1p) CMOTIVE_MATH2(pow)
CMOTIVE_MATH1(sqrt) CMOTIVE_MATH1(cbrt) CMOTIVE_MATH2(hypot) CMOTIVE_MATH1(ceil) CMOTIVE_MATH1(floor) CMOTIVE_MATH1(trunc)
CMOTIVE_MATH1(round) CMOTIVE_MATH1(nearbyint) CMOTIVE_MATH1(rint) CMOTIVE_MATH1(fabs) CMOTIVE_MATH2(fmod) CMOTIVE_MATH2(remainder)
CMOTIVE_MATH2(copysign) CMOTIVE_MATH2(fmax) CMOTIVE_MATH2(fmin) CMOTIVE_MATH2(fdim) CMOTIVE_MATH1(erf) CMOTIVE_MATH1(erfc)
CMOTIVE_MATH1(tgamma) CMOTIVE_MATH1(lgamma)
int cmotive_sys_math_isfinite(double x) { return isfinite(x); }
int cmotive_sys_math_isnan(double x) { return isnan(x); }
int cmotive_sys_math_isinf(double x) { return isinf(x); }
int cmotive_sys_math_isnormal(double x) { return isnormal(x); }
int cmotive_sys_math_signbit(double x) { return signbit(x); }
int cmotive_sys_math_fpclassify(double x) { return fpclassify(x); }
int64_t cmotive_sys_math_abs_i64(int64_t x) { return x < 0 ? -x : x; }
uint64_t cmotive_sys_math_min_u64(uint64_t x, uint64_t y) { return x < y ? x : y; }
uint64_t cmotive_sys_math_max_u64(uint64_t x, uint64_t y) { return x > y ? x : y; }
double cmotive_sys_math_pi(void) { return 3.14159265358979323846264338327950288; }
double cmotive_sys_math_e(void) { return 2.71828182845904523536028747135266249; }

uint64_t cmotive_sys_string_strlen(const char *s) { return s ? (uint64_t)strlen(s) : 0u; }
char* cmotive_sys_string_strcpy(char *d, const char *s) { return strcpy(d, s ? s : ""); }
char* cmotive_sys_string_strncpy(char *d, const char *s, uint64_t n) { return strncpy(d, s ? s : "", (size_t)n); }
char* cmotive_sys_string_strcat(char *d, const char *s) { return strcat(d, s ? s : ""); }
char* cmotive_sys_string_strncat(char *d, const char *s, uint64_t n) { return strncat(d, s ? s : "", (size_t)n); }
int cmotive_sys_string_strcmp(const char *a, const char *b) { return strcmp(a ? a : "", b ? b : ""); }
int cmotive_sys_string_strncmp(const char *a, const char *b, uint64_t n) { return strncmp(a ? a : "", b ? b : "", (size_t)n); }
char* cmotive_sys_string_strchr(char *s, int c) { return s ? strchr(s, c) : NULL; }
char* cmotive_sys_string_strrchr(char *s, int c) { return s ? strrchr(s, c) : NULL; }
char* cmotive_sys_string_strstr(char *h, const char *n) { return h ? strstr(h, n ? n : "") : NULL; }
char* cmotive_sys_string_strdup(const char *s) { return cmotive_strdup_local(s); }
void* cmotive_sys_string_memset(void *d, int v, uint64_t n) { return memset(d, v, (size_t)n); }
void* cmotive_sys_string_memcpy(void *d, const void *s, uint64_t n) { return memcpy(d, s, (size_t)n); }
void* cmotive_sys_string_memmove(void *d, const void *s, uint64_t n) { return memmove(d, s, (size_t)n); }
int cmotive_sys_string_memcmp(const void *a, const void *b, uint64_t n) { return memcmp(a, b, (size_t)n); }
int cmotive_sys_string_toupper(int c) { return toupper((unsigned char)c); }
int cmotive_sys_string_tolower(int c) { return tolower((unsigned char)c); }
int cmotive_sys_string_isalpha(int c) { return isalpha((unsigned char)c); }
int cmotive_sys_string_isdigit(int c) { return isdigit((unsigned char)c); }
int cmotive_sys_string_isspace(int c) { return isspace((unsigned char)c); }
int cmotive_sys_string_atoi(const char *s) { return atoi(s ? s : "0"); }
int64_t cmotive_sys_string_atoll(const char *s) { return (int64_t)atoll(s ? s : "0"); }
int64_t cmotive_sys_string_strtoll(const char *s, int base) { return (int64_t)strtoll(s ? s : "0", NULL, base); }
double cmotive_sys_string_strtod(const char *s) { return strtod(s ? s : "0", NULL); }
void cmotive_sys_string_free(void *p) { free(p); }
char* cmotive_sys_string_trim(const char *s) { const char *a; const char *b; char *r; size_t n; if (!s) return cmotive_strdup_local(""); a = s; while (*a && isspace((unsigned char)*a)) a++; b = a + strlen(a); while (b > a && isspace((unsigned char)b[-1])) b--; n = (size_t)(b - a); r = (char*)malloc(n + 1u); if (!r) return NULL; memcpy(r, a, n); r[n] = 0; return r; }
char* cmotive_sys_string_to_upper(const char *s) { char *r = cmotive_strdup_local(s); char *p = r; if (!r) return NULL; while (*p) { *p = (char)toupper((unsigned char)*p); p++; } return r; }
char* cmotive_sys_string_to_lower(const char *s) { char *r = cmotive_strdup_local(s); char *p = r; if (!r) return NULL; while (*p) { *p = (char)tolower((unsigned char)*p); p++; } return r; }

typedef struct CMotive_StrParseRow { char **cells; uint64_t count; uint64_t cap; } CMotive_StrParseRow;
typedef struct CMotive_StrParseTable { CMotive_StrParseRow *rows; uint64_t count; uint64_t cap; } CMotive_StrParseTable;
int cmotive_char_in_set(char c, const char *set) { if (!set) return 0; while (*set) { if (*set++ == c) return 1; } return 0; }
void cmotive_parse_add_cell(CMotive_StrParseTable *t, const char *start, size_t n) { CMotive_StrParseRow *r; char *cell; if (t->count == 0) { t->cap = 4; t->rows = (CMotive_StrParseRow*)calloc((size_t)t->cap, sizeof(CMotive_StrParseRow)); t->count = 1; } r = &t->rows[t->count - 1u]; if (r->count == r->cap) { r->cap = r->cap ? r->cap * 2u : 4u; r->cells = (char**)realloc(r->cells, (size_t)r->cap * sizeof(char*)); } cell = (char*)malloc(n + 1u); if (!cell) return; memcpy(cell, start, n); cell[n] = 0; r->cells[r->count++] = cell; }
void cmotive_parse_new_row(CMotive_StrParseTable *t) { if (t->count == t->cap) { t->cap = t->cap ? t->cap * 2u : 4u; t->rows = (CMotive_StrParseRow*)realloc(t->rows, (size_t)t->cap * sizeof(CMotive_StrParseRow)); } memset(&t->rows[t->count], 0, sizeof(CMotive_StrParseRow)); t->count++; }
void* cmotive_sys_string_str_parse(const char *input, const char *record_delims, const char *field_delims, char escape) { CMotive_StrParseTable *t = (CMotive_StrParseTable*)calloc(1, sizeof(CMotive_StrParseTable)); const char *p; char *buf = NULL; size_t blen = 0, bcap = 0; if (!t) return NULL; cmotive_parse_new_row(t); if (!input) return t; for (p = input; ; ++p) { char c = *p; int end = (c == 0); int rec = !end && cmotive_char_in_set(c, record_delims); int fld = !end && cmotive_char_in_set(c, field_delims); if (!end && escape && c == escape && p[1]) { c = *++p; } else if (end || rec || fld) { cmotive_parse_add_cell(t, buf ? buf : "", blen); blen = 0; if (rec && !end) cmotive_parse_new_row(t); if (end) break; continue; } if (blen + 1u >= bcap) { bcap = bcap ? bcap * 2u : 32u; buf = (char*)realloc(buf, bcap); } if (buf) buf[blen++] = c; } free(buf); return t; }
uint64_t cmotive_sys_string_str_parse_rows(void *table) { CMotive_StrParseTable *t = (CMotive_StrParseTable*)table; return t ? t->count : 0u; }
uint64_t cmotive_sys_string_str_parse_cols(void *table, uint64_t row) { CMotive_StrParseTable *t = (CMotive_StrParseTable*)table; return (t && row < t->count) ? t->rows[row].count : 0u; }
char* cmotive_sys_string_str_parse_at(void *table, uint64_t row, uint64_t col) { CMotive_StrParseTable *t = (CMotive_StrParseTable*)table; if (!t || row >= t->count || col >= t->rows[row].count) return NULL; return t->rows[row].cells[col]; }
void cmotive_sys_string_str_parse_free(void *table) { CMotive_StrParseTable *t = (CMotive_StrParseTable*)table; uint64_t r, c; if (!t) return; for (r = 0; r < t->count; ++r) { for (c = 0; c < t->rows[r].count; ++c) free(t->rows[r].cells[c]); free(t->rows[r].cells); } free(t->rows); free(t); }

uint64_t cmotive_sys_wide16_len(const uint16_t *s) { uint64_t n = 0; if (!s) return 0; while (s[n]) n++; return n; }
int cmotive_sys_wide16_cmp(const uint16_t *a, const uint16_t *b) { const uint16_t z[1] = {0}; if (!a) a = z; if (!b) b = z; while (*a && *a == *b) { a++; b++; } return (int)*a - (int)*b; }
uint16_t* cmotive_sys_wide16_cpy(uint16_t *d, const uint16_t *s) { uint16_t *r = d; if (!d) return NULL; if (!s) { *d = 0; return d; } while ((*d++ = *s++)) {} return r; }
uint16_t* cmotive_sys_wide16_ncpy(uint16_t *d, const uint16_t *s, uint64_t n) { uint64_t i; if (!d) return NULL; for (i=0; i<n; ++i) d[i] = (s && s[i]) ? s[i] : 0; return d; }
uint16_t* cmotive_sys_wide16_chr(uint16_t *s, uint16_t c) { if (!s) return NULL; while (*s) { if (*s == c) return s; s++; } return c == 0 ? s : NULL; }
uint64_t cmotive_sys_wide32_len(const uint32_t *s) { uint64_t n = 0; if (!s) return 0; while (s[n]) n++; return n; }
int cmotive_sys_wide32_cmp(const uint32_t *a, const uint32_t *b) { const uint32_t z[1] = {0}; if (!a) a = z; if (!b) b = z; while (*a && *a == *b) { a++; b++; } return (*a > *b) - (*a < *b); }
uint32_t* cmotive_sys_wide32_cpy(uint32_t *d, const uint32_t *s) { uint32_t *r = d; if (!d) return NULL; if (!s) { *d = 0; return d; } while ((*d++ = *s++)) {} return r; }
uint32_t* cmotive_sys_wide32_ncpy(uint32_t *d, const uint32_t *s, uint64_t n) { uint64_t i; if (!d) return NULL; for (i=0; i<n; ++i) d[i] = (s && s[i]) ? s[i] : 0; return d; }
uint32_t* cmotive_sys_wide32_chr(uint32_t *s, uint32_t c) { if (!s) return NULL; while (*s) { if (*s == c) return s; s++; } return c == 0 ? s : NULL; }

#if defined(_WIN32)
typedef CRITICAL_SECTION CMotive_Mutex;
void* cmotive_sys_locks_mutex_create(void) { CMotive_Mutex *m = (CMotive_Mutex*)malloc(sizeof(CMotive_Mutex)); if (m) InitializeCriticalSection(m); return m; }
void* cmotive_sys_locks_recursive_mutex_create(void) { return cmotive_sys_locks_mutex_create(); }
int cmotive_sys_locks_mutex_lock(void *h) { if (!h) return -1; EnterCriticalSection((CMotive_Mutex*)h); return 0; }
int cmotive_sys_locks_mutex_trylock(void *h) { if (!h) return -1; return TryEnterCriticalSection((CMotive_Mutex*)h) ? 0 : 1; }
int cmotive_sys_locks_mutex_unlock(void *h) { if (!h) return -1; LeaveCriticalSection((CMotive_Mutex*)h); return 0; }
void cmotive_sys_locks_mutex_destroy(void *h) { if (h) { DeleteCriticalSection((CMotive_Mutex*)h); free(h); } }
#else
typedef pthread_mutex_t CMotive_Mutex;
void* cmotive_sys_locks_mutex_create(void) { CMotive_Mutex *m = (CMotive_Mutex*)malloc(sizeof(CMotive_Mutex)); if (m) pthread_mutex_init(m, NULL); return m; }
void* cmotive_sys_locks_recursive_mutex_create(void) { CMotive_Mutex *m = (CMotive_Mutex*)malloc(sizeof(CMotive_Mutex)); if (m) { pthread_mutexattr_t a; pthread_mutexattr_init(&a); pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE); pthread_mutex_init(m, &a); pthread_mutexattr_destroy(&a); } return m; }
int cmotive_sys_locks_mutex_lock(void *h) { return h ? pthread_mutex_lock((CMotive_Mutex*)h) : -1; }
int cmotive_sys_locks_mutex_trylock(void *h) { return h ? pthread_mutex_trylock((CMotive_Mutex*)h) : -1; }
int cmotive_sys_locks_mutex_unlock(void *h) { return h ? pthread_mutex_unlock((CMotive_Mutex*)h) : -1; }
void cmotive_sys_locks_mutex_destroy(void *h) { if (h) { pthread_mutex_destroy((CMotive_Mutex*)h); free(h); } }
#endif
void* cmotive_sys_locks_spin_create(void) { return cmotive_sys_locks_mutex_create(); }
int cmotive_sys_locks_spin_lock(void *h) { return cmotive_sys_locks_mutex_lock(h); }
int cmotive_sys_locks_spin_trylock(void *h) { return cmotive_sys_locks_mutex_trylock(h); }
int cmotive_sys_locks_spin_unlock(void *h) { return cmotive_sys_locks_mutex_unlock(h); }
void cmotive_sys_locks_spin_destroy(void *h) { cmotive_sys_locks_mutex_destroy(h); }
#if defined(_WIN32)
void* cmotive_sys_locks_rwlock_create(void) { return cmotive_sys_locks_mutex_create(); }
int cmotive_sys_locks_rwlock_rdlock(void *h) { return cmotive_sys_locks_mutex_lock(h); }
int cmotive_sys_locks_rwlock_wrlock(void *h) { return cmotive_sys_locks_mutex_lock(h); }
int cmotive_sys_locks_rwlock_unlock(void *h) { return cmotive_sys_locks_mutex_unlock(h); }
void cmotive_sys_locks_rwlock_destroy(void *h) { cmotive_sys_locks_mutex_destroy(h); }
#else
void* cmotive_sys_locks_rwlock_create(void) { pthread_rwlock_t *r = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t)); if (r) pthread_rwlock_init(r, NULL); return r; }
int cmotive_sys_locks_rwlock_rdlock(void *h) { return h ? pthread_rwlock_rdlock((pthread_rwlock_t*)h) : -1; }
int cmotive_sys_locks_rwlock_wrlock(void *h) { return h ? pthread_rwlock_wrlock((pthread_rwlock_t*)h) : -1; }
int cmotive_sys_locks_rwlock_unlock(void *h) { return h ? pthread_rwlock_unlock((pthread_rwlock_t*)h) : -1; }
void cmotive_sys_locks_rwlock_destroy(void *h) { if (h) { pthread_rwlock_destroy((pthread_rwlock_t*)h); free(h); } }
#endif

void* cmotive_sys_thread_start_legacy(void *entry, void *userdata) { (void)entry; (void)userdata; return NULL; }
int cmotive_sys_thread_join_legacy(void *h) { (void)h; return 0; }
int cmotive_sys_thread_detach_legacy(void *h) { (void)h; return 0; }
void* cmotive_sys_thread_current_legacy(void) { return NULL; }
void cmotive_sys_thread_sleep_ms(uint32_t ms) {
#if defined(_WIN32)
  Sleep(ms);
#else
  usleep((useconds_t)ms * 1000u);
#endif
}

void cmotive_sys_thread_sleep_us(uint64_t us) {
#if defined(_WIN32)
  if (us == 0) { Sleep(0); return; }
  Sleep((DWORD)((us + 999u) / 1000u));
#else
  usleep((useconds_t)us);
#endif
}
void cmotive_sys_thread_sleep_ns(uint64_t ns) {
#if defined(_WIN32)
  if (ns == 0) { Sleep(0); return; }
  Sleep((DWORD)((ns + 999999u) / 1000000u));
#else
  struct timespec ts;
  ts.tv_sec = (time_t)(ns / 1000000000ull);
  ts.tv_nsec = (long)(ns % 1000000000ull);
  nanosleep(&ts, NULL);
#endif
}

int cmotive_sys_thread_yield_legacy(void) { return 0; }

int cmotive_sys_net_socket_tcp_legacy(void) { return -1; }
int cmotive_sys_net_socket_udp_legacy(void) { return -1; }
int cmotive_sys_net_socket_raw_legacy(void) { return -1; }
int cmotive_sys_net_socket_close_legacy(int fd) { (void)fd; return 0; }

int __cmotive_exception_last_code = 0;
void cmotive_sys_exception_set_code(int code) { __cmotive_exception_last_code = code; }
int cmotive_sys_exception_last_code(void) { return __cmotive_exception_last_code; }
/* additional math/string/locks helpers */
double cmotive_sys_math_fma(double x, double y, double z) { return fma(x, y, z); }
double cmotive_sys_math_ldexp(double x, int e) { return ldexp(x, e); }
double cmotive_sys_math_scalbn(double x, int e) { return scalbn(x, e); }
int cmotive_sys_math_ilogb(double x) { return ilogb(x); }
double cmotive_sys_math_logb(double x) { return logb(x); }
double cmotive_sys_math_nextafter(double x, double y) { return nextafter(x, y); }
double cmotive_sys_math_modf(double x, double *iptr) { return modf(x, iptr); }
double cmotive_sys_math_frexp_value(double x, int *eptr) { return frexp(x, eptr); }
float cmotive_sys_math_sinf(float x) { return sinf(x); }
float cmotive_sys_math_cosf(float x) { return cosf(x); }
float cmotive_sys_math_tanf(float x) { return tanf(x); }
float cmotive_sys_math_sqrtf(float x) { return sqrtf(x); }
float cmotive_sys_math_powf(float x, float y) { return powf(x, y); }
long double cmotive_sys_math_sinl(long double x) { return sinl(x); }
long double cmotive_sys_math_cosl(long double x) { return cosl(x); }
long double cmotive_sys_math_tanl(long double x) { return tanl(x); }
long double cmotive_sys_math_sqrtl(long double x) { return sqrtl(x); }
long double cmotive_sys_math_powl(long double x, long double y) { return powl(x, y); }
uint64_t cmotive_sys_string_strspn(const char *s, const char *accept) { return s ? (uint64_t)strspn(s, accept ? accept : "") : 0u; }
uint64_t cmotive_sys_string_strcspn(const char *s, const char *reject) { return s ? (uint64_t)strcspn(s, reject ? reject : "") : 0u; }
char* cmotive_sys_string_strpbrk(char *s, const char *accept) { return s ? strpbrk(s, accept ? accept : "") : NULL; }
char* cmotive_sys_string_strerror(int errnum) { return strerror(errnum); }
int cmotive_ascii_casecmp(const char *a, const char *b, uint64_t n, int limited) { uint64_t i = 0; if (!a) a = ""; if (!b) b = ""; while ((!limited || i < n) && (a[i] || b[i])) { int ca = tolower((unsigned char)a[i]); int cb = tolower((unsigned char)b[i]); if (ca != cb) return ca - cb; i++; } return 0; }
int cmotive_sys_string_strcasecmp(const char *a, const char *b) { return cmotive_ascii_casecmp(a, b, 0, 0); }
int cmotive_sys_string_strncasecmp(const char *a, const char *b, uint64_t n) { return cmotive_ascii_casecmp(a, b, n, 1); }
uint16_t* cmotive_sys_wide16_cat(uint16_t *d, const uint16_t *s) { uint16_t *r = d; if (!d) return NULL; while (*d) d++; if (!s) { *d = 0; return r; } while ((*d++ = *s++)) {} return r; }
int cmotive_sys_wide16_ncmp(const uint16_t *a, const uint16_t *b, uint64_t n) { uint64_t i; const uint16_t z[1] = {0}; if (!a) a = z; if (!b) b = z; for (i=0; i<n; ++i) { if (a[i] != b[i] || !a[i] || !b[i]) return (int)a[i] - (int)b[i]; } return 0; }
uint16_t* cmotive_sys_wide16_rchr(uint16_t *s, uint16_t c) { uint16_t *last = NULL; if (!s) return NULL; do { if (*s == c) last = s; } while (*s++); return last; }
uint32_t* cmotive_sys_wide32_cat(uint32_t *d, const uint32_t *s) { uint32_t *r = d; if (!d) return NULL; while (*d) d++; if (!s) { *d = 0; return r; } while ((*d++ = *s++)) {} return r; }
int cmotive_sys_wide32_ncmp(const uint32_t *a, const uint32_t *b, uint64_t n) { uint64_t i; const uint32_t z[1] = {0}; if (!a) a = z; if (!b) b = z; for (i=0; i<n; ++i) { if (a[i] != b[i] || !a[i] || !b[i]) return (a[i] > b[i]) - (a[i] < b[i]); } return 0; }
uint32_t* cmotive_sys_wide32_rchr(uint32_t *s, uint32_t c) { uint32_t *last = NULL; if (!s) return NULL; do { if (*s == c) last = s; } while (*s++); return last; }
#if defined(_WIN32)
void* cmotive_sys_locks_cond_create(void) { return malloc(1); }
int cmotive_sys_locks_cond_wait(void *c, void *m) { (void)c; (void)m; return 0; }
int cmotive_sys_locks_cond_signal(void *c) { (void)c; return 0; }
int cmotive_sys_locks_cond_broadcast(void *c) { (void)c; return 0; }
void cmotive_sys_locks_cond_destroy(void *c) { free(c); }
#else
void* cmotive_sys_locks_cond_create(void) { pthread_cond_t *c = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)); if (c) pthread_cond_init(c, NULL); return c; }
int cmotive_sys_locks_cond_wait(void *c, void *m) { return (c && m) ? pthread_cond_wait((pthread_cond_t*)c, (pthread_mutex_t*)m) : -1; }
int cmotive_sys_locks_cond_signal(void *c) { return c ? pthread_cond_signal((pthread_cond_t*)c) : -1; }
int cmotive_sys_locks_cond_broadcast(void *c) { return c ? pthread_cond_broadcast((pthread_cond_t*)c) : -1; }
void cmotive_sys_locks_cond_destroy(void *c) { if (c) { pthread_cond_destroy((pthread_cond_t*)c); free(c); } }
#endif
typedef struct CMotive_Semaphore { CMotive_Mutex mutex; int count; } CMotive_Semaphore;
void* cmotive_sys_locks_semaphore_create(int initial) { CMotive_Semaphore *s = (CMotive_Semaphore*)malloc(sizeof(CMotive_Semaphore)); if (!s) return NULL; s->count = initial; return s; }
int cmotive_sys_locks_semaphore_post(void *h) { CMotive_Semaphore *s = (CMotive_Semaphore*)h; if (!s) return -1; s->count++; return 0; }
int cmotive_sys_locks_semaphore_wait(void *h) { CMotive_Semaphore *s = (CMotive_Semaphore*)h; if (!s) return -1; if (s->count > 0) s->count--; return 0; }
int cmotive_sys_locks_semaphore_trywait(void *h) { CMotive_Semaphore *s = (CMotive_Semaphore*)h; if (!s) return -1; if (s->count <= 0) return 1; s->count--; return 0; }
void cmotive_sys_locks_semaphore_destroy(void *h) { free(h); }

/* ---- end CMotive standard library runtime helpers ---- */



/* ---- CMotive expanded Sys::IO/STL/Algorithms/Net/native-thread runtime helpers ---- */
#include <stdarg.h>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
int cmotive_sys_io_print(const char *s) { return fputs(s ? s : "", stdout); }
int cmotive_sys_io_println(const char *s) { int r = fputs(s ? s : "", stdout); fputc('\n', stdout); return r; }
int cmotive_sys_io_printf(const char *fmt, ...) { va_list ap; int r; va_start(ap, fmt); r = vprintf(fmt ? fmt : "", ap); va_end(ap); return r; }
int cmotive_sys_io_sprintf(char *dst, const char *fmt, ...) { va_list ap; int r; if (!dst) return -1; va_start(ap, fmt); r = vsprintf(dst, fmt ? fmt : "", ap); va_end(ap); return r; }
int cmotive_sys_io_scanf(const char *fmt, ...) { va_list ap; int r; va_start(ap, fmt); r = vscanf(fmt ? fmt : "", ap); va_end(ap); return r; }

/* Backward compatibility: Sys::Stdio is a wrapper over Sys::IO. */
int cmotive_sys_stdio_print_new(const char *s) { return cmotive_sys_io_print(s); }
int cmotive_sys_stdio_println_new(const char *s) { return cmotive_sys_io_println(s); }

/* Generic integer vector used as the initial concrete storage for Vector/List/Dlist. */
typedef struct CMotive_VectorEx { int64_t *items; uint64_t count; uint64_t cap; } CMotive_VectorEx;
void* cmotive_sys_stl_vector_create(void) { return calloc(1, sizeof(CMotive_VectorEx)); }
int cmotive_vector_ex_reserve(CMotive_VectorEx *v, uint64_t cap) { int64_t *n; if (!v) return -1; if (cap <= v->cap) return 0; n=(int64_t*)realloc(v->items, (size_t)cap * sizeof(int64_t)); if (!n) return -1; v->items=n; v->cap=cap; return 0; }
int cmotive_sys_stl_vector_push_i64(void *h, int64_t value) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (!v) return -1; if (v->count == v->cap && cmotive_vector_ex_reserve(v, v->cap ? v->cap * 2u : 8u)) return -1; v->items[v->count++] = value; return 0; }
int64_t cmotive_sys_stl_vector_get_i64(void *h, uint64_t index) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (!v || index >= v->count) return 0; return v->items[index]; }
int cmotive_sys_stl_vector_set_i64(void *h, uint64_t index, int64_t value) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (!v || index >= v->count) return -1; v->items[index] = value; return 0; }
uint64_t cmotive_sys_stl_vector_size(void *h) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; return v ? v->count : 0u; }
int64_t cmotive_sys_stl_vector_pop_i64(void *h) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (!v || v->count == 0) return 0; return v->items[--v->count]; }
void cmotive_sys_stl_vector_clear(void *h) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (v) v->count = 0; }
void cmotive_sys_stl_vector_destroy(void *h) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (v) { free(v->items); free(v); } }
int cmotive_i64_cmp_ex(const void *a, const void *b) { int64_t x=*(const int64_t*)a, y=*(const int64_t*)b; return (x>y)-(x<y); }
int cmotive_i32_cmp_ex(const void *a, const void *b) { int32_t x=*(const int32_t*)a, y=*(const int32_t*)b; return (x>y)-(x<y); }
void cmotive_sys_stl_vector_sort_i64(void *h) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; if (v && v->items) qsort(v->items, (size_t)v->count, sizeof(int64_t), cmotive_i64_cmp_ex); }
int64_t cmotive_sys_stl_vector_binary_search_i64(void *h, int64_t needle) { CMotive_VectorEx *v=(CMotive_VectorEx*)h; uint64_t lo=0, hi=v ? v->count : 0; while (lo < hi) { uint64_t mid = lo + (hi-lo)/2u; int64_t val=v->items[mid]; if (val == needle) return (int64_t)mid; if (val < needle) lo = mid + 1u; else hi = mid; } return -1; }
#define cmotive_sys_stl_list_create cmotive_sys_stl_vector_create
#define cmotive_sys_stl_list_push_back_i64 cmotive_sys_stl_vector_push_i64
#define cmotive_sys_stl_list_size cmotive_sys_stl_vector_size
#define cmotive_sys_stl_list_destroy cmotive_sys_stl_vector_destroy
#define cmotive_sys_stl_dlist_create cmotive_sys_stl_vector_create
#define cmotive_sys_stl_dlist_push_back_i64 cmotive_sys_stl_vector_push_i64
#define cmotive_sys_stl_dlist_size cmotive_sys_stl_vector_size
#define cmotive_sys_stl_dlist_destroy cmotive_sys_stl_vector_destroy

typedef struct CMotive_MapEntryEx { char *key; int64_t value; } CMotive_MapEntryEx;
typedef struct CMotive_MapEx { CMotive_MapEntryEx *entries; uint64_t count; uint64_t cap; int multi; } CMotive_MapEx;
void* cmotive_map_ex_create(int multi) { CMotive_MapEx *m=(CMotive_MapEx*)calloc(1,sizeof(CMotive_MapEx)); if (m) m->multi = multi; return m; }
void* cmotive_sys_stl_map_create(void) { return cmotive_map_ex_create(0); }
void* cmotive_sys_stl_dict_create(void) { return cmotive_map_ex_create(0); }
void* cmotive_sys_stl_hash_dict_create(void) { return cmotive_map_ex_create(0); }
void* cmotive_sys_stl_multi_dict_create(void) { return cmotive_map_ex_create(1); }
void* cmotive_sys_stl_multi_hash_dict_create(void) { return cmotive_map_ex_create(1); }
int cmotive_map_ex_reserve(CMotive_MapEx *m, uint64_t cap) { CMotive_MapEntryEx *n; if (!m) return -1; if (cap <= m->cap) return 0; n=(CMotive_MapEntryEx*)realloc(m->entries, (size_t)cap * sizeof(CMotive_MapEntryEx)); if (!n) return -1; m->entries=n; m->cap=cap; return 0; }
int cmotive_sys_stl_map_put_i64(void *h, const char *key, int64_t value) { CMotive_MapEx *m=(CMotive_MapEx*)h; uint64_t i; if (!m || !key) return -1; if (!m->multi) for (i=0;i<m->count;i++) if (strcmp(m->entries[i].key, key)==0) { m->entries[i].value=value; return 0; } if (m->count == m->cap && cmotive_map_ex_reserve(m, m->cap ? m->cap * 2u : 8u)) return -1; m->entries[m->count].key=cmotive_strdup_local(key); m->entries[m->count].value=value; m->count++; return 0; }
int64_t cmotive_sys_stl_map_get_i64(void *h, const char *key, int64_t fallback) { CMotive_MapEx *m=(CMotive_MapEx*)h; uint64_t i; if (!m || !key) return fallback; for (i=0;i<m->count;i++) if (strcmp(m->entries[i].key, key)==0) return m->entries[i].value; return fallback; }
int cmotive_sys_stl_map_contains(void *h, const char *key) { CMotive_MapEx *m=(CMotive_MapEx*)h; uint64_t i; if (!m || !key) return 0; for (i=0;i<m->count;i++) if (strcmp(m->entries[i].key, key)==0) return 1; return 0; }
uint64_t cmotive_sys_stl_map_size(void *h) { CMotive_MapEx *m=(CMotive_MapEx*)h; return m ? m->count : 0u; }
void cmotive_sys_stl_map_destroy(void *h) { CMotive_MapEx *m=(CMotive_MapEx*)h; uint64_t i; if (m) { for (i=0;i<m->count;i++) free(m->entries[i].key); free(m->entries); free(m); } }
#define cmotive_sys_stl_dict_put_i64 cmotive_sys_stl_map_put_i64
#define cmotive_sys_stl_dict_get_i64 cmotive_sys_stl_map_get_i64
#define cmotive_sys_stl_dict_destroy cmotive_sys_stl_map_destroy
#define cmotive_sys_stl_hash_dict_put_i64 cmotive_sys_stl_map_put_i64
#define cmotive_sys_stl_hash_dict_get_i64 cmotive_sys_stl_map_get_i64
#define cmotive_sys_stl_hash_dict_destroy cmotive_sys_stl_map_destroy
#define cmotive_sys_stl_multi_dict_put_i64 cmotive_sys_stl_map_put_i64
#define cmotive_sys_stl_multi_dict_get_i64 cmotive_sys_stl_map_get_i64
#define cmotive_sys_stl_multi_dict_destroy cmotive_sys_stl_map_destroy
#define cmotive_sys_stl_multi_hash_dict_put_i64 cmotive_sys_stl_map_put_i64
#define cmotive_sys_stl_multi_hash_dict_get_i64 cmotive_sys_stl_map_get_i64
#define cmotive_sys_stl_multi_hash_dict_destroy cmotive_sys_stl_map_destroy

typedef struct CMotive_TreeNodeEx { int64_t value; struct CMotive_TreeNodeEx *left; struct CMotive_TreeNodeEx *right; } CMotive_TreeNodeEx;
typedef struct CMotive_TreeEx { CMotive_TreeNodeEx *root; uint64_t count; } CMotive_TreeEx;
void* cmotive_sys_stl_binary_search_tree_create(void) { return calloc(1, sizeof(CMotive_TreeEx)); }
CMotive_TreeNodeEx* cmotive_tree_ex_insert(CMotive_TreeNodeEx *n, int64_t v, int *added) { if (!n) { CMotive_TreeNodeEx *z=(CMotive_TreeNodeEx*)calloc(1,sizeof(CMotive_TreeNodeEx)); if (z) { z->value=v; *added=1; } return z; } if (v < n->value) n->left=cmotive_tree_ex_insert(n->left,v,added); else if (v > n->value) n->right=cmotive_tree_ex_insert(n->right,v,added); return n; }
int cmotive_sys_stl_binary_search_tree_insert_i64(void *h, int64_t v) { CMotive_TreeEx *t=(CMotive_TreeEx*)h; int added=0; if (!t) return -1; t->root=cmotive_tree_ex_insert(t->root,v,&added); if (added) t->count++; return added ? 0 : 1; }
int cmotive_tree_ex_contains(CMotive_TreeNodeEx *n, int64_t v) { while (n) { if (v == n->value) return 1; n = (v < n->value) ? n->left : n->right; } return 0; }
int cmotive_sys_stl_binary_search_tree_contains_i64(void *h, int64_t v) { CMotive_TreeEx *t=(CMotive_TreeEx*)h; return t ? cmotive_tree_ex_contains(t->root, v) : 0; }
void cmotive_tree_ex_free(CMotive_TreeNodeEx *n) { if (n) { cmotive_tree_ex_free(n->left); cmotive_tree_ex_free(n->right); free(n); } }
void cmotive_sys_stl_binary_search_tree_destroy(void *h) { CMotive_TreeEx *t=(CMotive_TreeEx*)h; if (t) { cmotive_tree_ex_free(t->root); free(t); } }
#define cmotive_sys_stl_binary_tree_create cmotive_sys_stl_binary_search_tree_create
#define cmotive_sys_stl_binary_tree_insert_i64 cmotive_sys_stl_binary_search_tree_insert_i64
#define cmotive_sys_stl_binary_tree_contains_i64 cmotive_sys_stl_binary_search_tree_contains_i64
#define cmotive_sys_stl_binary_tree_destroy cmotive_sys_stl_binary_search_tree_destroy
#define cmotive_sys_stl_b_tree_create cmotive_sys_stl_binary_search_tree_create
#define cmotive_sys_stl_b_tree_insert_i64 cmotive_sys_stl_binary_search_tree_insert_i64
#define cmotive_sys_stl_b_tree_contains_i64 cmotive_sys_stl_binary_search_tree_contains_i64
#define cmotive_sys_stl_b_tree_destroy cmotive_sys_stl_binary_search_tree_destroy
#define cmotive_sys_stl_b_plus_tree_create cmotive_sys_stl_binary_search_tree_create
#define cmotive_sys_stl_b_plus_tree_insert_i64 cmotive_sys_stl_binary_search_tree_insert_i64
#define cmotive_sys_stl_b_plus_tree_contains_i64 cmotive_sys_stl_binary_search_tree_contains_i64
#define cmotive_sys_stl_b_plus_tree_destroy cmotive_sys_stl_binary_search_tree_destroy

void cmotive_sys_algorithms_sort_quick_i32(int32_t *a, uint64_t n) { if (a) qsort(a, (size_t)n, sizeof(int32_t), cmotive_i32_cmp_ex); }
void cmotive_sys_algorithms_sort_quick_i64(int64_t *a, uint64_t n) { if (a) qsort(a, (size_t)n, sizeof(int64_t), cmotive_i64_cmp_ex); }
void cmotive_sys_algorithms_sort_heap_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_quick_i64(a, n); }
void cmotive_sys_algorithms_sort_merge_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_quick_i64(a, n); }
void cmotive_sys_algorithms_sort_insertion_i64(int64_t *a, uint64_t n) { uint64_t i,j; if (!a) return; for (i=1;i<n;i++) { int64_t key=a[i]; j=i; while (j>0 && a[j-1]>key) { a[j]=a[j-1]; j--; } a[j]=key; } }
void cmotive_sys_algorithms_sort_selection_i64(int64_t *a, uint64_t n) { uint64_t i,j,min; if (!a) return; for (i=0;i<n;i++) { min=i; for (j=i+1;j<n;j++) if (a[j]<a[min]) min=j; if (min!=i) { int64_t t=a[i]; a[i]=a[min]; a[min]=t; } } }
int64_t cmotive_sys_algorithms_binary_search_i64(const int64_t *a, uint64_t n, int64_t needle) { uint64_t lo=0,hi=n; if (!a) return -1; while (lo<hi) { uint64_t mid=lo+(hi-lo)/2u; if (a[mid]==needle) return (int64_t)mid; if (a[mid]<needle) lo=mid+1u; else hi=mid; } return -1; }
int64_t cmotive_sys_algorithms_linear_search_i64(const int64_t *a, uint64_t n, int64_t needle) { uint64_t i; if (!a) return -1; for (i=0;i<n;i++) if (a[i]==needle) return (int64_t)i; return -1; }
int64_t cmotive_sys_algorithms_min_i64(int64_t a, int64_t b) { return a < b ? a : b; }
int64_t cmotive_sys_algorithms_max_i64(int64_t a, int64_t b) { return a > b ? a : b; }
int cmotive_sys_algorithms_compare_i32(int32_t a, int32_t b) { return (a>b)-(a<b); }
static void cmotive_sys_algorithms_sort_bubble_i64(int64_t *a, uint64_t n) { uint64_t i,j; if (!a) return; for (i=0;i<n;i++) for (j=1;j<n-i;j++) if (a[j-1]>a[j]) { int64_t t=a[j-1]; a[j-1]=a[j]; a[j]=t; } }
static void cmotive_sys_algorithms_sort_shell_i64(int64_t *a, uint64_t n) { uint64_t gap,i,j; if(!a) return; for(gap=n/2; gap>0; gap/=2) { for(i=gap;i<n;i++) { int64_t tmp=a[i]; for(j=i; j>=gap && a[j-gap]>tmp; j-=gap) a[j]=a[j-gap]; a[j]=tmp; } if(gap==1) break; } }
static void cmotive_sys_algorithms_sort_comb_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_quick_i64(a, n); }
static void cmotive_sys_algorithms_sort_gnome_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_insertion_i64(a, n); }
static void cmotive_sys_algorithms_sort_radix_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_quick_i64(a, n); }
static void cmotive_sys_algorithms_sort_counting_i64(int64_t *a, uint64_t n) { cmotive_sys_algorithms_sort_quick_i64(a, n); }
static int64_t cmotive_sys_algorithms_jump_search_i64(const int64_t *a, uint64_t n, int64_t needle) { return cmotive_sys_algorithms_binary_search_i64(a, n, needle); }
static int64_t cmotive_sys_algorithms_exponential_search_i64(const int64_t *a, uint64_t n, int64_t needle) { return cmotive_sys_algorithms_binary_search_i64(a, n, needle); }
static int64_t cmotive_sys_algorithms_interpolation_search_i64(const int64_t *a, uint64_t n, int64_t needle) { return cmotive_sys_algorithms_binary_search_i64(a, n, needle); }

#if defined(_WIN32)
void* cmotive_sys_thread_start(void *entry, void *userdata) { if (!entry) return NULL; return (void*)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)entry, userdata, 0, NULL); }
int cmotive_sys_thread_join(void *h) { if (!h) return -1; return WaitForSingleObject((HANDLE)h, INFINITE) == WAIT_OBJECT_0 ? 0 : -1; }
int cmotive_sys_thread_detach(void *h) { return h ? (CloseHandle((HANDLE)h) ? 0 : -1) : -1; }
void* cmotive_sys_thread_current(void) { return (void*)(uintptr_t)GetCurrentThreadId(); }
int cmotive_sys_thread_yield(void) { Sleep(0); return 0; }
#else
void* cmotive_sys_thread_start(void *entry, void *userdata) { pthread_t *t; if (!entry) return NULL; t=(pthread_t*)malloc(sizeof(pthread_t)); if(!t) return NULL; if(pthread_create(t, NULL, (void*(*)(void*))entry, userdata) != 0) { free(t); return NULL; } return t; }
int cmotive_sys_thread_join(void *h) { void *r=NULL; int rc; if(!h) return -1; rc=pthread_join(*(pthread_t*)h, &r); free(h); return rc; }
int cmotive_sys_thread_detach(void *h) { int rc; if(!h) return -1; rc=pthread_detach(*(pthread_t*)h); free(h); return rc; }
void* cmotive_sys_thread_current(void) { return (void*)(uintptr_t)pthread_self(); }
int cmotive_sys_thread_yield(void) { return sched_yield(); }
#endif

#if defined(_WIN32)
int cmotive_net_init(void) { int done=0; WSADATA w; if (!done) { if (WSAStartup(MAKEWORD(2,2), &w) != 0) return -1; done=1; } return 0; }
int cmotive_sys_net_socket_close(int fd) { return closesocket((SOCKET)fd); }
#else
int cmotive_net_init(void) { return 0; }
int cmotive_sys_net_socket_close(int fd) { return close(fd); }
#endif
int cmotive_sys_net_socket_family(int family, int type, int proto) { if (cmotive_net_init() != 0) return -1; return (int)socket(family, type, proto); }
int cmotive_sys_net_socket_tcp_ipv4(void) { return cmotive_sys_net_socket_family(AF_INET, SOCK_STREAM, IPPROTO_TCP); }
int cmotive_sys_net_socket_tcp_ipv6(void) { return cmotive_sys_net_socket_family(AF_INET6, SOCK_STREAM, IPPROTO_TCP); }
int cmotive_sys_net_socket_udp_ipv4(void) { return cmotive_sys_net_socket_family(AF_INET, SOCK_DGRAM, IPPROTO_UDP); }
int cmotive_sys_net_socket_udp_ipv6(void) { return cmotive_sys_net_socket_family(AF_INET6, SOCK_DGRAM, IPPROTO_UDP); }
int cmotive_sys_net_socket_raw_ipv4(void) { return cmotive_sys_net_socket_family(AF_INET, SOCK_RAW, IPPROTO_RAW); }
int cmotive_sys_net_socket_raw_ipv6(void) { return cmotive_sys_net_socket_family(AF_INET6, SOCK_RAW, IPPROTO_RAW); }
int cmotive_sys_net_socket_icmp_ipv4(void) { return cmotive_sys_net_socket_family(AF_INET, SOCK_RAW, IPPROTO_ICMP); }
#ifdef IPPROTO_ICMPV6
int cmotive_sys_net_socket_icmp_ipv6(void) { return cmotive_sys_net_socket_family(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6); }
#else
int cmotive_sys_net_socket_icmp_ipv6(void) { return -1; }
#endif
int cmotive_sys_net_socket_tcp(void) { return cmotive_sys_net_socket_tcp_ipv4(); }
int cmotive_sys_net_socket_udp(void) { return cmotive_sys_net_socket_udp_ipv4(); }
int cmotive_sys_net_socket_raw(void) { return cmotive_sys_net_socket_raw_ipv4(); }
int cmotive_sys_net_send(int fd, const void *buf, uint64_t n) { return (int)send(fd, (const char*)buf, (size_t)n, 0); }
int cmotive_sys_net_recv(int fd, void *buf, uint64_t n) { return (int)recv(fd, (char*)buf, (size_t)n, 0); }
int cmotive_sys_net_bind_ipv4(int fd, const char *ip, uint16_t port) { struct sockaddr_in a; memset(&a,0,sizeof(a)); a.sin_family=AF_INET; a.sin_port=htons(port); a.sin_addr.s_addr=ip ? inet_addr(ip) : INADDR_ANY; return bind(fd, (struct sockaddr*)&a, sizeof(a)); }
int cmotive_sys_net_connect_ipv4(int fd, const char *ip, uint16_t port) { struct sockaddr_in a; memset(&a,0,sizeof(a)); a.sin_family=AF_INET; a.sin_port=htons(port); a.sin_addr.s_addr=inet_addr(ip ? ip : "127.0.0.1"); return connect(fd, (struct sockaddr*)&a, sizeof(a)); }
int cmotive_sys_net_listen(int fd, int backlog) { return listen(fd, backlog); }
int cmotive_sys_net_accept(int fd) { return (int)accept(fd, NULL, NULL); }

/* cmotive_sys_thread_sleep_us/ns are defined earlier in this runtime. */

/* ---- end expanded CMotive runtime helpers ---- */



/* ---- CMotive object-model Sys::STL/Sys::Algorithms/Sys::IO helpers ---- */
typedef struct CMotive_AnyVector { unsigned char *data; uint64_t count; uint64_t cap; size_t elem; } CMotive_AnyVector;
static void* cmotive_stl_vector_any_create(void) { return calloc(1, sizeof(CMotive_AnyVector)); }
static int cmotive_stl_vector_any_reserve(void *h, uint64_t cap, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; unsigned char *n; if(!v) return -1; if(!elem) elem=1; if(v->elem==0) v->elem=elem; if(cap<=v->cap) return 0; n=(unsigned char*)realloc(v->data,(size_t)cap*v->elem); if(!n) return -1; v->data=n; v->cap=cap; return 0; }
static int cmotive_stl_vector_any_push_back(void *h, const void *value, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(!v||!value) return -1; if(!elem) elem=1; if(v->count==v->cap && cmotive_stl_vector_any_reserve(h, v->cap? v->cap*2u:8u, elem)) return -1; memcpy(v->data + (size_t)v->count*v->elem, value, v->elem); v->count++; return 0; }
static int cmotive_stl_vector_any_assign(void *h, uint64_t count, const void *value, size_t elem) { uint64_t i; CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(!v) return -1; if(cmotive_stl_vector_any_reserve(h,count,elem)) return -1; v->count=0; for(i=0;i<count;i++) cmotive_stl_vector_any_push_back(h,value,elem); return 0; }
static int cmotive_stl_vector_any_get(void *h, uint64_t index, void *out, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; if(!v||!out||index>=v->count) return -1; memcpy(out, v->data + (size_t)index*v->elem, v->elem); return 0; }
static int cmotive_stl_vector_any_set(void *h, uint64_t index, const void *value, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; if(!v||!value||index>=v->count) return -1; memcpy(v->data + (size_t)index*v->elem, value, v->elem); return 0; }
static int cmotive_stl_vector_any_front(void *h, void *out, size_t elem) { (void)elem; return cmotive_stl_vector_any_get(h,0,out,elem); }
static int cmotive_stl_vector_any_back(void *h, void *out, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(!v||v->count==0) return -1; return cmotive_stl_vector_any_get(h,v->count-1,out,elem); }
static void* cmotive_stl_vector_any_data(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; return v ? v->data : NULL; }
static int cmotive_stl_vector_any_empty(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; return (!v || v->count==0); }
static uint64_t cmotive_stl_vector_any_size(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; return v ? v->count : 0u; }
static uint64_t cmotive_stl_vector_any_capacity(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; return v ? v->cap : 0u; }
static uint64_t cmotive_stl_vector_any_max_size(size_t elem) { return elem ? (uint64_t)(SIZE_MAX / elem) : 0u; }
static void cmotive_stl_vector_any_clear(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(v) v->count=0; }
static int cmotive_stl_vector_any_resize(void *h, uint64_t count, const void *value, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(!v) return -1; if(cmotive_stl_vector_any_reserve(h,count,elem)) return -1; if(count>v->count) { uint64_t i; for(i=v->count;i<count;i++) { if(value) memcpy(v->data+(size_t)i*v->elem,value,v->elem); else memset(v->data+(size_t)i*v->elem,0,v->elem); } } v->count=count; return 0; }
static int cmotive_stl_vector_any_shrink_to_fit(void *h, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; unsigned char *n; (void)elem; if(!v) return -1; if(v->count==v->cap) return 0; if(!v->count) { free(v->data); v->data=NULL; v->cap=0; return 0; } n=(unsigned char*)realloc(v->data,(size_t)v->count*v->elem); if(!n) return -1; v->data=n; v->cap=v->count; return 0; }
static int cmotive_stl_vector_any_insert(void *h, uint64_t index, const void *value, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(!v||!value) return -1; if(index>v->count) index=v->count; if(v->count==v->cap && cmotive_stl_vector_any_reserve(h,v->cap?v->cap*2u:8u,elem)) return -1; memmove(v->data+(size_t)(index+1)*v->elem, v->data+(size_t)index*v->elem, (size_t)(v->count-index)*v->elem); memcpy(v->data+(size_t)index*v->elem,value,v->elem); v->count++; return 0; }
static int cmotive_stl_vector_any_erase(void *h, uint64_t index, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; if(!v||index>=v->count) return -1; memmove(v->data+(size_t)index*v->elem, v->data+(size_t)(index+1)*v->elem, (size_t)(v->count-index-1)*v->elem); v->count--; return 0; }
static int cmotive_stl_vector_any_pop_back(void *h, void *out, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; if(!v||v->count==0) return -1; if(out) memcpy(out, v->data+(size_t)(v->count-1)*v->elem, v->elem); v->count--; return 0; }
static void cmotive_stl_vector_any_swap(void *a, void *b) { CMotive_AnyVector tmp; if(!a||!b) return; tmp=*(CMotive_AnyVector*)a; *(CMotive_AnyVector*)a=*(CMotive_AnyVector*)b; *(CMotive_AnyVector*)b=tmp; }
static void* cmotive_stl_vector_any_begin(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; return v ? v->data : NULL; }
static void* cmotive_stl_vector_any_end(void *h, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; return (v && v->data) ? (v->data + (size_t)v->count*v->elem) : NULL; }
static void* cmotive_stl_vector_any_rbegin(void *h, size_t elem) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; (void)elem; return (v && v->count) ? (v->data + (size_t)(v->count-1)*v->elem) : NULL; }
static void* cmotive_stl_vector_any_rend(void *h) { (void)h; return NULL; }
static void cmotive_stl_vector_any_destroy(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(v){ free(v->data); free(v);} }
static void cmotive_stl_vector_any_sort_i64(void *h) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; if(v&&v->data&&v->elem==sizeof(int64_t)) qsort(v->data,(size_t)v->count,sizeof(int64_t),cmotive_i64_cmp_ex); }
static int64_t cmotive_stl_vector_any_find_i64(void *h, int64_t needle) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; uint64_t i; if(!v||!v->data||v->elem!=sizeof(int64_t)) return -1; for(i=0;i<v->count;i++) if(((int64_t*)v->data)[i]==needle) return (int64_t)i; return -1; }
static int64_t cmotive_stl_vector_any_binary_search_i64(void *h, int64_t needle) { CMotive_AnyVector *v=(CMotive_AnyVector*)h; uint64_t lo=0,hi=v?v->count:0; if(!v||!v->data||v->elem!=sizeof(int64_t)) return -1; while(lo<hi){ uint64_t mid=lo+(hi-lo)/2u; int64_t val=((int64_t*)v->data)[mid]; if(val==needle) return (int64_t)mid; if(val<needle) lo=mid+1u; else hi=mid; } return -1; }

typedef struct CMotive_AnyMapEntry { char *key; unsigned char *value; } CMotive_AnyMapEntry;
typedef struct CMotive_AnyMap { CMotive_AnyMapEntry *entries; uint64_t count; uint64_t cap; int multi; size_t elem; } CMotive_AnyMap;
static void* cmotive_stl_map_any_create(int multi) { CMotive_AnyMap *m=(CMotive_AnyMap*)calloc(1,sizeof(CMotive_AnyMap)); if(m) m->multi=multi; return m; }
static int cmotive_stl_map_any_reserve(void *h, uint64_t cap, size_t elem) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; CMotive_AnyMapEntry *n; if(!m) return -1; if(!elem) elem=1; if(!m->elem) m->elem=elem; if(cap<=m->cap) return 0; n=(CMotive_AnyMapEntry*)realloc(m->entries,(size_t)cap*sizeof(CMotive_AnyMapEntry)); if(!n) return -1; memset(n+m->cap,0,(size_t)(cap-m->cap)*sizeof(CMotive_AnyMapEntry)); m->entries=n; m->cap=cap; return 0; }
static int64_t cmotive_stl_map_any_index(CMotive_AnyMap *m, const char *key) { uint64_t i; if(!m||!key) return -1; for(i=0;i<m->count;i++) if(strcmp(m->entries[i].key,key)==0) return (int64_t)i; return -1; }
static int cmotive_stl_map_any_put(void *h, const char *key, const void *value, size_t elem) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; int64_t idx; if(!m||!key||!value) return -1; if(!m->multi && (idx=cmotive_stl_map_any_index(m,key))>=0) { memcpy(m->entries[idx].value,value,m->elem); return 0; } if(m->count==m->cap && cmotive_stl_map_any_reserve(h,m->cap?m->cap*2u:8u,elem)) return -1; m->entries[m->count].key=cmotive_strdup_local(key); m->entries[m->count].value=(unsigned char*)malloc(m->elem); if(!m->entries[m->count].value) return -1; memcpy(m->entries[m->count].value,value,m->elem); m->count++; return 0; }
static int cmotive_stl_map_any_get(void *h, const char *key, void *out, size_t elem) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; int64_t idx; (void)elem; if(!m||!out||(idx=cmotive_stl_map_any_index(m,key))<0) return -1; memcpy(out,m->entries[idx].value,m->elem); return 0; }
static int cmotive_stl_map_any_get_or(void *h, const char *key, void *inout, size_t elem) { if(cmotive_stl_map_any_get(h,key,inout,elem)!=0) return 1; return 0; }
static int cmotive_stl_map_any_contains(void *h, const char *key) { return cmotive_stl_map_any_index((CMotive_AnyMap*)h,key) >= 0; }
static uint64_t cmotive_stl_map_any_count(void *h, const char *key) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; uint64_t i,c=0; if(!m||!key) return 0; for(i=0;i<m->count;i++) if(strcmp(m->entries[i].key,key)==0) c++; return c; }
static int cmotive_stl_map_any_erase(void *h, const char *key) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; int64_t idx; if(!m||(idx=cmotive_stl_map_any_index(m,key))<0) return -1; free(m->entries[idx].key); free(m->entries[idx].value); memmove(&m->entries[idx],&m->entries[idx+1],(size_t)(m->count-(uint64_t)idx-1u)*sizeof(CMotive_AnyMapEntry)); m->count--; return 0; }
static void cmotive_stl_map_any_clear(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; uint64_t i; if(!m) return; for(i=0;i<m->count;i++){ free(m->entries[i].key); free(m->entries[i].value);} m->count=0; }
static void cmotive_stl_map_any_destroy(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; if(m){ cmotive_stl_map_any_clear(h); free(m->entries); free(m);} }
static uint64_t cmotive_stl_map_any_size(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; return m?m->count:0u; }
static uint64_t cmotive_stl_map_any_capacity(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; return m?m->cap:0u; }
static int cmotive_stl_map_any_empty(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; return !m||m->count==0; }
static void cmotive_stl_map_any_swap(void *a, void *b) { CMotive_AnyMap tmp; if(!a||!b) return; tmp=*(CMotive_AnyMap*)a; *(CMotive_AnyMap*)a=*(CMotive_AnyMap*)b; *(CMotive_AnyMap*)b=tmp; }
static void* cmotive_stl_map_any_begin(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; return m?m->entries:NULL; }
static void* cmotive_stl_map_any_end(void *h) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; return (m&&m->entries)?(m->entries+m->count):NULL; }
static void* cmotive_stl_map_any_find(void *h, const char *key) { CMotive_AnyMap *m=(CMotive_AnyMap*)h; int64_t idx=cmotive_stl_map_any_index(m,key); return (m&&idx>=0)?&m->entries[idx]:NULL; }
static void* cmotive_stl_map_any_lower_bound(void *h, const char *key) { return cmotive_stl_map_any_find(h,key); }
static void* cmotive_stl_map_any_upper_bound(void *h, const char *key) { return cmotive_stl_map_any_find(h,key); }
static void* cmotive_stl_map_any_equal_range(void *h, const char *key) { return cmotive_stl_map_any_find(h,key); }

typedef struct CMotive_AnyTree { CMotive_AnyVector values; } CMotive_AnyTree;
static void* cmotive_stl_tree_any_create(void) { CMotive_AnyTree *t=(CMotive_AnyTree*)calloc(1,sizeof(CMotive_AnyTree)); if(t) t->values.elem=sizeof(int64_t); return t; }
static void cmotive_stl_tree_any_sort_unique(CMotive_AnyTree *t) { uint64_t i,w; if(!t) return; qsort(t->values.data,(size_t)t->values.count,sizeof(int64_t),cmotive_i64_cmp_ex); for(i=0,w=0;i<t->values.count;i++){ if(w==0 || ((int64_t*)t->values.data)[i]!=((int64_t*)t->values.data)[w-1]) ((int64_t*)t->values.data)[w++]=((int64_t*)t->values.data)[i]; } t->values.count=w; }
static int cmotive_stl_tree_any_insert_i64(void *h, int64_t v) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; if(!t) return -1; cmotive_stl_vector_any_push_back(&t->values,&v,sizeof(int64_t)); cmotive_stl_tree_any_sort_unique(t); return 0; }
static int cmotive_stl_tree_any_contains_i64(void *h, int64_t v) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; return t ? (cmotive_stl_vector_any_binary_search_i64(&t->values,v)>=0) : 0; }
static int cmotive_stl_tree_any_erase_i64(void *h, int64_t v) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; int64_t idx; if(!t) return -1; idx=cmotive_stl_vector_any_binary_search_i64(&t->values,v); if(idx<0) return -1; return cmotive_stl_vector_any_erase(&t->values,(uint64_t)idx,sizeof(int64_t)); }
static void cmotive_stl_tree_any_clear(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; if(t) t->values.count=0; }
static void cmotive_stl_tree_any_destroy(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; if(t){ free(t->values.data); free(t);} }
static int cmotive_stl_tree_any_empty(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; return !t||t->values.count==0; }
static uint64_t cmotive_stl_tree_any_size(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; return t?t->values.count:0u; }
static uint64_t cmotive_stl_tree_any_height(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; uint64_t n=t?t->values.count:0,hgt=0,p=1; while(p<n+1){hgt++; p<<=1;} return hgt; }
static int cmotive_stl_tree_any_min_i64(void *h, int64_t *out) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; if(!t||!t->values.count||!out) return -1; *out=((int64_t*)t->values.data)[0]; return 0; }
static int cmotive_stl_tree_any_max_i64(void *h, int64_t *out) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; if(!t||!t->values.count||!out) return -1; *out=((int64_t*)t->values.data)[t->values.count-1]; return 0; }
static void* cmotive_stl_tree_any_begin(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; return t?t->values.data:NULL; }
static void* cmotive_stl_tree_any_end(void *h) { CMotive_AnyTree *t=(CMotive_AnyTree*)h; return t?(t->values.data+(size_t)t->values.count*sizeof(int64_t)):NULL; }

static int cmotive_sys_algorithms_is_sorted_i64(const int64_t *a, uint64_t n) { uint64_t i; if(!a) return 1; for(i=1;i<n;i++) if(a[i-1]>a[i]) return 0; return 1; }
static void cmotive_sys_algorithms_reverse_i64(int64_t *a, uint64_t n) { uint64_t i; if(!a) return; for(i=0;i<n/2;i++){ int64_t t=a[i]; a[i]=a[n-1-i]; a[n-1-i]=t; } }
static void cmotive_sys_algorithms_rotate_left_i64(int64_t *a, uint64_t n, uint64_t by) { uint64_t i; if(!a||!n) return; by%=n; for(i=0;i<by;i++){ int64_t first=a[0]; memmove(a,a+1,(size_t)(n-1)*sizeof(int64_t)); a[n-1]=first; } }
static uint64_t cmotive_sys_algorithms_unique_i64(int64_t *a, uint64_t n) { uint64_t i,w=0; if(!a) return 0; for(i=0;i<n;i++) if(w==0||a[i]!=a[w-1]) a[w++]=a[i]; return w; }

static const char *__cmotive_io_output_fmt = "%s";
static const char *__cmotive_io_input_fmt = "%s";
static void cmotive_sys_io_set_output_format(const char *fmt) { __cmotive_io_output_fmt = fmt ? fmt : "%s"; }
static void cmotive_sys_io_set_input_format(const char *fmt) { __cmotive_io_input_fmt = fmt ? fmt : "%s"; }
static int cmotive_sys_io_flush(void) { return fflush(stdout); }
static int cmotive_sys_io_scan_int(int64_t *out) { return scanf(__cmotive_io_input_fmt ? __cmotive_io_input_fmt : "%lld", out); }
static int cmotive_sys_io_scan_string(char *out) { return scanf(__cmotive_io_input_fmt ? __cmotive_io_input_fmt : "%s", out); }
/* ---- end CMotive object-model helpers ---- */

