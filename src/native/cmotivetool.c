/*
 * CMotive native C frontend/tool driver.
 * Replaces the previous Python bootstrap executable path for cmotive,
 * cmotive++, cmotivepp and CMotiveSymsToDebugFile.
 */
#define _CRT_SECURE_NO_WARNINGS 1
#if !defined(_WIN32)
#define _XOPEN_SOURCE 700
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <direct.h>
#include <process.h>
#include <windows.h>
#define strtok_r strtok_s
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#define PATH_SEP '/'
#endif

#define CMOTIVE_VERSION "0.2.4-cfrontend"

static void *xmalloc(size_t n) { void *p = malloc(n ? n : 1); if (!p) { fprintf(stderr, "cmotive: out of memory\n"); exit(99); } return p; }
static void *xrealloc(void *p, size_t n) { void *r = realloc(p, n ? n : 1); if (!r) { fprintf(stderr, "cmotive: out of memory\n"); exit(99); } return r; }
static char *xstrdup(const char *s) { size_t n; char *p; if (!s) s = ""; n = strlen(s) + 1u; p = (char*)xmalloc(n); memcpy(p, s, n); return p; }
static int streq(const char *a, const char *b) { return strcmp(a ? a : "", b ? b : "") == 0; }
static int starts_with(const char *s, const char *p) { return s && p && strncmp(s, p, strlen(p)) == 0; }
static int ends_with(const char *s, const char *suf) { size_t n, m; if (!s || !suf) return 0; n = strlen(s); m = strlen(suf); return n >= m && memcmp(s + n - m, suf, m) == 0; }
static int is_ident_start(int c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident_part(int c) { return isalnum((unsigned char)c) || c == '_'; }
static char *trim_inplace(char *s) { char *e; if (!s) return s; while (*s && isspace((unsigned char)*s)) s++; e = s + strlen(s); while (e > s && isspace((unsigned char)e[-1])) *--e = 0; return s; }
static char *path_dirname(const char *path) { const char *p, *last = NULL; char *r; if (!path) return xstrdup("."); for (p = path; *p; ++p) if (*p == '/' || *p == '\\') last = p; if (!last) return xstrdup("."); if (last == path) return xstrdup("/"); r = (char*)xmalloc((size_t)(last - path) + 1u); memcpy(r, path, (size_t)(last - path)); r[last - path] = 0; return r; }
static char *path_basename(const char *path) { const char *p, *last = path; if (!path) return xstrdup(""); for (p = path; *p; ++p) if (*p == '/' || *p == '\\') last = p + 1; return xstrdup(last); }
static char *path_join(const char *a, const char *b) { size_t na = strlen(a ? a : ""), nb = strlen(b ? b : ""); int need = na && a[na-1] != '/' && a[na-1] != '\\'; char *r = (char*)xmalloc(na + nb + (need ? 2u : 1u)); memcpy(r, a ? a : "", na); if (need) r[na++] = PATH_SEP; memcpy(r + na, b ? b : "", nb + 1u); return r; }
static int file_exists(const char *p) { FILE *f = fopen(p, "rb"); if (f) { fclose(f); return 1; } return 0; }
static char *read_file(const char *path) { FILE *f = fopen(path, "rb"); long n; char *buf; if (!f) { fprintf(stderr, "cmotive: cannot read %s: %s\n", path, strerror(errno)); return NULL; } fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET); if (n < 0) n = 0; buf = (char*)xmalloc((size_t)n + 1u); if (n && fread(buf, 1u, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); fprintf(stderr, "cmotive: short read %s\n", path); return NULL; } fclose(f); buf[n] = 0; return buf; }
static int write_file(const char *path, const char *text) { FILE *f; char *dir = path_dirname(path); if (dir && !streq(dir, ".")) { char tmp[4096]; size_t i, n = strlen(dir); if (n < sizeof(tmp)) { memcpy(tmp, dir, n + 1u); for (i = 1; i < n; ++i) if (tmp[i] == '/' || tmp[i] == '\\') { char old = tmp[i]; tmp[i] = 0; if (strlen(tmp) > 0) {
#if defined(_WIN32)
 _mkdir(tmp);
#else
 mkdir(tmp, 0777);
#endif
 } tmp[i] = old; } }
#if defined(_WIN32)
 _mkdir(dir);
#else
 mkdir(dir, 0777);
#endif
 }
 free(dir); f = fopen(path, "wb"); if (!f) { fprintf(stderr, "cmotive: cannot write %s: %s\n", path, strerror(errno)); return 1; } if (text && fputs(text, f) < 0) { fclose(f); return 1; } fclose(f); return 0; }

/* string builder */
typedef struct Str { char *s; size_t n, cap; } Str;
static void sb_init(Str *b) { b->cap = 1024; b->n = 0; b->s = (char*)xmalloc(b->cap); b->s[0] = 0; }
static void sb_reserve(Str *b, size_t add) { if (b->n + add + 1u > b->cap) { while (b->n + add + 1u > b->cap) b->cap *= 2u; b->s = (char*)xrealloc(b->s, b->cap); } }
static void sb_addn(Str *b, const char *s, size_t n) { sb_reserve(b, n); memcpy(b->s + b->n, s, n); b->n += n; b->s[b->n] = 0; }
static void sb_add(Str *b, const char *s) { if (s) sb_addn(b, s, strlen(s)); }
static void sb_ch(Str *b, char c) { sb_reserve(b, 1u); b->s[b->n++] = c; b->s[b->n] = 0; }
static void sb_printf(Str *b, const char *fmt, ...) { char stackbuf[4096]; va_list ap; int n; va_start(ap, fmt); n = vsnprintf(stackbuf, sizeof(stackbuf), fmt, ap); va_end(ap); if (n < 0) return; if ((size_t)n < sizeof(stackbuf)) { sb_addn(b, stackbuf, (size_t)n); } else { char *p = (char*)xmalloc((size_t)n + 1u); va_start(ap, fmt); vsnprintf(p, (size_t)n + 1u, fmt, ap); va_end(ap); sb_addn(b, p, (size_t)n); free(p); } }
static char *sb_take(Str *b) { char *r = b->s; b->s = NULL; b->n = b->cap = 0; return r; }

/* dynamic string vector */
typedef struct StrVec { char **v; int n, cap; } StrVec;
static void sv_push(StrVec *a, const char *s) { if (a->n == a->cap) { a->cap = a->cap ? a->cap * 2 : 8; a->v = (char**)xrealloc(a->v, (size_t)a->cap * sizeof(char*)); } a->v[a->n++] = xstrdup(s); }
static int sv_contains(StrVec *a, const char *s) { int i; for (i = 0; i < a->n; ++i) if (streq(a->v[i], s)) return 1; return 0; }

/* preprocessor */
typedef struct Macro { char *name, *value; } Macro;
typedef struct PPContext { StrVec include_dirs; StrVec seen; Macro *macros; int macro_n, macro_cap; char *root; } PPContext;
static void pp_set_macro(PPContext *pp, const char *name, const char *value) { int i; for (i = 0; i < pp->macro_n; ++i) if (streq(pp->macros[i].name, name)) { free(pp->macros[i].value); pp->macros[i].value = xstrdup(value ? value : "1"); return; } if (pp->macro_n == pp->macro_cap) { pp->macro_cap = pp->macro_cap ? pp->macro_cap * 2 : 16; pp->macros = (Macro*)xrealloc(pp->macros, (size_t)pp->macro_cap * sizeof(Macro)); } pp->macros[pp->macro_n].name = xstrdup(name); pp->macros[pp->macro_n].value = xstrdup(value ? value : "1"); pp->macro_n++; }
static const char *pp_get_macro(PPContext *pp, const char *name) { int i; for (i = 0; i < pp->macro_n; ++i) if (streq(pp->macros[i].name, name)) return pp->macros[i].value; return NULL; }
static void pp_undef_macro(PPContext *pp, const char *name) { int i; for (i = 0; i < pp->macro_n; ++i) if (streq(pp->macros[i].name, name)) { free(pp->macros[i].name); free(pp->macros[i].value); memmove(&pp->macros[i], &pp->macros[i+1], (size_t)(pp->macro_n-i-1)*sizeof(Macro)); pp->macro_n--; return; } }
static char *expand_macros(PPContext *pp, const char *line) { Str out; const char *p = line; sb_init(&out); while (*p) { if (is_ident_start((unsigned char)*p)) { const char *q = p + 1; char name[256]; size_t n; while (is_ident_part((unsigned char)*q)) q++; n = (size_t)(q - p); if (n >= sizeof(name)) n = sizeof(name) - 1u; memcpy(name, p, n); name[n] = 0; { const char *v = pp_get_macro(pp, name); sb_add(&out, v ? v : name); } p = q; } else { sb_ch(&out, *p++); } } return sb_take(&out); }
static char *resolve_include(PPContext *pp, const char *name, const char *current) { int i; char *dir = path_dirname(current), *p = path_join(dir, name); free(dir); if (file_exists(p)) return p; free(p); for (i = 0; i < pp->include_dirs.n; ++i) { p = path_join(pp->include_dirs.v[i], name); if (file_exists(p)) return p; free(p); } p = path_join(pp->root, "lib"); { char *p2 = path_join(p, name); free(p); if (file_exists(p2)) return p2; free(p2); } return NULL; }
static char *plugin_rel(const char *plugin) { Str b; const char *p; sb_init(&b); for (p = plugin; *p; ++p) { if (p[0] == ':' && p[1] == ':') { sb_ch(&b, PATH_SEP); ++p; } else if (!isspace((unsigned char)*p) && *p != ';') sb_ch(&b, *p); } return sb_take(&b); }
static char *resolve_plugin(PPContext *pp, const char *plugin, const char *current) { static const char *exts[] = {".HMOT",".HMTV",".CMOT",".CMTV",".hmot",".hmtv",".cmot",".cmtv"}; char *rel = plugin_rel(plugin); char *dir = path_dirname(current); int base_i, ext_i; StrVec bases = {0}; sv_push(&bases, dir); free(dir); for (base_i = 0; base_i < pp->include_dirs.n; ++base_i) sv_push(&bases, pp->include_dirs.v[base_i]); { char *lib = path_join(pp->root, "lib"); sv_push(&bases, lib); free(lib); } for (base_i = 0; base_i < bases.n; ++base_i) { for (ext_i = 0; ext_i < 8; ++ext_i) { Str s; char *candidate; sb_init(&s); sb_add(&s, rel); sb_add(&s, exts[ext_i]); candidate = path_join(bases.v[base_i], s.s); free(s.s); if (file_exists(candidate)) { free(rel); return candidate; } free(candidate); } } free(rel); return NULL; }
static int pp_eval_defined_expr(PPContext *pp, const char *expr) { const char *p = expr; long lhs = 0, rhs = 0; char name[128], op[3] = {0}; while (*p && !is_ident_start((unsigned char)*p) && !isdigit((unsigned char)*p)) p++; if (is_ident_start((unsigned char)*p)) { const char *q = p; const char *v; size_t n; while (is_ident_part((unsigned char)*q)) q++; n = (size_t)(q-p); if (n >= sizeof(name)) n = sizeof(name)-1; memcpy(name,p,n); name[n]=0; v = pp_get_macro(pp, name); lhs = v ? strtol(v, NULL, 0) : 0; p = q; } else lhs = strtol(p, (char**)&p, 0); while (*p && isspace((unsigned char)*p)) p++; if (strchr("<>!=", *p)) { op[0] = *p++; if (*p == '=') op[1] = *p++; } while (*p && !isdigit((unsigned char)*p) && *p != '-') p++; rhs = strtol(p, NULL, 0); if (streq(op, ">=")) return lhs >= rhs; if (streq(op, "<=")) return lhs <= rhs; if (streq(op, "==")) return lhs == rhs; if (streq(op, "!=")) return lhs != rhs; if (streq(op, ">")) return lhs > rhs; if (streq(op, "<")) return lhs < rhs; return lhs != 0; }
static int pp_eval_plugcase(PPContext *pp, const char *cond) { char up[512]; size_t i, n = strlen(cond); if (n >= sizeof(up)) n = sizeof(up)-1; for (i=0;i<n;i++) up[i]=(char)toupper((unsigned char)cond[i]); up[n]=0; if (strstr(up, "OS")) {
#if defined(_WIN32)
 if (strstr(up, "WIN32") || strstr(up, "WIN64")) return 1;
#elif defined(__APPLE__)
 if (strstr(up, "MACOS") || strstr(up, "UNIX")) return 1;
#elif defined(__linux__)
 if (strstr(up, "LINUX") || strstr(up, "UNIX")) return 1;
#else
 if (strstr(up, "UNIX")) return 1;
#endif
 }
 if (strstr(up, "PROCESSOR")) {
#if defined(__x86_64__) || defined(_M_X64)
  if (strstr(up, "X64") || strstr(up, "X86_64")) return 1;
#elif defined(__aarch64__) || defined(_M_ARM64)
  if (strstr(up, "ARM64") || strstr(up, "AARCH64") || strstr(up, "ARM")) return 1;
#endif
 }
 if (strstr(up, "ENDIAN")) { unsigned x = 1; int little = *((unsigned char*)&x); if ((little && strstr(up,"LITTLE")) || (!little && strstr(up,"BIG"))) return 1; }
 if (strstr(up, "DEFINED")) { const char *p = strchr(cond, ':'); return pp_eval_defined_expr(pp, p ? p + 1 : cond); }
 return 0; }
static char *pp_process_file(PPContext *pp, const char *path);
static char *pp_process_text(PPContext *pp, const char *text, const char *current) { Str out; char *copy = xstrdup(text), *save = NULL, *line; sb_init(&out); for (line = strtok_r(copy, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) { char *raw = trim_inplace(line); if (starts_with(raw, "#include")) { char *a = strchr(raw, '"'); char *b = a ? strchr(a+1, '"') : NULL; if (!a) { a = strchr(raw, '<'); b = a ? strchr(a+1, '>') : NULL; } if (a && b) { char name[1024]; char *res, *sub; size_t n = (size_t)(b-a-1); if (n >= sizeof(name)) n = sizeof(name)-1; memcpy(name,a+1,n); name[n]=0; res = resolve_include(pp, name, current); if (!res) { fprintf(stderr, "cmotivepp: include not found: %s\n", name); } else { sub = pp_process_file(pp, res); if (sub) { sb_add(&out, sub); sb_ch(&out, '\n'); free(sub); } free(res); } } continue; }
 if (starts_with(raw, "#define")) { char *p = raw + 7; char *name, *val; p = trim_inplace(p); name = p; while (*p && !isspace((unsigned char)*p)) p++; if (*p) *p++ = 0; val = trim_inplace(p); pp_set_macro(pp, name, *val ? val : "1"); continue; }
 if (starts_with(raw, "#undef")) { pp_undef_macro(pp, trim_inplace(raw + 6)); continue; }
 if (starts_with(raw, "Replace")) { char *p = trim_inplace(raw + 7); char *name = p; while (*p && !isspace((unsigned char)*p)) p++; if (*p) *p++=0; p = trim_inplace(p); { char *semi = strrchr(p, ';'); if (semi) *semi = 0; } pp_set_macro(pp, name, *p ? p : "1"); continue; }
 if (starts_with(raw, "Plugin")) { char *plug = trim_inplace(raw + 6); char *semi = strrchr(plug, ';'); if (semi) *semi = 0; if (starts_with(plug, "Sys::")) { sb_printf(&out, "\n/* Builtin Plugin %s handled by C frontend. */\n", plug); continue; } else { char *res = resolve_plugin(pp, plug, current); if (res) { char *sub = pp_process_file(pp, res); sb_printf(&out, "\n/* Plugin %s loaded. */\n", plug); if (sub) { sb_add(&out, sub); free(sub); } sb_printf(&out, "\n/* End Plugin %s. */\n", plug); free(res); continue; } } }
 if (starts_with(raw, "Plugswitch")) { Str block; int depth = 1; char *line2; sb_init(&block); while ((line2 = strtok_r(NULL, "\n", &save)) != NULL) { char *t = trim_inplace(line2); if (starts_with(t, "Plugswitch")) depth++; if (starts_with(t, "Plugend")) { depth--; if (depth == 0) break; } sb_add(&block, line2); sb_ch(&block, '\n'); }
 { char *bc = block.s; char *save2 = NULL; char *l; Str chosen, current_case; char cond[512] = {0}; int have = 0, active_default = 0, chose = 0; sb_init(&chosen); sb_init(&current_case); for (l = strtok_r(bc, "\n", &save2); l; l = strtok_r(NULL, "\n", &save2)) { char *tr = trim_inplace(l); if (starts_with(tr, "Plugcase") || starts_with(tr, "Plugdefault")) { if (have && !chose && (active_default || pp_eval_plugcase(pp, cond))) { sb_add(&chosen, current_case.s); chose = 1; } current_case.n = 0; current_case.s[0] = 0; have = 1; active_default = starts_with(tr, "Plugdefault"); if (!active_default) { strncpy(cond, trim_inplace(tr + 8), sizeof(cond)-1); cond[sizeof(cond)-1] = 0; } else cond[0] = 0; } else { sb_add(&current_case, l); sb_ch(&current_case, '\n'); } }
 if (have && !chose && (active_default || pp_eval_plugcase(pp, cond))) { sb_add(&chosen, current_case.s); } { char *sub = pp_process_text(pp, chosen.s, current); sb_add(&out, sub); free(sub); } free(current_case.s); free(chosen.s); free(block.s); }
 continue; }
 { char *expanded = expand_macros(pp, line); sb_add(&out, expanded); sb_ch(&out, '\n'); free(expanded); } }
 free(copy); return sb_take(&out); }
static char *pp_process_file(PPContext *pp, const char *path) { char *text, *out; if (sv_contains(&pp->seen, path)) return xstrdup(""); sv_push(&pp->seen, path); text = read_file(path); if (!text) return NULL; out = pp_process_text(pp, text, path); free(text); return out; }

/* Lexer */
typedef enum TokKind { TK_ID, TK_NUM, TK_STR, TK_CHAR, TK_OP, TK_EOL, TK_EOF } TokKind;
typedef struct Tok { TokKind kind; char *v; int line, col; } Tok;
typedef struct TokVec { Tok *v; int n, cap; } TokVec;
static void tv_push(TokVec *a, TokKind k, const char *s, size_t n, int line, int col) { if (a->n == a->cap) { a->cap = a->cap ? a->cap * 2 : 256; a->v = (Tok*)xrealloc(a->v, (size_t)a->cap * sizeof(Tok)); } a->v[a->n].kind = k; a->v[a->n].v = (char*)xmalloc(n + 1u); memcpy(a->v[a->n].v, s, n); a->v[a->n].v[n] = 0; a->v[a->n].line = line; a->v[a->n].col = col; a->n++; }
static int is_multi_op(const char *p, const char **op) { static const char *ops[] = {">>>","<<<","==","!=","<=",">=","->","::","++","--","&&","||","<<",">>","+=","-=","*=","/=","%=","&=","|=","^=","##","...",NULL}; int i; for (i=0; ops[i]; ++i) if (starts_with(p, ops[i])) { *op = ops[i]; return 1; } return 0; }
static TokVec lex_text(const char *text) { TokVec tv = {0}; int line = 1, col = 1; const char *p = text; while (*p) { const char *start = p; int sc = col; if (*p == '\r' || *p == '\n') { if (*p == '\r' && p[1] == '\n') p++; tv_push(&tv, TK_EOL, "\n", 1, line, col); p++; line++; col = 1; continue; } if (isspace((unsigned char)*p)) { while (*p && *p != '\n' && *p != '\r' && isspace((unsigned char)*p)) { p++; col++; } continue; } if (starts_with(p, "//")) { while (*p && *p != '\n' && *p != '\r') { p++; col++; } continue; } if (starts_with(p, "/*")) { p += 2; col += 2; while (*p && !starts_with(p, "*/")) { if (*p == '\n') { line++; col = 1; p++; } else { p++; col++; } } if (*p) { p += 2; col += 2; } continue; } if (*p == '"' || *p == '\'') { char quote = *p++; col++; while (*p) { if (*p == '\\' && p[1]) { p += 2; col += 2; continue; } if (*p == quote) { p++; col++; break; } if (*p == '\n') { line++; col = 1; p++; } else { p++; col++; } } tv_push(&tv, quote == '"' ? TK_STR : TK_CHAR, start, (size_t)(p-start), line, sc); continue; } if (is_ident_start((unsigned char)*p)) { p++; col++; while (is_ident_part((unsigned char)*p)) { p++; col++; } tv_push(&tv, TK_ID, start, (size_t)(p-start), line, sc); continue; } if (isdigit((unsigned char)*p)) { p++; col++; while (isalnum((unsigned char)*p) || *p == '.' || *p == '_' || *p == '+' || *p == '-') { if ((*p == '+' || *p == '-') && !(p[-1] == 'e' || p[-1] == 'E')) break; p++; col++; } tv_push(&tv, TK_NUM, start, (size_t)(p-start), line, sc); continue; } { const char *op = NULL; if (is_multi_op(p, &op)) { tv_push(&tv, TK_OP, op, strlen(op), line, col); p += strlen(op); col += (int)strlen(op); } else { tv_push(&tv, TK_OP, p, 1, line, col); p++; col++; } } } tv_push(&tv, TK_EOF, "", 0, line, col); return tv; }

/* AST */
typedef struct Param { char *name, *type; } Param;
typedef struct Field { char *name, *type, *init; int block; } Field;
typedef struct Func { char *name, *ret, *package, *method_of; Param *params; int param_n, param_cap; Tok *body; int body_n; int ctor, dtor, pure, fptr; char *op; char *hit_sender, *hit_id; } Func;
typedef struct Class { char *name, *base, *package; Field *fields; int field_n, field_cap; Func *methods; int method_n, method_cap; } Class;
typedef struct Global { char *name, *type, *init, *package; } Global;
typedef struct TemplateUse { char *base, *arg1, *arg2; } TemplateUse;
typedef struct Program { Class *classes; int class_n, class_cap; Func *funcs; int func_n, func_cap; Global *globals; int global_n, global_cap; Field *dyn_fields; int dyn_n, dyn_cap; char *dyn_name; TemplateUse *tuses; int tuse_n, tuse_cap; } Program;
static void add_param(Func *f, const char *name, const char *type) { if (f->param_n == f->param_cap) { f->param_cap = f->param_cap ? f->param_cap * 2 : 4; f->params = (Param*)xrealloc(f->params, (size_t)f->param_cap * sizeof(Param)); } f->params[f->param_n].name=xstrdup(name); f->params[f->param_n].type=xstrdup(type); f->param_n++; }
static void add_field_arr(Field **arr, int *n, int *cap, const char *name, const char *type, const char *init, int block) { if (*n == *cap) { *cap = *cap ? *cap * 2 : 8; *arr = (Field*)xrealloc(*arr, (size_t)*cap * sizeof(Field)); } (*arr)[*n].name=xstrdup(name); (*arr)[*n].type=xstrdup(type); (*arr)[*n].init=xstrdup(init ? init : ""); (*arr)[*n].block=block; (*n)++; }
static Class *prog_add_class(Program *p, const char *name, const char *base, const char *pkg) { if (p->class_n == p->class_cap) { p->class_cap = p->class_cap ? p->class_cap * 2 : 8; p->classes = (Class*)xrealloc(p->classes, (size_t)p->class_cap * sizeof(Class)); } memset(&p->classes[p->class_n],0,sizeof(Class)); p->classes[p->class_n].name=xstrdup(name); p->classes[p->class_n].base=xstrdup(base?base:""); p->classes[p->class_n].package=xstrdup(pkg?pkg:"StartPackage"); return &p->classes[p->class_n++]; }
static Func *prog_add_func(Program *p, Func f) { if (p->func_n == p->func_cap) { p->func_cap = p->func_cap ? p->func_cap * 2 : 16; p->funcs = (Func*)xrealloc(p->funcs, (size_t)p->func_cap * sizeof(Func)); } p->funcs[p->func_n] = f; return &p->funcs[p->func_n++]; }
static void class_add_method(Class *c, Func f) { if (c->method_n == c->method_cap) { c->method_cap = c->method_cap ? c->method_cap * 2 : 8; c->methods = (Func*)xrealloc(c->methods, (size_t)c->method_cap * sizeof(Func)); } c->methods[c->method_n++] = f; }
static void prog_add_global(Program *p, const char *name, const char *type, const char *init, const char *pkg) { if (p->global_n == p->global_cap) { p->global_cap = p->global_cap ? p->global_cap * 2 : 8; p->globals = (Global*)xrealloc(p->globals, (size_t)p->global_cap * sizeof(Global)); } p->globals[p->global_n].name=xstrdup(name); p->globals[p->global_n].type=xstrdup(type); p->globals[p->global_n].init=xstrdup(init?init:"0"); p->globals[p->global_n].package=xstrdup(pkg?pkg:"StartPackage"); p->global_n++; }
static Class *find_class(Program *p, const char *name) { int i; for (i=0;i<p->class_n;i++) if (streq(p->classes[i].name,name)) return &p->classes[i]; return NULL; }
static Func *find_func(Program *p, const char *name) { int i; for (i=0;i<p->func_n;i++) if (streq(p->funcs[i].name,name)) return &p->funcs[i]; return NULL; }

/* parser */
typedef struct Parser { Tok *t; int n, i; Program *prog; char *package; } Parser;
static Tok *pk(Parser *p, int off) { int j = p->i + off; if (j >= p->n) j = p->n - 1; return &p->t[j]; }
static void skip_eol(Parser *p) { while (pk(p,0)->kind == TK_EOL) p->i++; }
static int tok_is(Parser *p, const char *s) { return streq(pk(p,0)->v, s); }
static int parser_accept(Parser *p, const char *s) { skip_eol(p); if (tok_is(p,s)) { p->i++; return 1; } return 0; }
static char *take_value(Parser *p) { char *r; skip_eol(p); r = xstrdup(pk(p,0)->v); p->i++; return r; }
static void parse_error(Parser *p, const char *msg) { fprintf(stderr, "cmotive parse error at %d:%d: %s near '%s'\n", pk(p,0)->line, pk(p,0)->col, msg, pk(p,0)->v); }
static int is_type_start(Tok *t) { static const char *types[] = {"Boolean","Bool","Char","Char16","Char32","Uchar","I16","Int16","I32","Int32","I64","Int","U16","Uint16","U32","Uint32","U64","Uint","Float","Double","Ldouble","Void","Type","Dynamic","Tstore","ThreadStore","Vector","Map","BinarySearchTree","CString","Path","Filesystem","Socket","Net","Thread","Threading","Algorithms","OStream","Formatter","Character","StringParser","Wide16String","Wide32String","Mutex",NULL}; int i; if (t->kind != TK_ID) return 0; for (i=0; types[i]; ++i) if (streq(t->v, types[i])) return 1; return 1; }
static char *collect_type(Parser *p) {
 Str b;
 int depth = 0;
 int consumed_id = 0;
 sb_init(&b);
 skip_eol(p);
 while (p->i < p->n) {
  Tok *t = pk(p,0);
  if (t->kind == TK_EOL) {
   int j = p->i + 1;
   while (j < p->n && p->t[j].kind == TK_EOL) j++;
   if (b.n && depth == 0 && !(streq(b.s,"Tstore") || streq(b.s,"ThreadStore") || streq(b.s,"Global"))) break;
   p->i++;
   continue;
  }
  if (depth == 0 && (streq(t->v, ":") || streq(t->v, ",") || streq(t->v, "(") || streq(t->v, ")") || streq(t->v, "=") || streq(t->v, ";") || streq(t->v, "{"))) break;
  if (depth == 0 && consumed_id > 0 && t->kind == TK_ID && !(streq(b.s,"Tstore") || streq(b.s,"ThreadStore") || streq(b.s,"Global"))) break;
  if (streq(t->v, "<") || streq(t->v, "[") || streq(t->v, "(")) depth++;
  if (streq(t->v, ">") || streq(t->v, "]") || streq(t->v, ")")) depth--;
  if (b.n && !strchr("*[]<>,", t->v[0]) && b.s[b.n-1] != '<' && b.s[b.n-1] != ',' && b.s[b.n-1] != '*') sb_ch(&b, ' ');
  sb_add(&b, t->v);
  if (t->kind == TK_ID) consumed_id++;
  p->i++;
  if (depth <= 0 && (streq(pk(p,0)->v,"*") || streq(pk(p,0)->v,"[") || streq(pk(p,0)->v,"<"))) continue;
 }
 return sb_take(&b);
}
static char *collect_until(Parser *p, const char *end) { Str b; int depth = 0; sb_init(&b); while (p->i < p->n) { Tok *t = pk(p,0); if (depth == 0 && streq(t->v, end)) break; if (streq(t->v, "(") || streq(t->v,"[") || streq(t->v,"{")) depth++; if (streq(t->v, ")") || streq(t->v,"]") || streq(t->v,"}")) depth--; if (t->kind != TK_EOL) { if (b.n) sb_ch(&b, ' '); sb_add(&b, t->v); } p->i++; } return sb_take(&b); }
static void collect_body(Parser *p, Func *f) { int start, depth = 0, end; skip_eol(p); if (!parser_accept(p, "{")) { f->body = NULL; f->body_n = 0; return; } start = p->i; depth = 1; while (p->i < p->n && depth) { if (streq(pk(p,0)->v,"{")) depth++; else if (streq(pk(p,0)->v,"}")) { depth--; if (depth == 0) break; } p->i++; } end = p->i; f->body_n = end - start; if (f->body_n > 0) { f->body = (Tok*)xmalloc((size_t)f->body_n * sizeof(Tok)); memcpy(f->body, &p->t[start], (size_t)f->body_n * sizeof(Tok)); } parser_accept(p, "}"); }
static char *op_name(const char *op) { if (streq(op,"+")) return xstrdup("Plus"); if (streq(op,"-")) return xstrdup("Minus"); if (streq(op,"*")) return xstrdup("Multiply"); if (streq(op,"/")) return xstrdup("Divide"); if (streq(op,"%")) return xstrdup("Modulo"); if (streq(op,"==")) return xstrdup("Equal"); return xstrdup("Op"); }
static void parse_params_until_paren(Parser *p, Func *f) { while (p->i < p->n) { skip_eol(p); if (streq(pk(p,0)->v, "(")) break; if (pk(p,0)->kind == TK_ID && streq(pk(p,1)->v, ":")) { char *name = take_value(p); parser_accept(p, ":"); { char *type = collect_type(p); add_param(f, name, type); free(type); } free(name); if (parser_accept(p, ",")) continue; } else { p->i++; } } }
static Func parse_func_formal(Parser *p, const char *method_of, const char *forced_name, const char *forced_ret, int ctor, int dtor) { Func f; memset(&f,0,sizeof(f)); f.package=xstrdup(p->package); f.method_of=xstrdup(method_of?method_of:""); f.ctor=ctor; f.dtor=dtor; if (forced_ret) f.ret=xstrdup(forced_ret); else f.ret = dtor ? xstrdup("Void") : (ctor ? xstrdup("Void") : collect_type(p)); if (forced_name) f.name=xstrdup(forced_name); else f.name=take_value(p); if (streq(f.name,"Operation")) { char *op = take_value(p); char *on = op_name(op); free(f.name); f.name=xstrdup("Operation"); f.op=on; free(op); }
 parse_params_until_paren(p,&f); parser_accept(p,"("); parser_accept(p,")"); skip_eol(p); if (parser_accept(p,"=")) { parser_accept(p,"0"); parser_accept(p,";"); f.pure=1; } else collect_body(p,&f); return f; }
static Func parse_func_keyword(Parser *p, const char *method_of) { Func f; memset(&f,0,sizeof(f)); f.package=xstrdup(p->package); f.method_of=xstrdup(method_of?method_of:""); parser_accept(p,"func"); f.name=take_value(p); parser_accept(p,"("); while (!parser_accept(p,")")) { if (pk(p,0)->kind == TK_ID) { char *n = take_value(p); char *type = xstrdup("I32"); if (parser_accept(p,":")) { free(type); type=collect_type(p); } add_param(&f,n,type); free(n); free(type); parser_accept(p,","); } else p->i++; }
 f.ret=xstrdup("I32"); if (parser_accept(p,":")) { free(f.ret); f.ret=collect_type(p); } collect_body(p,&f); return f; }
static void parse_class(Parser *p) {
 char *name, *base = NULL;
 Class *c;
 int access_depth = 0;
 if (!parser_accept(p,"Class")) parser_accept(p,"class");
 name = take_value(p);
 skip_eol(p);
 if (parser_accept(p,"Inherits") || parser_accept(p,"extends")) {
  base = take_value(p);
  if (pk(p,0)->kind == TK_ID && (streq(pk(p,0)->v,"Public") || streq(pk(p,0)->v,"Private") || streq(pk(p,0)->v,"Protected"))) p->i++;
 }
 c = prog_add_class(p->prog, name, base ? base : "", p->package);
 free(name);
 if (base) free(base);
 parser_accept(p,"{");
 while (p->i < p->n) {
  skip_eol(p);
  if (streq(pk(p,0)->v,"}")) {
   p->i++;
   if (access_depth > 0) { access_depth--; continue; }
   break;
  }
  if (parser_accept(p,"Public") || parser_accept(p,"Private") || parser_accept(p,"Protected")) {
   if (parser_accept(p,"{")) access_depth++;
   continue;
  }
  if (parser_accept(p,";")) continue;
  if (parser_accept(p,"Overridable") || parser_accept(p,"virtual")) {
   Func f = parse_func_formal(p, c->name, NULL, NULL, 0, 0);
   class_add_method(c, f);
   continue;
  }
  if (parser_accept(p,"Hit")) {
   char *sender = xstrdup(""), *hid = xstrdup("");
   if (parser_accept(p,":")) { free(hid); hid = take_value(p); }
   else if (pk(p,0)->kind == TK_ID && streq(pk(p,1)->v, ":")) { free(sender); sender = take_value(p); parser_accept(p,":"); free(hid); hid = take_value(p); }
   { Func f = parse_func_formal(p,c->name,NULL,NULL,0,0); f.hit_sender=sender; f.hit_id=hid; class_add_method(c,f); }
   continue;
  }
  if (streq(pk(p,0)->v,"~")) {
   p->i++;
   { char *dn = take_value(p); Func f = parse_func_formal(p,c->name,dn,"Void",0,1); free(dn); class_add_method(c,f); }
   continue;
  }
  if (streq(pk(p,0)->v, c->name)) {
   char *cn = take_value(p); Func f = parse_func_formal(p,c->name,cn,"Void",1,0); free(cn); class_add_method(c,f); continue;
  }
  if (streq(pk(p,0)->v,"func")) { Func f = parse_func_keyword(p,c->name); class_add_method(c,f); continue; }
  if (pk(p,0)->kind == TK_ID && streq(pk(p,1)->v, ":")) {
   char *fname = take_value(p);
   char *type, *init = NULL;
   int block = 0;
   parser_accept(p,":");
   type = collect_type(p);
   if (parser_accept(p,"=")) init = collect_until(p,";");
   parser_accept(p,";");
   if (parser_accept(p,"Block")) block = 1;
   add_field_arr(&c->fields,&c->field_n,&c->field_cap,fname,type,init?init:"0",block);
   free(fname); free(type); if (init) free(init);
   continue;
  }
  if (is_type_start(pk(p,0))) { Func f = parse_func_formal(p, c->name, NULL, NULL, 0, 0); class_add_method(c,f); continue; }
  p->i++;
 }
 parser_accept(p,";");
}
static void parse_dynamic_struct(Parser *p) { char *name; parser_accept(p,"Dynamic"); parser_accept(p,"Struct"); name=take_value(p); free(p->prog->dyn_name); p->prog->dyn_name=xstrdup(name); parser_accept(p,"{"); while (!parser_accept(p,"}")) { char *type, *fname; skip_eol(p); if (streq(pk(p,0)->v,"}")) continue; type=collect_type(p); fname=take_value(p); parser_accept(p,";"); add_field_arr(&p->prog->dyn_fields,&p->prog->dyn_n,&p->prog->dyn_cap,fname,type,"0",0); free(type); free(fname); } parser_accept(p,";"); free(name); }
static void parse_dynamic_expand(Parser *p) { char *name = take_value(p); (void)name; parser_accept(p,"Expand"); parser_accept(p,"{"); while (!parser_accept(p,"}")) { char *type, *fname; skip_eol(p); if (streq(pk(p,0)->v,"}")) continue; type=collect_type(p); fname=take_value(p); parser_accept(p,";"); add_field_arr(&p->prog->dyn_fields,&p->prog->dyn_n,&p->prog->dyn_cap,fname,type,"0",0); free(type); free(fname); } parser_accept(p,";"); free(name); }
static void skip_template(Parser *p) { parser_accept(p,"Template"); while (p->i < p->n && !streq(pk(p,0)->v,"Class") && !is_type_start(pk(p,0))) p->i++; if (streq(pk(p,0)->v,"Class")) { int depth=0; while (p->i < p->n) { if (streq(pk(p,0)->v,"{")) depth++; else if (streq(pk(p,0)->v,"}")) { depth--; if (depth == 0) { p->i++; break; } } p->i++; } parser_accept(p,";"); } else { Func tmp = parse_func_formal(p,NULL,NULL,NULL,0,0); (void)tmp; } }
static void parse_fptr(Parser *p) { Func f; memset(&f,0,sizeof(f)); parser_accept(p,"Fptr"); f.fptr=1; f.package=xstrdup(p->package); f.method_of=xstrdup(""); f.ret=collect_type(p); f.name=take_value(p); parse_params_until_paren(p,&f); parser_accept(p,"("); parser_accept(p,")"); parser_accept(p,";"); prog_add_func(p->prog,f); }
static void parse_global(Parser *p) { char *name, *type, *init = NULL; if (parser_accept(p,"Global")) { name=take_value(p); parser_accept(p,":"); type=collect_type(p); } else { name=take_value(p); parser_accept(p,":"); if (parser_accept(p,"Global")) type=collect_type(p); else type=collect_type(p); } if (parser_accept(p,"=")) init = collect_until(p,";"); parser_accept(p,";"); prog_add_global(p->prog,name,type,init?init:"0",p->package); free(name); free(type); if (init) free(init); }
static void parse_package(Parser *p) { Str b; if (!parser_accept(p,"Package")) parser_accept(p,"package"); sb_init(&b); while (p->i < p->n && !parser_accept(p,";")) { if (pk(p,0)->kind != TK_EOL) sb_add(&b, pk(p,0)->v); p->i++; } if (b.n) { free(p->package); p->package = sb_take(&b); } else free(b.s); }
static void parse_program(Parser *p) { while (p->i < p->n && pk(p,0)->kind != TK_EOF) { skip_eol(p); if (pk(p,0)->kind == TK_EOF) break; if (parser_accept(p,";")) continue; if (streq(pk(p,0)->v,"Package") || streq(pk(p,0)->v,"package")) { parse_package(p); continue; } if (streq(pk(p,0)->v,"Plugin")) { while (p->i < p->n && !parser_accept(p,";") && pk(p,0)->kind != TK_EOL) p->i++; continue; } if (streq(pk(p,0)->v,"Class") || streq(pk(p,0)->v,"class")) { parse_class(p); continue; } if (streq(pk(p,0)->v,"Dynamic") && streq(pk(p,1)->v,"Struct")) { parse_dynamic_struct(p); continue; } if (p->prog->dyn_name && streq(pk(p,0)->v,p->prog->dyn_name) && streq(pk(p,1)->v,"Expand")) { parse_dynamic_expand(p); continue; } if (streq(pk(p,0)->v,"Template")) { skip_template(p); continue; } if (streq(pk(p,0)->v,"Fptr")) { parse_fptr(p); continue; } if (streq(pk(p,0)->v,"Global") || (pk(p,0)->kind == TK_ID && streq(pk(p,1)->v,":"))) { parse_global(p); continue; } if (streq(pk(p,0)->v,"func")) { Func f = parse_func_keyword(p,NULL); prog_add_func(p->prog,f); continue; } if (streq(pk(p,0)->v,"Hit")) { char *sender=xstrdup(""), *hid=xstrdup(""); parser_accept(p,"Hit"); if (parser_accept(p,":")) { free(hid); hid=take_value(p); } else if (pk(p,0)->kind == TK_ID && streq(pk(p,1)->v,":")) { free(sender); sender=take_value(p); parser_accept(p,":"); free(hid); hid=take_value(p); } { Func f = parse_func_formal(p,NULL,NULL,NULL,0,0); f.hit_sender=sender; f.hit_id=hid; prog_add_func(p->prog,f); } continue; }
 if (is_type_start(pk(p,0))) { Func f = parse_func_formal(p,NULL,NULL,NULL,0,0); prog_add_func(p->prog,f); continue; }
 parse_error(p,"unrecognized declaration"); p->i++; } }

static int block_end(Tok *t, int n, int open);
static char *join_tokens(Tok *t, int a, int b);
static void scan_dynamic_expand_tokens(Program *prog, Tok *t, int n)
{
 int i;
 if (!prog->dyn_name) return;
 for (i = 0; i + 3 < n; ++i) {
  if (streq(t[i].v, prog->dyn_name) && streq(t[i+1].v, "Expand")) {
   int open = i + 2;
   int close;
   int j;
   while (open < n && !streq(t[open].v, "{")) open++;
   if (open >= n) continue;
   close = block_end(t, n, open);
   j = open + 1;
   while (j < close) {
    int ts, te;
    char *type, *fname;
    while (j < close && t[j].kind == TK_EOL) j++;
    if (j >= close || streq(t[j].v, "}")) break;
    ts = j;
    while (j < close && !streq(t[j].v, ";")) j++;
    te = j;
    if (te - ts >= 2) {
     type = join_tokens(t, ts, te - 1);
     fname = xstrdup(t[te - 1].v);
     add_field_arr(&prog->dyn_fields, &prog->dyn_n, &prog->dyn_cap, fname, type, "0", 0);
     free(type); free(fname);
    }
    if (j < close && streq(t[j].v, ";")) j++;
   }
  }
 }
}
static void scan_all_dynamic_expands(Program *prog)
{
 int i;
 for (i = 0; i < prog->func_n; ++i) scan_dynamic_expand_tokens(prog, prog->funcs[i].body, prog->funcs[i].body_n);
 for (i = 0; i < prog->class_n; ++i) {
  int j;
  for (j = 0; j < prog->classes[i].method_n; ++j) scan_dynamic_expand_tokens(prog, prog->classes[i].methods[j].body, prog->classes[i].methods[j].body_n);
 }
}

/* codegen helpers */
typedef struct Var { char *name, *type; int pointer; } Var;
typedef struct VarTab { Var *v; int n, cap; } VarTab;
static void vt_add(VarTab *vt, const char *name, const char *type, int pointer) { int i; for (i=0;i<vt->n;i++) if (streq(vt->v[i].name,name)) { free(vt->v[i].type); vt->v[i].type=xstrdup(type); vt->v[i].pointer=pointer; return; } if (vt->n == vt->cap) { vt->cap=vt->cap?vt->cap*2:16; vt->v=(Var*)xrealloc(vt->v,(size_t)vt->cap*sizeof(Var)); } vt->v[vt->n].name=xstrdup(name); vt->v[vt->n].type=xstrdup(type); vt->v[vt->n].pointer=pointer; vt->n++; }
static Var *vt_find(VarTab *vt, const char *name) { int i; for (i=vt->n-1;i>=0;i--) if (streq(vt->v[i].name,name)) return &vt->v[i]; return NULL; }
static char *safe_pkg(const char *pkg) { Str b; const char *p; sb_init(&b); for (p=pkg?pkg:"StartPackage"; *p; ++p) { if (p[0]==':' && p[1]==':') { sb_add(&b,"__"); ++p; } else if (isalnum((unsigned char)*p) || *p=='_') sb_ch(&b,*p); else sb_ch(&b,'_'); } return sb_take(&b); }
static const char *type_suffix(const char *t) { if (!t) return "V"; if (strstr(t,"I32")||strstr(t,"Int32")) return "I32"; if (strstr(t,"I64")||streq(t,"Int")) return "I64"; if (strstr(t,"Char")) return "Char"; if (strstr(t,"Double")) return "Double"; if (strstr(t,"Float")) return "Float"; return "Obj"; }
static char *mangle_class_member(Class *c, const char *name, Func *f) { Str b; char *pkg = safe_pkg(c ? c->package : "StartPackage"); sb_init(&b); sb_add(&b,pkg); sb_add(&b,"__"); sb_add(&b,c?c->name:""); sb_add(&b,"__"); if (f && f->ctor) sb_add(&b,"ctor"); else if (f && f->dtor) sb_add(&b,"dtor"); else if (f && f->op) { sb_add(&b,"Operation__"); sb_add(&b,f->op); } else sb_add(&b,name); if (f && f->ctor && f->param_n) { int i; for (i=0;i<f->param_n;i++) { sb_add(&b,"__"); sb_add(&b,type_suffix(f->params[i].type)); } } free(pkg); return sb_take(&b); }
static char *mangle_func(Func *f) { Str b; char *pkg = safe_pkg(f->package); if (streq(f->name,"main")) { free(pkg); return xstrdup("main"); } sb_init(&b); sb_add(&b,pkg); sb_add(&b,"__"); sb_add(&b,f->name); free(pkg); return sb_take(&b); }
static char *ctor_symbol_to_new(const char *ctor)
{
    const char *pos = strstr(ctor ? ctor : "", "__ctor");
    Str b;
    if (!pos) return xstrdup(ctor ? ctor : "");
    sb_init(&b);
    sb_addn(&b, ctor, (size_t)(pos - ctor));
    sb_add(&b, "__new");
    sb_add(&b, pos + 6);
    return sb_take(&b);
}
static char *ctype(const char *type) {
 Str b;
 const char *t = type ? type : "I32";
 while (*t && isspace((unsigned char)*t)) t++;
 if (starts_with(t,"Global ")) return ctype(t + 7);
 if (starts_with(t,"Tstore ")) return ctype(t + 7);
 if (starts_with(t,"ThreadStore ")) return ctype(t + 12);
 if (starts_with(t,"Vector")) return xstrdup("CMotive_Vector_Int");
 if (starts_with(t,"Map")) return xstrdup("CMotive_Map_CharPtr_Int");
 if (starts_with(t,"BinarySearchTree")) return xstrdup("CMotive_Tree_Int");
 if (streq(t,"Boolean")||streq(t,"Bool")||streq(t,"boolean")) return xstrdup("int");
 if (streq(t,"Char")||streq(t,"char")) return xstrdup("char");
 if (streq(t,"Char*")||streq(t,"Char *")) return xstrdup("char*");
 if (streq(t,"Char**")||streq(t,"Char **")) return xstrdup("char**");
 if (streq(t,"Char16")) return xstrdup("uint16_t");
 if (streq(t,"Char16*")||streq(t,"Char16 *")) return xstrdup("uint16_t*");
 if (streq(t,"Char32")) return xstrdup("uint32_t");
 if (streq(t,"Char32*")||streq(t,"Char32 *")) return xstrdup("uint32_t*");
 if (streq(t,"Uchar")) return xstrdup("unsigned char");
 if (streq(t,"I16")||streq(t,"Int16")) return xstrdup("int16_t");
 if (streq(t,"I32")||streq(t,"Int32")||streq(t,"int")) return xstrdup("int32_t");
 if (streq(t,"I64")||streq(t,"Int")) return xstrdup("int64_t");
 if (streq(t,"U16")||streq(t,"Uint16")) return xstrdup("uint16_t");
 if (streq(t,"U32")||streq(t,"Uint32")) return xstrdup("uint32_t");
 if (streq(t,"U64")||streq(t,"Uint")) return xstrdup("uint64_t");
 if (streq(t,"Float")) return xstrdup("float");
 if (streq(t,"Double")) return xstrdup("double");
 if (streq(t,"Ldouble")) return xstrdup("long double");
 if (streq(t,"Void")||streq(t,"void")) return xstrdup("void");
 if (streq(t,"Void*")||streq(t,"Void *")||streq(t,"Type")||streq(t,"Dynamic")) return xstrdup("void*");
 sb_init(&b);
 while (*t) {
  if (*t == '<') { sb_ch(&b,'_'); t++; continue; }
  if (*t == '>' || *t == ',' || isspace((unsigned char)*t)) { t++; continue; }
  if (*t == '*') { sb_ch(&b,'*'); t++; continue; }
  sb_ch(&b,*t++);
 }
 return sb_take(&b);
}
static int type_decl_is_pointer(const char *type) {
 const char *p = type ? type : "";
 size_t n;
 while (*p && isspace((unsigned char)*p)) p++;
 n = strlen(p);
 while (n > 0 && isspace((unsigned char)p[n-1])) n--;
 return n > 0 && p[n-1] == '*';
}
static const char *skip_storage_prefix_const(const char *type) {
 const char *t = type ? type : "";
 while (*t && isspace((unsigned char)*t)) t++;
 if (starts_with(t,"Global ")) t += 7;
 while (*t && isspace((unsigned char)*t)) t++;
 if (starts_with(t,"Tstore ")) t += 7;
 else if (starts_with(t,"ThreadStore ")) t += 12;
 while (*t && isspace((unsigned char)*t)) t++;
 return t;
}
static int type_is_class(Program *p, const char *type) { char base[256]; size_t i=0; const char *t=skip_storage_prefix_const(type); while (*t && *t!='*' && *t!='<' && !isspace((unsigned char)*t) && i<sizeof(base)-1) base[i++]=*t++; base[i]=0; return find_class(p,base)!=NULL; }
static Class *class_from_type(Program *p, const char *type) { char base[256]; size_t i=0; const char *t=skip_storage_prefix_const(type); while (*t && *t!='*' && *t!='<' && !isspace((unsigned char)*t) && i<sizeof(base)-1) base[i++]=*t++; base[i]=0; return find_class(p,base); }
static int type_is_builtin_obj(const char *type) { static const char *objs[] = {"OStream","Formatter","Path","Filesystem","Socket","Net","Thread","Threading","Algorithms","CString","Character","StringParser","Wide16String","Wide32String","Mutex",NULL}; int i; for (i=0; objs[i]; ++i) if (streq(type, objs[i]) || starts_with(type,"Vector") || starts_with(type,"Map") || starts_with(type,"BinarySearchTree")) return 1; return 0; }

static char *expr_from_tokens(Program *prog, Tok *t, int n, VarTab *vt, Class *curcls, int as_lvalue);
static void append_expr(Str *out, Program *prog, Tok *t, int a, int b, VarTab *vt, Class *curcls) { char *e = expr_from_tokens(prog, t+a, b-a, vt, curcls, 0); sb_add(out,e); free(e); }
static int matching_paren(Tok *t, int n, int open) { int d=0,i; for (i=open;i<n;i++) { if (streq(t[i].v,"(")) d++; else if (streq(t[i].v,")")) { d--; if (d==0) return i; } } return n-1; }
static char *map_sys_call(const char *full) { struct M { const char *a,*b; } m[] = {
 {"Sys::IO::print","cmotive_sys_stdio_print"},{"Sys::IO::println","cmotive_sys_stdio_println"},{"Sys::IO::printf","printf"},{"Sys::IO::scanf","scanf"},
 {"Sys::STL::VectorCreate","cmotive_sys_stl_vector_create"},{"Sys::STL::VectorPushI64","cmotive_sys_stl_vector_push_i64"},{"Sys::STL::VectorSortI64","cmotive_sys_stl_vector_sort_i64"},{"Sys::STL::VectorGetI64","cmotive_sys_stl_vector_get_i64"},{"Sys::STL::VectorDestroy","cmotive_sys_stl_vector_destroy"},{"Sys::STL::DictCreate","cmotive_sys_stl_dict_create"},{"Sys::STL::DictPutI64","cmotive_sys_stl_dict_put_i64"},{"Sys::STL::DictGetI64","cmotive_sys_stl_dict_get_i64"},{"Sys::STL::DictDestroy","cmotive_sys_stl_dict_destroy"},{"Sys::STL::BinarySearchTreeCreate","cmotive_sys_stl_binary_search_tree_create"},{"Sys::STL::BinarySearchTreeInsertI64","cmotive_sys_stl_binary_search_tree_insert_i64"},{"Sys::STL::BinarySearchTreeContainsI64","cmotive_sys_stl_binary_search_tree_contains_i64"},{"Sys::STL::BinarySearchTreeDestroy","cmotive_sys_stl_binary_search_tree_destroy"},
 {"Sys::Algorithms::MinI64","cmotive_sys_algorithms_min_i64"},{"Sys::Algorithms::MaxI64","cmotive_sys_algorithms_max_i64"},{"Sys::Algorithms::CompareI32","cmotive_sys_algorithms_compare_i32"},
 {"Sys::Net::SocketTcpIPv4","cmotive_sys_net_socket_tcp_ipv4"},{"Sys::Net::SocketUdpIPv4","cmotive_sys_net_socket_udp_ipv4"},{"Sys::Net::SocketClose","cmotive_sys_net_socket_close"},
 {"Sys::Thread::Current","cmotive_sys_thread_current"},{"Sys::Thread::Yield","cmotive_sys_thread_yield"},{"Sys::Thread::MicroSleep","cmotive_sys_thread_sleep_us"},{"Sys::Thread::NanoSleep","cmotive_sys_thread_sleep_ns"},{"Sys::Thread::SleepMs","cmotive_sys_thread_sleep_ms"},
 {NULL,NULL}}; int i; for (i=0;m[i].a;i++) if (streq(full,m[i].a)) return xstrdup(m[i].b); return NULL; }
static char *map_builtin_method(const char *type, const char *method) { Str b; sb_init(&b); if (streq(type,"OStream")) { if (streq(method,"Expect")||streq(method,"expect")) sb_add(&b,"CMOStream_Expect"); else if (streq(method,"Write")||streq(method,"write")) sb_add(&b,"CMOStream_Write"); }
 else if (streq(type,"Formatter")) { if (streq(method,"Println")) sb_add(&b,"CMFormatter_Println"); }
 else if (streq(type,"Path")) { sb_add(&b,"CMPath_"); sb_add(&b,method); }
 else if (streq(type,"Filesystem")) { sb_add(&b,"CMFilesystem_"); sb_add(&b,method); }
 else if (streq(type,"Socket")) { sb_add(&b,"CMSocket_"); sb_add(&b,method); }
 else if (streq(type,"Net")) { sb_add(&b,"CMNet_"); sb_add(&b,method); }
 else if (streq(type,"Thread") || streq(type,"Threading")) { sb_add(&b,"CMThread_"); sb_add(&b,method); }
 else if (streq(type,"Algorithms")) { sb_add(&b,"CMAlgorithms_"); sb_add(&b,method); }
 else if (starts_with(type,"Vector")) { sb_add(&b,"CMVector_"); sb_add(&b,method); }
 else if (starts_with(type,"Map")) { sb_add(&b,"CMMap_"); sb_add(&b,method); }
 else if (starts_with(type,"BinarySearchTree")) { sb_add(&b,"CMTree_"); sb_add(&b,method); }
 else if (streq(type,"CString")) { sb_add(&b,"CMCString_"); sb_add(&b,method); }
 else if (streq(type,"Character")) { sb_add(&b,"CMCharacter_"); sb_add(&b,method); }
 else if (streq(type,"StringParser")) { sb_add(&b,"CMStringParser_"); sb_add(&b,method); }
 else if (streq(type,"Wide16String")) { sb_add(&b,"CMWide16_"); sb_add(&b,method); }
 else if (streq(type,"Wide32String")) { sb_add(&b,"CMWide32_"); sb_add(&b,method); }
 else if (streq(type,"Mutex")) { if (streq(method,"lock")) sb_add(&b,"CMMutex_lock"); else if (streq(method,"unlock")) sb_add(&b,"CMMutex_unlock"); }
 if (b.n==0) { free(b.s); return NULL; } return sb_take(&b); }
static char *expr_from_tokens(Program *prog, Tok *t, int n, VarTab *vt, Class *curcls, int as_lvalue) { Str out; int i; (void)as_lvalue; sb_init(&out); for (i=0;i<n;i++) { Tok *x=&t[i]; if (x->kind==TK_EOL) continue; if ((streq(x->v,"This") || streq(x->v,"this")) && i+2<n && (streq(t[i+1].v,".") || streq(t[i+1].v,"->"))) { sb_add(&out,"this->"); sb_add(&out,t[i+2].v); i+=2; continue; } if (streq(x->v,"True")||streq(x->v,"true")) { sb_add(&out,"1"); continue; } if (streq(x->v,"False")||streq(x->v,"false")) { sb_add(&out,"0"); continue; } if (streq(x->v,"Null")) { sb_add(&out,"NULL"); continue; } if (streq(x->v,"This")) { sb_add(&out,"this"); continue; } if (streq(x->v,"Sizeof")) { sb_add(&out,"sizeof"); continue; }
 /* casts */
 if (streq(x->v,"(") && i+2<n && streq(t[i+2].v,")")) { char *ct = ctype(t[i+1].v); if (!type_is_class(prog,t[i+1].v) && !type_is_builtin_obj(t[i+1].v)) { sb_printf(&out,"(%s)",ct); free(ct); i+=2; continue; } free(ct); }
 /* Sys::A::B call */
 if (i+4<n && streq(t[i+1].v,"::") && streq(t[i+3].v,"::")) { Str full; char *mapped; sb_init(&full); sb_add(&full,t[i].v); sb_add(&full,"::"); sb_add(&full,t[i+2].v); sb_add(&full,"::"); sb_add(&full,t[i+4].v); mapped = map_sys_call(full.s); free(full.s); if (mapped) { sb_add(&out,mapped); free(mapped); i+=4; continue; } }
 /* class operation overload */
 if (x->kind==TK_ID && i+2<n && t[i+1].kind==TK_OP && t[i+2].kind==TK_ID && (streq(t[i+1].v,"+")||streq(t[i+1].v,"-")||streq(t[i+1].v,"*")||streq(t[i+1].v,"/")||streq(t[i+1].v,"%")||streq(t[i+1].v,"=="))) {
   Var *lv = vt_find(vt, x->v);
   Var *rv = vt_find(vt, t[i+2].v);
   if (lv && rv && type_is_class(prog, lv->type)) {
     Class *c = class_from_type(prog, lv->type);
     char *on = op_name(t[i+1].v);
     Func *mf = NULL;
     int k;
     for (k=0; c && k<c->method_n; ++k) if (c->methods[k].op && streq(c->methods[k].op, on)) { mf=&c->methods[k]; break; }
     if (mf) {
       char *sym = mangle_class_member(c, "Operation", mf);
       sb_add(&out, sym);
       sb_ch(&out, '(');
       if (lv->pointer) sb_add(&out, x->v); else { sb_ch(&out, '&'); sb_add(&out, x->v); }
       sb_add(&out, ", ");
       if (rv->pointer && mf->param_n > 0 && !type_decl_is_pointer(mf->params[0].type)) { sb_ch(&out, '*'); sb_add(&out, t[i+2].v); }
       else sb_add(&out, t[i+2].v);
       sb_ch(&out, ')');
       free(sym); free(on); i += 2; continue;
     }
     free(on);
   }
 }
 /* function call bare -> mangle */
 if (x->kind==TK_ID && i+1<n && streq(t[i+1].v,"(") ) { Func *fn = find_func(prog,x->v); if (fn && !streq(x->v,"main")) { char *m = mangle_func(fn); sb_add(&out,m); free(m); continue; } if (streq(x->v,"pi")) { sb_add(&out,"cmotive_sys_math_pi"); continue; } if (streq(x->v,"toUpperChar")) { sb_add(&out,"toupper"); continue; } if (streq(x->v,"str_parse")) { sb_add(&out,"cmotive_sys_string_str_parse"); continue; } if (streq(x->v,"str_parse_rows")) { sb_add(&out,"cmotive_sys_string_str_parse_rows"); continue; } if (streq(x->v,"str_parse_cols")) { sb_add(&out,"cmotive_sys_string_str_parse_cols"); continue; } if (streq(x->v,"str_parse_at")) { sb_add(&out,"cmotive_sys_string_str_parse_at"); continue; } if (streq(x->v,"str_parse_free")) { sb_add(&out,"cmotive_sys_string_str_parse_free"); continue; } if (streq(x->v,"exists")) { sb_add(&out,"cmotive_sys_filesystem_exists"); continue; } if (streq(x->v,"info")) { sb_add(&out,"cmotive_sys_logging_info"); continue; } }
 /* templated Identity<I32>(x) */
 if (x->kind==TK_ID && i+5<n && streq(t[i+1].v,"<")) { if (streq(x->v,"Identity")) { sb_add(&out,"Identity_I32"); while (i<n && !streq(t[i].v,">")) i++; continue; } }
 /* New Class(args) */
 if (streq(x->v,"New") && i+2<n) {
   char *cls = t[i+1].v;
   int par = i+2;
   if (streq(t[par].v,"<")) { while (par<n && !streq(t[par].v,">")) par++; par++; }
   if (par<n && streq(t[par].v,"(")) {
     int end = matching_paren(t,n,par);
     Class *c = find_class(prog,cls);
     if (c) {
       Func *ctor = NULL;
       const char *desired = NULL;
       int k;
       if (end > par + 3 && streq(t[par+1].v,"(") && streq(t[par+3].v,")")) desired = type_suffix(t[par+2].v);
       for (k=0;k<c->method_n;k++) if (c->methods[k].ctor) {
         if ((end-par-1)==0 && c->methods[k].param_n==0) ctor=&c->methods[k];
         else if ((end-par-1)>0 && c->methods[k].param_n>0) {
           if (desired && c->methods[k].param_n>0 && streq(type_suffix(c->methods[k].params[0].type), desired)) { ctor=&c->methods[k]; break; }
           if (!ctor) ctor=&c->methods[k];
         }
       }
       if (!ctor) { static Func fake; memset(&fake,0,sizeof(fake)); fake.ctor=1; fake.param_n=0; ctor=&fake; }
       { char *sym = mangle_class_member(c,"ctor",ctor); char *newname = ctor_symbol_to_new(sym); free(sym); sb_add(&out,newname); free(newname); }
       i = par - 1; continue;
     }
   }
 }
 /* object method call */
 if (x->kind==TK_ID && i+3<n && (streq(t[i+1].v,".")||streq(t[i+1].v,"->"))) { char *var=x->v, *op=t[i+1].v, *meth=t[i+2].v; int mi=i+2, atfield=0; Var *vv = vt_find(vt,var); if (streq(t[i+3].v,"@") && i+4<n) { meth=t[i+4].v; mi=i+4; atfield=1; }
 if (mi+1<n && streq(t[mi+1].v,"(")) { int end = matching_paren(t,n,mi+1); Str args; sb_init(&args); if (end > mi+2) append_expr(&args,prog,t,mi+2,end,vt,curcls); if (vv) { if (type_is_class(prog,vv->type)) { Class *c = class_from_type(prog,vv->type); Func *mf=NULL; int k; for(k=0;c&&k<c->method_n;k++) if ((atfield && (starts_with(c->methods[k].name,"Get")||starts_with(c->methods[k].name,"Set"))) || streq(c->methods[k].name,meth) || (c->methods[k].op && streq(meth,"Operation"))) { mf=&c->methods[k]; break; } if (atfield) { Str nm; sb_init(&nm); if (starts_with(t[i+2].v,"Get")) { sb_add(&nm,"Get__"); sb_add(&nm,meth); } else { sb_add(&nm,"Set__"); sb_add(&nm,meth); } { char *sym = mangle_class_member(c,nm.s,NULL); sb_add(&out,sym); free(sym); free(nm.s); } } else { char *sym = mangle_class_member(c,meth,mf); sb_add(&out,sym); free(sym); } sb_ch(&out,'('); if (streq(op,"->") || vv->pointer) sb_add(&out,var); else { sb_ch(&out,'&'); sb_add(&out,var); } if (args.n) { sb_add(&out,", "); sb_add(&out,args.s); } sb_ch(&out,')'); free(args.s); i=end; continue; } else if (type_is_builtin_obj(vv->type)) { char *bm = map_builtin_method(vv->type,meth); if (bm) { sb_add(&out,bm); free(bm); sb_ch(&out,'('); if (streq(op,"->") || vv->pointer) sb_add(&out,var); else { sb_ch(&out,'&'); sb_add(&out,var); } if (args.n) { sb_add(&out,", "); sb_add(&out,args.s); } sb_ch(&out,')'); free(args.s); i=end; continue; } } }
 free(args.s); }
 }
 /* field access */
 if (x->kind==TK_ID && i+2<n && (streq(t[i+1].v,".")||streq(t[i+1].v,"->"))) { Var *vv = vt_find(vt,x->v); if (vv && type_is_class(prog,vv->type)) { sb_add(&out,x->v); sb_add(&out, streq(t[i+1].v,"->")||vv->pointer ? "->" : "."); sb_add(&out,t[i+2].v); i+=2; continue; } }
 /* bare field inside method */
 if (x->kind==TK_ID && curcls && !vt_find(vt,x->v)) { int k, isfield=0; for(k=0;k<curcls->field_n;k++) if (streq(curcls->fields[k].name,x->v)) { isfield=1; break; } if (isfield) { sb_add(&out,"this->"); sb_add(&out,x->v); continue; } }
 sb_add(&out,x->v); if (i+1<n && !(streq(t[i+1].v,")")||streq(t[i+1].v,"]")||streq(t[i+1].v,";")||streq(t[i+1].v,",")||streq(t[i+1].v,".")||streq(t[i+1].v,"->")||streq(x->v,"(")||streq(x->v,"[")||streq(x->v,".")||streq(x->v,"->")||streq(x->v,"!")||streq(x->v,"~"))) sb_ch(&out,' '); }
 return sb_take(&out); }

typedef struct BodyCtx { Program *prog; Class *curcls; Func *func; VarTab vars; Str *out; int indent; } BodyCtx;
static void emit_indent(Str *out, int n) { while (n-- > 0) sb_add(out,"  "); }
static int stmt_end(Tok *t, int n, int start) { int d=0,i; for(i=start;i<n;i++){ if(streq(t[i].v,"(")||streq(t[i].v,"[")||streq(t[i].v,"{")) d++; else if(streq(t[i].v,")")||streq(t[i].v,"]")||streq(t[i].v,"}")) d--; if(d==0 && streq(t[i].v,";")) return i; } return n; }
static void emit_body_range(BodyCtx *bc, Tok *t, int n);
static int block_end(Tok *t, int n, int open)
{
    int d = 0;
    int i;
    for (i = open; i < n; ++i) {
        if (streq(t[i].v, "{")) d++;
        else if (streq(t[i].v, "}")) {
            d--;
            if (d == 0) return i;
        }
    }
    return n - 1;
}
static int token_is_statement_end(Tok *t)
{
    return t->kind == TK_EOL || streq(t->v, ";");
}
static char *join_tokens(Tok *t, int a, int b)
{
    Str s;
    int i;
    sb_init(&s);
    for (i = a; i < b; ++i) {
        if (t[i].kind == TK_EOL || t[i].kind == TK_EOF) continue;
        if (s.n && !streq(t[i].v, ")") && !streq(t[i].v, "]") && !streq(t[i].v, ",") && !streq(t[i].v, ";") && !streq(t[i].v, ">") && !streq(t[i-1].v, "(") && !streq(t[i-1].v, "[") && !streq(t[i-1].v, "<") && !streq(t[i-1].v, "::") && !streq(t[i].v, "::") && !streq(t[i].v, "*") && !streq(t[i-1].v, "*")) sb_ch(&s, ' ');
        sb_add(&s, t[i].v);
    }
    return sb_take(&s);
}
static char *strip_storage_qualifiers(const char *type)
{
    const char *p = type ? type : "";
    while (*p && isspace((unsigned char)*p)) p++;
    if (starts_with(p, "Global ")) p += 7;
    while (*p && isspace((unsigned char)*p)) p++;
    if (starts_with(p, "Tstore ")) p += 7;
    else if (starts_with(p, "ThreadStore ")) p += 12;
    while (*p && isspace((unsigned char)*p)) p++;
    return xstrdup(p);
}
static int parse_type_decl_tokens(Tok *t, int n, int i, char **name, char **type, char **init, int *arrsz, int *is_global)
{
    int p = i;
    int colon;
    int type_start;
    int type_end;
    int d = 0;
    *name = NULL;
    *type = NULL;
    *init = NULL;
    *arrsz = 0;
    *is_global = 0;
    if (p >= n || t[p].kind == TK_EOL || t[p].kind == TK_EOF) return 0;
    if (streq(t[p].v, "Global")) {
        *is_global = 1;
        p++;
    }
    if (p + 1 >= n || t[p].kind != TK_ID || !streq(t[p+1].v, ":")) return 0;
    *name = xstrdup(t[p].v);
    p += 2;
    if (p < n && streq(t[p].v, "Global")) {
        *is_global = 1;
        p++;
    }
    type_start = p;
    colon = p;
    while (colon < n) {
        if (streq(t[colon].v, "<") || streq(t[colon].v, "[") || streq(t[colon].v, "(")) d++;
        else if (streq(t[colon].v, ">") || streq(t[colon].v, "]") || streq(t[colon].v, ")")) d--;
        if (d == 0 && (streq(t[colon].v, "=") || streq(t[colon].v, ";") || t[colon].kind == TK_EOL)) break;
        colon++;
    }
    type_end = colon;
    if (type_end <= type_start) {
        free(*name); *name = NULL;
        return 0;
    }
    if (type_end - type_start >= 4 && streq(t[type_end-3].v, "[") && streq(t[type_end-1].v, "]")) {
        *arrsz = atoi(t[type_end-2].v);
        type_end -= 3;
    }
    *type = join_tokens(t, type_start, type_end);
    if (colon < n && streq(t[colon].v, "=")) {
        int e = colon + 1;
        int end = e;
        d = 0;
        while (end < n) {
            if (streq(t[end].v, "(") || streq(t[end].v, "[") || streq(t[end].v, "{")) d++;
            else if (streq(t[end].v, ")") || streq(t[end].v, "]") || streq(t[end].v, "}")) d--;
            if (d == 0 && token_is_statement_end(&t[end])) break;
            end++;
        }
        *init = join_tokens(t, e, end);
    }
    return 1;
}
static int init_starts_with_new(const char *init)
{
    const char *p = init ? init : "";
    while (*p && isspace((unsigned char)*p)) p++;
    return starts_with(p, "New ") || starts_with(p, "New<") || starts_with(p, "New\t");
}
static void emit_local_decl(BodyCtx *bc, const char *name, const char *type, const char *init, int arrsz)
{
    char *ntype = strip_storage_qualifiers(type);
    char *ct = ctype(ntype);
    char *expr = NULL;
    int is_cls = type_is_class(bc->prog, ntype);
    int is_ptr = type_decl_is_pointer(ntype);
    if (init && *init) {
        TokVec tv = lex_text(init);
        expr = expr_from_tokens(bc->prog, tv.v, tv.n > 0 ? tv.n - 1 : 0, &bc->vars, bc->curcls, 0);
    }
    emit_indent(bc->out, bc->indent);
    if (arrsz > 0) {
        sb_printf(bc->out, "%s %s[%d]; memset(%s, 0, sizeof(%s));\n", ct, name, arrsz, name, name);
        vt_add(&bc->vars, name, ntype, 1);
    } else if (is_cls && init && init_starts_with_new(init)) {
        if (type_decl_is_pointer(ntype) || strchr(ct, '*')) sb_printf(bc->out, "%s %s = %s;\n", ct, name, expr && *expr ? expr : "NULL");
        else sb_printf(bc->out, "%s *%s = %s;\n", ct, name, expr && *expr ? expr : "NULL");
        vt_add(&bc->vars, name, ntype, 1);
    } else if (bc->prog->dyn_name && streq(ntype, bc->prog->dyn_name) && (!init || !*init)) {
        sb_printf(bc->out, "%s %s; memset(&%s, 0, sizeof(%s));\n", ct, name, name, name);
        vt_add(&bc->vars, name, ntype, 0);
    } else if (is_cls && (!init || !*init)) {
        Class *c = class_from_type(bc->prog, ntype);
        sb_printf(bc->out, "%s %s; memset(&%s, 0, sizeof(%s));", ct, name, name, name);
        if (c) {
            sb_printf(bc->out, " %s__%s__ctor(&%s);", c->package, c->name, name);
            sb_printf(bc->out, " CMotive_Cleanup_Push(&%s, (CMotive_CleanupFn)%s__%s__dtor);", name, c->package, c->name);
        }
        sb_add(bc->out, "\n");
        vt_add(&bc->vars, name, ntype, is_ptr);
    } else if (type_is_builtin_obj(ntype) && (!init || !*init)) {
        sb_printf(bc->out, "%s %s; memset(&%s, 0, sizeof(%s));\n", ct, name, name, name);
        vt_add(&bc->vars, name, ntype, is_ptr);
    } else {
        sb_printf(bc->out, "%s %s = %s;\n", ct, name, expr && *expr ? expr : "0");
        vt_add(&bc->vars, name, ntype, is_ptr);
    }
    free(ntype);
    free(ct);
    free(expr);
}
static int emit_io_chain_if_match(BodyCtx *bc, Tok *t, int start, int end)
{
    int p = start;
    int fmt_open, fmt_close, meth_open, meth_close;
    char *fmt = NULL;
    char *args = NULL;
    int is_cout;
    if (p + 7 >= end) return 0;
    is_cout = streq(t[p].v, "cout");
    if (!is_cout && !streq(t[p].v, "cin")) return 0;
    if (!streq(t[p+1].v, ".")) return 0;
    if (!streq(t[p+2].v, "expect") && !streq(t[p+2].v, "Expect")) return 0;
    fmt_open = p + 3;
    if (!streq(t[fmt_open].v, "(")) return 0;
    fmt_close = matching_paren(t, end, fmt_open);
    if (fmt_close + 4 >= end) return 0;
    if (!streq(t[fmt_close+1].v, ".")) return 0;
    if (is_cout) {
        if (!streq(t[fmt_close+2].v, "write") && !streq(t[fmt_close+2].v, "Write")) return 0;
    } else {
        if (!streq(t[fmt_close+2].v, "read") && !streq(t[fmt_close+2].v, "Read")) return 0;
    }
    meth_open = fmt_close + 3;
    if (!streq(t[meth_open].v, "(")) return 0;
    meth_close = matching_paren(t, end, meth_open);
    fmt = expr_from_tokens(bc->prog, t + fmt_open + 1, fmt_close - fmt_open - 1, &bc->vars, bc->curcls, 0);
    args = expr_from_tokens(bc->prog, t + meth_open + 1, meth_close - meth_open - 1, &bc->vars, bc->curcls, 0);
    emit_indent(bc->out, bc->indent);
    if (is_cout) {
        if (args && *args) sb_printf(bc->out, "printf(%s, %s);\n", fmt, args);
        else sb_printf(bc->out, "printf(%s);\n", fmt);
    } else {
        if (args && *args) sb_printf(bc->out, "scanf(%s, %s);\n", fmt, args);
        else sb_printf(bc->out, "scanf(%s);\n", fmt);
    }
    free(fmt);
    free(args);
    return 1;
}
static Func *find_class_hit(Class *c, const char *sender, const char *id)
{
    int k;
    if (!c) return NULL;
    for (k = 0; k < c->method_n; ++k) {
        Func *f = &c->methods[k];
        if (f->hit_id && streq(f->hit_id, id ? id : "") && streq(f->hit_sender ? f->hit_sender : "", sender ? sender : "")) return f;
    }
    return NULL;
}
static Func *find_global_hit(Program *p, const char *sender, const char *id)
{
    int k;
    for (k = 0; k < p->func_n; ++k) {
        Func *f = &p->funcs[k];
        if (f->hit_id && streq(f->hit_id, id ? id : "") && streq(f->hit_sender ? f->hit_sender : "", sender ? sender : "")) return f;
    }
    return NULL;
}
static void emit_target(BodyCtx *bc, Tok *t, int n, int start, int end)
{
    const char *sender = "";
    const char *obj = NULL;
    const char *id = "";
    int args_start = -1;
    int args_end = -1;
    int p = start + 1;
    int colon;
    (void)n;
    if (p >= end) return;
    if (streq(t[p].v, ":")) {
        obj = t[p+1].v;
        args_start = p + 3;
    } else if (p + 1 < end && streq(t[p+1].v, "::")) {
        sender = t[p].v;
        obj = NULL;
        args_start = p + 2;
    } else if (p + 2 < end && streq(t[p+1].v, ":")) {
        sender = t[p].v;
        obj = t[p+2].v;
        args_start = p + 4;
    }
    if (args_start < 0 || args_start > end) { emit_indent(bc->out, bc->indent); sb_add(bc->out, "/* malformed Target ignored */\n"); return; }
    colon = end - 1;
    while (colon > args_start && !streq(t[colon].v, ":")) colon--;
    if (colon > args_start && colon + 1 < end) { args_end = colon; id = t[colon+1].v; }
    else { args_end = end; id = ""; }
    emit_indent(bc->out, bc->indent);
    if (obj) {
        Var *vv = vt_find(&bc->vars, obj);
        Class *c = vv ? class_from_type(bc->prog, vv->type) : NULL;
        Func *f = find_class_hit(c, sender, id);
        if (f && c) {
            char *sym = mangle_class_member(c, f->name, f);
            char *args = expr_from_tokens(bc->prog, t + args_start, args_end - args_start, &bc->vars, bc->curcls, 0);
            sb_printf(bc->out, "%s(", sym);
            if (vv && vv->pointer) sb_add(bc->out, obj); else { sb_ch(bc->out, '&'); sb_add(bc->out, obj); }
            if (args && *args) sb_printf(bc->out, ", %s", args);
            sb_add(bc->out, ");\n");
            free(sym); free(args);
            return;
        }
    } else {
        Func *f = find_global_hit(bc->prog, sender, id);
        if (f) {
            char *sym = mangle_func(f);
            char *args = expr_from_tokens(bc->prog, t + args_start, args_end - args_start, &bc->vars, bc->curcls, 0);
            sb_printf(bc->out, "%s(%s);\n", sym, args && *args ? args : "");
            free(sym); free(args);
            return;
        }
    }
    sb_add(bc->out, "/* unresolved Target ignored */\n");
}
static void emit_try(BodyCtx *bc, Tok *t, int n, int *idx)
{
    int i = *idx;
    int try_open = i + 1;
    int try_close;
    int catch_open = -1;
    int catch_close = -1;
    int after;
    while (try_open < n && !streq(t[try_open].v, "{")) try_open++;
    try_close = block_end(t, n, try_open);
    after = try_close + 1;
    while (after < n && t[after].kind == TK_EOL) after++;
    if (after < n && (streq(t[after].v, "Catchall") || streq(t[after].v, "Catch"))) {
        catch_open = after + 1;
        while (catch_open < n && !streq(t[catch_open].v, "{")) catch_open++;
        catch_close = block_end(t, n, catch_open);
    }
    emit_indent(bc->out, bc->indent);
    sb_add(bc->out, "{ CMotive_ExceptionFrame __cmotive_frame; CMotive_CleanupEntry *__cmotive_mark = __cmotive_cleanup_stack; __cmotive_frame.prev = __cmotive_exception_stack; __cmotive_exception_stack = &__cmotive_frame; if (setjmp(__cmotive_frame.env) == 0) {\n");
    bc->indent++;
    emit_body_range(bc, t + try_open + 1, try_close - try_open - 1);
    emit_indent(bc->out, bc->indent);
    sb_add(bc->out, "CMotive_Cleanup_RunTo(__cmotive_mark);\n");
    bc->indent--;
    emit_indent(bc->out, bc->indent);
    sb_add(bc->out, "CMOTIVE_EXCEPTION_POP(&__cmotive_frame); } else { CMotive_Cleanup_RunTo(__cmotive_mark); CMOTIVE_EXCEPTION_POP(&__cmotive_frame);\n");
    if (catch_open >= 0 && catch_close > catch_open) {
        bc->indent++;
        emit_body_range(bc, t + catch_open + 1, catch_close - catch_open - 1);
        bc->indent--;
        emit_indent(bc->out, bc->indent);
        sb_add(bc->out, "} }\n");
        *idx = catch_close + 1;
    } else {
        sb_add(bc->out, "} }\n");
        *idx = try_close + 1;
    }
}
static void emit_body_range(BodyCtx *bc, Tok *t, int n)
{
    int i = 0;
    while (i < n) {
        while (i < n && t[i].kind == TK_EOL) i++;
        if (i >= n) break;

        if (bc->prog->dyn_name && i + 1 < n && streq(t[i].v, bc->prog->dyn_name) && streq(t[i+1].v, "Expand")) {
            int open = i + 2;
            int close;
            while (open < n && !streq(t[open].v, "{")) open++;
            close = block_end(t, n, open);
            i = close + 1;
            while (i < n && !streq(t[i].v, ";")) i++;
            if (i < n) i++;
            continue;
        }

        if (streq(t[i].v, "Try")) {
            emit_try(bc, t, n, &i);
            continue;
        }

        if (streq(t[i].v, "If") || streq(t[i].v, "if")) {
            int condopen = i + 1;
            while (condopen < n && !streq(t[condopen].v, "(")) condopen++;
            int condclose = matching_paren(t, n, condopen);
            int bodyopen = condclose + 1;
            while (bodyopen < n && !streq(t[bodyopen].v, "{")) bodyopen++;
            int bodyclose = block_end(t, n, bodyopen);
            char *cond = expr_from_tokens(bc->prog, t + condopen + 1, condclose - condopen - 1, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            sb_printf(bc->out, "if (%s) {\n", cond);
            free(cond);
            bc->indent++;
            emit_body_range(bc, t + bodyopen + 1, bodyclose - bodyopen - 1);
            bc->indent--;
            emit_indent(bc->out, bc->indent);
            sb_add(bc->out, "}");
            i = bodyclose + 1;
            while (i < n && t[i].kind == TK_EOL) i++;
            if (i < n && (streq(t[i].v, "Else") || streq(t[i].v, "else"))) {
                int eopen = i + 1;
                while (eopen < n && !streq(t[eopen].v, "{")) eopen++;
                int eclose = block_end(t, n, eopen);
                sb_add(bc->out, " else {\n");
                bc->indent++;
                emit_body_range(bc, t + eopen + 1, eclose - eopen - 1);
                bc->indent--;
                emit_indent(bc->out, bc->indent);
                sb_add(bc->out, "}\n");
                i = eclose + 1;
            } else {
                sb_add(bc->out, "\n");
            }
            continue;
        }

        if (streq(t[i].v, "While") || streq(t[i].v, "while")) {
            int condopen = i + 1;
            while (condopen < n && !streq(t[condopen].v, "(")) condopen++;
            int condclose = matching_paren(t, n, condopen);
            int bodyopen = condclose + 1;
            while (bodyopen < n && !streq(t[bodyopen].v, "{")) bodyopen++;
            int bodyclose = block_end(t, n, bodyopen);
            char *cond = expr_from_tokens(bc->prog, t + condopen + 1, condclose - condopen - 1, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            sb_printf(bc->out, "while (%s) {\n", cond);
            free(cond);
            bc->indent++;
            emit_body_range(bc, t + bodyopen + 1, bodyclose - bodyopen - 1);
            bc->indent--;
            emit_indent(bc->out, bc->indent);
            sb_add(bc->out, "}\n");
            i = bodyclose + 1;
            continue;
        }

        if (streq(t[i].v, "For") || streq(t[i].v, "for")) {
            int condopen = i + 1;
            while (condopen < n && !streq(t[condopen].v, "(")) condopen++;
            int condclose = matching_paren(t, n, condopen);
            int bodyopen = condclose + 1;
            while (bodyopen < n && !streq(t[bodyopen].v, "{")) bodyopen++;
            int bodyclose = block_end(t, n, bodyopen);
            char *cond = expr_from_tokens(bc->prog, t + condopen + 1, condclose - condopen - 1, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            sb_printf(bc->out, "for (%s) {\n", cond);
            free(cond);
            bc->indent++;
            emit_body_range(bc, t + bodyopen + 1, bodyclose - bodyopen - 1);
            bc->indent--;
            emit_indent(bc->out, bc->indent);
            sb_add(bc->out, "}\n");
            i = bodyclose + 1;
            continue;
        }

        int end = stmt_end(t, n, i);
        if (end <= i) { i++; continue; }

        if (emit_io_chain_if_match(bc, t, i, end)) {
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Return") || streq(t[i].v, "return")) {
            char *e = expr_from_tokens(bc->prog, t + i + 1, end - i - 1, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            if (strlen(e)) sb_printf(bc->out, "return %s;\n", e);
            else sb_add(bc->out, "return;\n");
            free(e);
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Throw")) {
            char *e = expr_from_tokens(bc->prog, t + i + 1, end - i - 1, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            sb_printf(bc->out, "CMotive_Throw(%s);\n", e);
            free(e);
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Break")) {
            emit_indent(bc->out, bc->indent);
            sb_add(bc->out, "break;\n");
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Continue")) {
            emit_indent(bc->out, bc->indent);
            sb_add(bc->out, "continue;\n");
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Delete")) {
            char *v = t[i + 1].v;
            Var *vv = vt_find(&bc->vars, v);
            emit_indent(bc->out, bc->indent);
            if (vv && type_is_class(bc->prog, vv->type)) {
                Class *c = class_from_type(bc->prog, vv->type);
                sb_printf(bc->out, "%s__%s__delete(%s);\n", c->package, c->name, v);
            } else {
                sb_printf(bc->out, "CMotive_Delete(%s);\n", v);
            }
            i = end + 1;
            continue;
        }

        if (streq(t[i].v, "Target")) {
            emit_target(bc, t, n, i, end);
            i = end + 1;
            continue;
        }

        {
            char *name = NULL, *type = NULL, *init = NULL;
            int arr = 0, glob = 0;
            if (parse_type_decl_tokens(t, n, i, &name, &type, &init, &arr, &glob)) {
                (void)glob;
                emit_local_decl(bc, name, type, init, arr);
                free(name); free(type); free(init);
                i = end + 1;
                continue;
            }
        }

        {
            char *e = expr_from_tokens(bc->prog, t + i, end - i, &bc->vars, bc->curcls, 0);
            emit_indent(bc->out, bc->indent);
            sb_printf(bc->out, "%s;\n", e);
            free(e);
            i = end + 1;
            continue;
        }
    }
}
static void emit_runtime_prelude(Str *out, const char *target_arch) { sb_add(out,"/* Generated by CMotive C frontend. */\n"); sb_printf(out,"/* target-arch: %s */\n", target_arch?target_arch:"native"); sb_add(out,"#include <stdio.h>\n#include <stdlib.h>\n#include <stdint.h>\n#include <stddef.h>\n#include <string.h>\n#include <stdarg.h>\n#include <setjmp.h>\n#include <math.h>\n#include <ctype.h>\n#include <time.h>\n#if defined(_WIN32)\n#include <windows.h>\n#else\n#include <unistd.h>\n#include <sched.h>\n#include <pthread.h>\n#endif\n#include \"runtime.c\"\n"); sb_add(out,"typedef struct CMotive_ExceptionFrame { jmp_buf env; const char *message; struct CMotive_ExceptionFrame *prev; } CMotive_ExceptionFrame;\nstatic CMotive_ExceptionFrame *__cmotive_exception_stack = NULL;\ntypedef void (*CMotive_CleanupFn)(void*);\ntypedef struct CMotive_CleanupEntry { void *object; CMotive_CleanupFn cleanup; struct CMotive_CleanupEntry *prev; } CMotive_CleanupEntry;\nstatic CMotive_CleanupEntry *__cmotive_cleanup_stack = NULL;\nstatic void CMotive_Cleanup_Push(void *object, CMotive_CleanupFn cleanup){CMotive_CleanupEntry*e=(CMotive_CleanupEntry*)malloc(sizeof(CMotive_CleanupEntry)); if(!e) exit(72); e->object=object; e->cleanup=cleanup; e->prev=__cmotive_cleanup_stack; __cmotive_cleanup_stack=e;}\nstatic void CMotive_Cleanup_RunTo(CMotive_CleanupEntry *mark){while(__cmotive_cleanup_stack!=mark){CMotive_CleanupEntry*e=__cmotive_cleanup_stack; if(!e)break; __cmotive_cleanup_stack=e->prev; if(e->cleanup&&e->object)e->cleanup(e->object); free(e);}}\nstatic void CMotive_Cleanup_DiscardTo(CMotive_CleanupEntry *mark){while(__cmotive_cleanup_stack!=mark){CMotive_CleanupEntry*e=__cmotive_cleanup_stack; if(!e)break; __cmotive_cleanup_stack=e->prev; free(e);}}\nstatic void CMotive_Throw(const char*msg){CMotive_ExceptionFrame*f=__cmotive_exception_stack; if(!f){fprintf(stderr,\"CMotive unhandled exception: %s\\n\",msg?msg:\"<null>\"); exit(70);} f->message=msg?msg:\"CMotive exception\"; longjmp(f->env,1);}\n#define CMOTIVE_EXCEPTION_POP(frameptr) do{ if(__cmotive_exception_stack==(frameptr)) __cmotive_exception_stack=(frameptr)->prev; }while(0)\nstatic void CMotive_UnresolvedTarget(const char*s,uint64_t id){fprintf(stderr,\"CMotive unresolved Target: %s %llu\\n\",s?s:\"\",(unsigned long long)id); exit(73);}\n");
 sb_add(out,"typedef struct { const char *fmt; } OStream; static void CMOStream_Expect(OStream*s,const char*f){s->fmt=f?f:\"%s\";} static int CMOStream_Write(OStream*s,...){va_list ap; int r; va_start(ap,s); r=vprintf(s&&s->fmt?s->fmt:\"%s\",ap); va_end(ap); return r;}\ntypedef struct { int unused; } Formatter; static int CMFormatter_Println(Formatter*f,const char*s){(void)f; return cmotive_sys_stdio_println(s);}\ntypedef struct { char *path; } Path; static void CMPath_Set(Path*p,char*v){p->path=v;} static char* CMPath_Get(Path*p){return p->path;} static int CMPath_Exists(Path*p){return cmotive_sys_filesystem_exists(p->path);} static int CMPath_IsFile(Path*p){return cmotive_sys_filesystem_is_file(p->path);} static int CMPath_IsDirectory(Path*p){return cmotive_sys_filesystem_is_directory(p->path);}\ntypedef struct { int unused; } Filesystem; static int CMFilesystem_Exists(Filesystem*f,char*p){(void)f;return cmotive_sys_filesystem_exists(p);} static int CMFilesystem_IsFile(Filesystem*f,char*p){(void)f;return cmotive_sys_filesystem_is_file(p);} static int CMFilesystem_IsDirectory(Filesystem*f,char*p){(void)f;return cmotive_sys_filesystem_is_directory(p);} static char* CMFilesystem_CurrentPath(Filesystem*f){(void)f;return cmotive_sys_filesystem_current_path();}\ntypedef struct { int fd; } Socket; static int CMSocket_OpenTcpIPv4(Socket*s){s->fd=cmotive_sys_net_socket_tcp_ipv4();return s->fd;} static int CMSocket_IsOpen(Socket*s){return s->fd>=0;} static void CMSocket_Close(Socket*s){if(s->fd>=0)cmotive_sys_net_socket_close(s->fd);s->fd=-1;} typedef struct { int unused; } Net; static int CMNet_TcpIPv4(Net*n){(void)n;return cmotive_sys_net_socket_tcp_ipv4();} static void CMNet_Close(Net*n,int fd){(void)n;cmotive_sys_net_socket_close(fd);}\ntypedef struct { void *handle; } Thread; typedef struct { int unused; } Threading; static void* CMThread_Current(void*x){(void)x;return cmotive_sys_thread_current();} static int CMThread_Yield(void*x){(void)x;return cmotive_sys_thread_yield();} static void CMThread_SleepMs(void*x,uint32_t ms){(void)x;cmotive_sys_thread_sleep_ms(ms);} static void CMThread_MicroSleep(void*x,uint64_t us){(void)x;cmotive_sys_thread_sleep_us(us);} static void CMThread_NanoSleep(void*x,uint64_t ns){(void)x;cmotive_sys_thread_sleep_ns(ns);}\ntypedef struct { int unused; } Algorithms; static void CMAlgorithms_QuickSort(Algorithms*a,int64_t*d,uint64_t n){(void)a;cmotive_sys_algorithms_sort_quick_i64(d,n);} static int CMAlgorithms_IsSorted(Algorithms*a,int64_t*d,uint64_t n){(void)a;return cmotive_sys_algorithms_is_sorted_i64(d,n);} static int64_t CMAlgorithms_BinarySearch(Algorithms*a,int64_t*d,uint64_t n,int64_t v){(void)a;return cmotive_sys_algorithms_binary_search_i64(d,n,v);} static void CMAlgorithms_Reverse(Algorithms*a,int64_t*d,uint64_t n){(void)a;cmotive_sys_algorithms_reverse_i64(d,n);}\ntypedef struct { void *h; } CMotive_Vector_Int; static void CMVector_ensure(CMotive_Vector_Int*v){if(!v->h)v->h=cmotive_sys_stl_vector_create();} static void CMVector_PushBack(CMotive_Vector_Int*v,int64_t x){CMVector_ensure(v);cmotive_sys_stl_vector_push_i64(v->h,x);} static uint64_t CMVector_Size(CMotive_Vector_Int*v){CMVector_ensure(v);return cmotive_sys_stl_vector_size(v->h);} static int64_t CMVector_At(CMotive_Vector_Int*v,uint64_t i){CMVector_ensure(v);return cmotive_sys_stl_vector_get_i64(v->h,i);} static void CMVector_Sort(CMotive_Vector_Int*v){CMVector_ensure(v);cmotive_sys_stl_vector_sort_i64(v->h);}\ntypedef struct { void *h; } CMotive_Map_CharPtr_Int; static void CMMap_ensure(CMotive_Map_CharPtr_Int*m){if(!m->h)m->h=cmotive_sys_stl_map_create();} static void CMMap_Put(CMotive_Map_CharPtr_Int*m,char*k,int64_t v){CMMap_ensure(m);cmotive_sys_stl_map_put_i64(m->h,k,v);} static int64_t CMMap_Get(CMotive_Map_CharPtr_Int*m,char*k,int64_t fb){CMMap_ensure(m);return cmotive_sys_stl_map_get_i64(m->h,k,fb);}\ntypedef struct { void *h; } CMotive_Tree_Int; static void CMTree_ensure(CMotive_Tree_Int*t){if(!t->h)t->h=cmotive_sys_stl_binary_search_tree_create();} static void CMTree_Insert(CMotive_Tree_Int*t,int64_t v){CMTree_ensure(t);cmotive_sys_stl_binary_search_tree_insert_i64(t->h,v);} static int CMTree_Contains(CMotive_Tree_Int*t,int64_t v){CMTree_ensure(t);return cmotive_sys_stl_binary_search_tree_contains_i64(t->h,v);}\ntypedef struct { char *value; } CString; static void CMCString_Set(CString*s,char*v){s->value=v;} static uint64_t CMCString_Length(CString*s){return cmotive_sys_string_strlen(s->value);} static int CMCString_Contains(CString*s,char*n){return cmotive_sys_string_strstr(s->value,n)!=NULL;} static int CMCString_Compare(CString*s,char*o){return cmotive_sys_string_strcmp(s->value,o);}\ntypedef struct { int unused; } Character; static int CMCharacter_IsDigit(Character*c,int ch){(void)c;return isdigit((unsigned char)ch)!=0;}\ntypedef struct { void *table; } StringParser; static void CMStringParser_Parse(StringParser*p,char*i,char*r,char*f,char e){p->table=cmotive_sys_string_str_parse(i,r,f,e);} static uint64_t CMStringParser_Rows(StringParser*p){return cmotive_sys_string_str_parse_rows(p->table);} static uint64_t CMStringParser_Cols(StringParser*p,uint64_t r){return cmotive_sys_string_str_parse_cols(p->table,r);}\ntypedef struct { uint16_t *value; } Wide16String; typedef struct { uint32_t *value; } Wide32String; static uint64_t CMWide16_Length(Wide16String*w){return cmotive_sys_wide16_len(w->value);} static uint64_t CMWide32_Length(Wide32String*w){return cmotive_sys_wide32_len(w->value);}\ntypedef struct { void *h; } Mutex; static void CMMutex_init(Mutex*m){if(!m->h)m->h=cmotive_sys_locks_mutex_create();} static void CMMutex_lock(Mutex*m){CMMutex_init(m);cmotive_sys_locks_mutex_lock(m->h);} static void CMMutex_unlock(Mutex*m){CMMutex_init(m);cmotive_sys_locks_mutex_unlock(m->h);}\nstatic int32_t Identity_I32(int32_t x){return x;}\n"); }
static void emit_structs(Program *p, Str *out) { int i,j; if (p->dyn_name) { sb_printf(out,"typedef struct %s {\n",p->dyn_name); for(j=0;j<p->dyn_n;j++){ char *ct=ctype(p->dyn_fields[j].type); sb_printf(out,"  %s %s;\n",ct,p->dyn_fields[j].name); free(ct);} sb_printf(out,"} %s;\n",p->dyn_name); }
 for(i=0;i<p->class_n;i++){ Class*c=&p->classes[i]; sb_printf(out,"typedef struct %s {\n",c->name); if(c->base&&*c->base){ Class*b=find_class(p,c->base); if(b){ for(j=0;j<b->field_n;j++){ char*ct=ctype(b->fields[j].type); sb_printf(out,"  %s %s;\n",ct,b->fields[j].name); free(ct);} } }
 for(j=0;j<c->field_n;j++){ char*ct=ctype(c->fields[j].type); sb_printf(out,"  %s %s;\n",ct,c->fields[j].name); free(ct);} sb_printf(out,"} %s;\n",c->name); sb_printf(out,"typedef struct %s__%s__PublicData {\n", c->package, c->name); for(j=0;j<c->field_n;j++) if(!c->fields[j].block){ char*ct=ctype(c->fields[j].type); sb_printf(out,"  %s %s;\n",ct,c->fields[j].name); free(ct);} sb_printf(out,"} %s__%s__PublicData;\n", c->package, c->name); } }
static void emit_prototypes(Program *p, Str *out) {
 int i,j;
 for(i=0;i<p->func_n;i++){
  Func*f=&p->funcs[i];
  if(f->fptr){
   char*rt=ctype(f->ret);
   sb_printf(out,"typedef %s (*%s)(",rt,f->name);
   free(rt);
   for(j=0;j<f->param_n;j++){ char*ct=ctype(f->params[j].type); sb_printf(out,"%s%s",j?", ":"",ct); free(ct); }
   sb_add(out,");\n");
   continue;
  }
  {
   char*rt=ctype(streq(f->name,"main")?"I32":f->ret);
   char*nm=mangle_func(f);
   sb_printf(out,"%s %s(",rt,nm);
   free(rt); free(nm);
   for(j=0;j<f->param_n;j++){ char*ct=ctype(f->params[j].type); sb_printf(out,"%s%s %s",j?", ":"",ct,f->params[j].name); free(ct); }
   if(f->param_n==0) sb_add(out,"void");
   sb_add(out,");\n");
  }
 }
 for(i=0;i<p->class_n;i++){
  Class*c=&p->classes[i];
  for(j=0;j<c->method_n;j++){
   Func*f=&c->methods[j];
   if(f->pure) continue;
   {
    char*rt=ctype(f->ret); char*sym=mangle_class_member(c,f->name,f); int k;
    sb_printf(out,"%s %s(%s *this", f->ctor||f->dtor?"void":rt, sym, c->name);
    for(k=0;k<f->param_n;k++){ char*ct=ctype(f->params[k].type); sb_printf(out,", %s %s",ct,f->params[k].name); free(ct); }
    sb_add(out,");\n"); free(rt); free(sym);
   }
  }
 }
 for(i=0;i<p->class_n;i++){
  Class*c=&p->classes[i];
  sb_printf(out,"void %s__%s__ctor(%s *this); void %s__%s__dtor(%s *this); %s *%s__%s__new(void); void %s__%s__delete(%s *this);\n",c->package,c->name,c->name,c->package,c->name,c->name,c->name,c->package,c->name,c->package,c->name,c->name);
  for(j=0;j<c->method_n;j++) if(c->methods[j].ctor && c->methods[j].param_n>0){
   Func*f=&c->methods[j]; char*ctor=mangle_class_member(c,"ctor",f); char*newn=ctor_symbol_to_new(ctor); int k;
   sb_printf(out,"%s *%s(",c->name,newn);
   for(k=0;k<f->param_n;k++){ char*ct=ctype(f->params[k].type); sb_printf(out,"%s%s %s",k?", ":"",ct,f->params[k].name); free(ct); }
   sb_add(out,");\n"); free(ctor); free(newn);
  }
  for(j=0;j<c->field_n;j++) if(!c->fields[j].block){
   char*ct=ctype(c->fields[j].type);
   sb_printf(out,"%s %s__%s__Get__%s(%s *this); void %s__%s__Set__%s(%s *this, %s value);\n",ct,c->package,c->name,c->fields[j].name,c->name,c->package,c->name,c->fields[j].name,c->name,ct);
   free(ct);
  }
  sb_printf(out,"%s__%s__PublicData %s__%s__Getall(%s *this); void %s__%s__Setall(%s *this",c->package,c->name,c->package,c->name,c->name,c->package,c->name,c->name);
  for(j=0;j<c->field_n;j++) if(!c->fields[j].block){ char*ct=ctype(c->fields[j].type); sb_printf(out,", %s %s",ct,c->fields[j].name); free(ct); }
  sb_add(out,");\n");
 }
}
static void emit_globals(Program *p, Str *out) { int i; for(i=0;i<p->global_n;i++){ char*ct=ctype(p->globals[i].type); TokVec tv=lex_text(p->globals[i].init); VarTab empty={0}; char*e=expr_from_tokens(p,tv.v,tv.n-1,&empty,NULL,0); sb_printf(out,"%s %s = %s;\n",ct,p->globals[i].name,strlen(e)?e:"0"); free(ct); free(e); } }
static void emit_function_body(Program *p, Str *out, Func *f, Class *c) { BodyCtx bc; int i; memset(&bc,0,sizeof(bc)); bc.prog=p; bc.curcls=c; bc.func=f; bc.out=out; bc.indent=1; if(c) vt_add(&bc.vars,"this",c->name,1); for(i=0;i<f->param_n;i++) vt_add(&bc.vars,f->params[i].name,f->params[i].type,type_decl_is_pointer(f->params[i].type)); emit_body_range(&bc,f->body,f->body_n); }
static void emit_functions(Program *p, Str *out) { int i,j; for(i=0;i<p->func_n;i++){ Func*f=&p->funcs[i]; if(f->fptr) continue; { char*rt=ctype(streq(f->name,"main")?"I32":f->ret); char*nm=mangle_func(f); sb_printf(out,"%s %s(",rt,nm); free(rt); free(nm); for(j=0;j<f->param_n;j++){ char*ct=ctype(f->params[j].type); sb_printf(out,"%s%s %s",j?", ":"",ct,f->params[j].name); free(ct);} if(f->param_n==0) sb_add(out,"void"); sb_add(out,") {\n"); emit_function_body(p,out,f,NULL); sb_add(out,"}\n"); } }
 for(i=0;i<p->class_n;i++){ Class*c=&p->classes[i]; int has_ctor0=0, has_dtor=0; for(j=0;j<c->method_n;j++){ Func*f=&c->methods[j]; if(f->pure) continue; if(f->ctor&&f->param_n==0) has_ctor0=1; if(f->dtor) has_dtor=1; { char*rt=ctype(f->ret); char*sym=mangle_class_member(c,f->name,f); int k; sb_printf(out,"%s %s(%s *this", f->ctor||f->dtor?"void":rt, sym, c->name); for(k=0;k<f->param_n;k++){ char*ct=ctype(f->params[k].type); sb_printf(out,", %s %s",ct,f->params[k].name); free(ct);} sb_add(out,") {\n"); emit_function_body(p,out,f,c); if(f->ctor||f->dtor) sb_add(out,"  return;\n"); sb_add(out,"}\n"); free(rt); free(sym); } }
 if(!has_ctor0) { sb_printf(out,"void %s__%s__ctor(%s *this){(void)this;}\n",c->package,c->name,c->name); } if(!has_dtor) { sb_printf(out,"void %s__%s__dtor(%s *this){(void)this;}\n",c->package,c->name,c->name); } sb_printf(out,"%s *%s__%s__new(void){%s*this=(%s*)CMotive_New(sizeof(%s)); if(!this)return NULL; %s__%s__ctor(this); return this;}\n",c->name,c->package,c->name,c->name,c->name,c->name,c->package,c->name); sb_printf(out,"void %s__%s__delete(%s *this){if(!this)return; %s__%s__dtor(this); CMotive_Delete(this);}\n",c->package,c->name,c->name,c->package,c->name); for(j=0;j<c->method_n;j++) if(c->methods[j].ctor && c->methods[j].param_n>0){ Func*f=&c->methods[j]; char*ctor=mangle_class_member(c,"ctor",f); char *newn; int k; newn = ctor_symbol_to_new(ctor); sb_printf(out,"%s *%s(",c->name,newn); for(k=0;k<f->param_n;k++){char*ct=ctype(f->params[k].type); sb_printf(out,"%s%s %s",k?", ":"",ct,f->params[k].name); free(ct);} sb_printf(out,"){%s*this=(%s*)CMotive_New(sizeof(%s)); if(!this)return NULL; %s(this",c->name,c->name,c->name,ctor); for(k=0;k<f->param_n;k++) sb_printf(out,", %s",f->params[k].name); sb_add(out,"); return this;}\n"); free(ctor); free(newn); }
 for(j=0;j<c->field_n;j++) if(!c->fields[j].block){ char*ct=ctype(c->fields[j].type); sb_printf(out,"%s %s__%s__Get__%s(%s*this){return this->%s;}\n",ct,c->package,c->name,c->fields[j].name,c->name,c->fields[j].name); sb_printf(out,"void %s__%s__Set__%s(%s*this,%s value){this->%s=value;}\n",c->package,c->name,c->fields[j].name,c->name,ct,c->fields[j].name); free(ct); }
 sb_printf(out,"%s__%s__PublicData %s__%s__Getall(%s*this){%s__%s__PublicData out; memset(&out,0,sizeof(out));",c->package,c->name,c->package,c->name,c->name,c->package,c->name); for(j=0;j<c->field_n;j++) if(!c->fields[j].block) sb_printf(out," out.%s=this->%s;",c->fields[j].name,c->fields[j].name); sb_add(out," return out;}\n"); sb_printf(out,"void %s__%s__Setall(%s*this",c->package,c->name,c->name); for(j=0;j<c->field_n;j++) if(!c->fields[j].block){char*ct=ctype(c->fields[j].type); sb_printf(out,", %s %s",ct,c->fields[j].name); free(ct);} sb_add(out,"){"); for(j=0;j<c->field_n;j++) if(!c->fields[j].block) sb_printf(out," this->%s=%s;",c->fields[j].name,c->fields[j].name); sb_add(out,"}\n"); } }
static char *generate_c(Program *p, const char *target_arch) { Str out; sb_init(&out); emit_runtime_prelude(&out,target_arch); emit_structs(p,&out); emit_prototypes(p,&out); emit_globals(p,&out); emit_functions(p,&out); return sb_take(&out); }

/* driver */
typedef struct Args { int compile_only, emit_c, keep_c, print_linker, print_toolchain, print_arch, version; int debug; char *opt, *out, *target_arch; StrVec includes, libdirs, libs, inputs, objs, defs; } Args;
static void parse_args(int argc, char **argv, Args *a) { int i; a->opt=xstrdup(""); for(i=1;i<argc;i++){ char *s=argv[i]; if(streq(s,"--version")) a->version=1; else if(streq(s,"-c")) a->compile_only=1; else if(streq(s,"--emit-c")) a->emit_c=1; else if(streq(s,"--keep-c")) a->keep_c=1; else if(streq(s,"--print-linker")) a->print_linker=1; else if(streq(s,"--print-toolchain")) a->print_toolchain=1; else if(streq(s,"--print-target-arch")) a->print_arch=1; else if(streq(s,"-g")) a->debug=1; else if(streq(s,"-g2")) a->debug=2; else if(streq(s,"-g3")) a->debug=3; else if(starts_with(s,"-O")) { free(a->opt); a->opt=xstrdup(s+1); } else if(streq(s,"-o") && i+1<argc) a->out=xstrdup(argv[++i]); else if(streq(s,"-I") && i+1<argc) sv_push(&a->includes,argv[++i]); else if(starts_with(s,"-I") && s[2]) sv_push(&a->includes,s+2); else if(streq(s,"-L") && i+1<argc) sv_push(&a->libdirs,argv[++i]); else if(starts_with(s,"-L") && s[2]) sv_push(&a->libdirs,s+2); else if(streq(s,"-l") && i+1<argc) sv_push(&a->libs,argv[++i]); else if(starts_with(s,"-l") && s[2]) sv_push(&a->libs,s+2); else if(streq(s,"-D") && i+1<argc) sv_push(&a->defs,argv[++i]); else if(starts_with(s,"-D") && s[2]) sv_push(&a->defs,s+2); else if(streq(s,"--target-arch") && i+1<argc) a->target_arch=xstrdup(argv[++i]); else if(ends_with(s,".o")||ends_with(s,".obj")) sv_push(&a->objs,s); else sv_push(&a->inputs,s); } }
static char *find_root_from_argv0(const char *argv0) { char cwd[4096]; char *dir, *base, *root; if (!argv0 || !strchr(argv0, PATH_SEP)) {
#if defined(_WIN32)
 _getcwd(cwd,sizeof(cwd));
#else
 getcwd(cwd,sizeof(cwd));
#endif
 dir=xstrdup(cwd); } else dir=path_dirname(argv0); base=path_basename(dir); if(streq(base,"bin")){ char *p=path_dirname(dir); char *b2=path_basename(p); if(streq(b2,"build")){ root=path_dirname(p); free(p); free(b2); } else { root=path_dirname(dir); free(p); free(b2); } } else if(streq(base,"tools")) root=path_dirname(dir); else root=xstrdup("."); free(dir); free(base); return root; }
static char *quote_arg(const char *s) { Str b; const char *p; sb_init(&b); sb_ch(&b,'"'); for(p=s;*p;p++){ if(*p=='"') sb_add(&b,"\\\""); else sb_ch(&b,*p); } sb_ch(&b,'"'); return sb_take(&b); }
static int run_cmdline(const char *cmd)
{
    int rc = system(cmd);
    if (rc == -1) return 127;
#if !defined(_WIN32)
    if (WIFEXITED(rc)) return WEXITSTATUS(rc);
#endif
    return rc;
}
static const char *default_temp_dir(void)
{
#if defined(_WIN32)
    const char *d = getenv("TEMP");
    if (!d || !*d) d = getenv("TMP");
    return (d && *d) ? d : ".";
#else
    return "/tmp";
#endif
}
static char *make_temp_path(const char *prefix, const char *ext)
{
    char buf[4096];
    unsigned long r = (unsigned long)time(NULL);
    const char *base = prefix && *prefix ? prefix : "cmotive_cfrontend";
    const char *dir = default_temp_dir();
#if defined(_WIN32)
    snprintf(buf, sizeof(buf), "%s%c%s_%lu_%d%s", dir, PATH_SEP, base, r, _getpid(), ext);
#else
    snprintf(buf, sizeof(buf), "%s%c%s_%lu_%d%s", dir, PATH_SEP, base, r, getpid(), ext);
#endif
    return xstrdup(buf);
}
static int write_debug_files(const char *out, Program *p, Args *a) { Str meta, syms; int i,j; char *mp, *sp; sb_init(&meta); sb_init(&syms); sb_add(&meta,"{\n  \"format\": \"CMotive debug metadata v1\",\n"); sb_printf(&meta,"  \"debug_level\": %d,\n  \"optimization\": \"%s\"\n}\n",a->debug,a->opt?a->opt:""); sb_add(&syms,"CMotive debug symbols\n"); sb_printf(&syms,"debug_level: %d\noptimization: %s\n",a->debug,a->opt?a->opt:""); for(i=0;i<p->func_n;i++) if(!p->funcs[i].fptr && !streq(p->funcs[i].name,"main")){ char*m=mangle_func(&p->funcs[i]); sb_printf(&syms,"0x%08x %s %s %s::%s(",0x1000+i,m,p->funcs[i].ret,p->funcs[i].package,p->funcs[i].name); for(j=0;j<p->funcs[i].param_n;j++) sb_printf(&syms,"%s%s: %s",j?", ":"",p->funcs[i].params[j].name,p->funcs[i].params[j].type); sb_add(&syms,")\n"); free(m); }
 for(i=0;i<p->class_n;i++){ Class*c=&p->classes[i]; for(j=0;j<c->method_n;j++){ Func*f=&c->methods[j]; if(f->ctor||f->dtor||f->pure) continue; { char*m=mangle_class_member(c,f->name,f); int k; sb_printf(&syms,"0x%08x %s %s %s::%s::%s(",0x2000+i*32+j,m,f->ret,c->package,c->name,f->name); for(k=0;k<f->param_n;k++) sb_printf(&syms,"%s%s: %s",k?", ":"",f->params[k].name,f->params[k].type); sb_add(&syms,")\n"); free(m); } } }
 mp=(char*)xmalloc(strlen(out)+32); sprintf(mp,"%s.cmotive.debug.json",out); sp=(char*)xmalloc(strlen(out)+32); sprintf(sp,"%s_cmot_debugsymbols.syms",out); write_file(mp,meta.s); write_file(sp,syms.s); free(mp); free(sp); free(meta.s); free(syms.s); return 0; }
static int cmotive_main(int argc, char **argv) { Args a; PPContext pp; Program prog; Parser ps; TokVec tv; char *root, *pptext, *csrc, *out, *tmpc, *cc, *qtmp, *qout, *cmd; int i, rc; Str combined; memset(&a,0,sizeof(a)); parse_args(argc,argv,&a); if(a.version){ printf("CMotive compiler %s\n",CMOTIVE_VERSION); return 0; } cc = getenv("CMOTIVE_CC") ? xstrdup(getenv("CMOTIVE_CC")) : xstrdup("cc"); if(a.print_linker){ puts(getenv("CMOTIVE_LD")?getenv("CMOTIVE_LD"):cc); return 0; } if(a.print_toolchain){ printf("cc=%s\nld=%s\n",cc,getenv("CMOTIVE_LD")?getenv("CMOTIVE_LD"):cc); return 0; } if(a.print_arch){ puts(a.target_arch?a.target_arch:"native"); return 0; } if(a.inputs.n==0 && a.objs.n==0){ fprintf(stderr,"cmotive: no input files\n"); return 2; } root=find_root_from_argv0(argv[0]); memset(&pp,0,sizeof(pp)); pp.root=root; { char *lib=path_join(root,"lib"); sv_push(&pp.include_dirs,lib); free(lib); } for(i=0;i<a.includes.n;i++) sv_push(&pp.include_dirs,a.includes.v[i]); for(i=0;i<a.defs.n;i++){ char *eq=strchr(a.defs.v[i],'='); if(eq){ *eq=0; pp_set_macro(&pp,a.defs.v[i],eq+1); *eq='='; } else pp_set_macro(&pp,a.defs.v[i],"1"); }
 sb_init(&combined); for(i=0;i<a.inputs.n;i++){ pptext=pp_process_file(&pp,a.inputs.v[i]); if(!pptext)return 1; sb_add(&combined,pptext); sb_ch(&combined,'\n'); free(pptext); }
 memset(&prog,0,sizeof(prog)); tv=lex_text(combined.s); memset(&ps,0,sizeof(ps)); ps.t=tv.v; ps.n=tv.n; ps.prog=&prog; ps.package=xstrdup("StartPackage"); parse_program(&ps); scan_all_dynamic_expands(&prog); for(i=0;i<prog.class_n;i++){ if(prog.classes[i].base && *prog.classes[i].base && !find_class(&prog,prog.classes[i].base)){ fprintf(stderr,"cmotive: undefined base class %s for class %s\n",prog.classes[i].base,prog.classes[i].name); return 1; } }
 csrc=generate_c(&prog,a.target_arch?a.target_arch:"native"); if(a.emit_c){ out=a.out?a.out:xstrdup("a.c"); rc=write_file(out,csrc); if(a.debug && a.out) write_debug_files(a.out,&prog,&a); return rc; } out=a.out?a.out:xstrdup(a.compile_only?(a.inputs.n?"a.o":"a.obj"):"a.out"); tmpc=make_temp_path("cmotive_cfrontend",".c"); write_file(tmpc,csrc); if(a.keep_c){ char *kc=(char*)xmalloc(strlen(out)+4); sprintf(kc,"%s.c",out); write_file(kc,csrc); free(kc); } qtmp=quote_arg(tmpc); qout=quote_arg(out); { Str cmdb; sb_init(&cmdb); sb_add(&cmdb,cc); sb_add(&cmdb," -I"); { char *inc=path_join(root,"lib/Sys"); char *qi=quote_arg(inc); sb_add(&cmdb,qi); free(qi); free(inc); } if(a.debug) sb_printf(&cmdb," -g%d",a.debug); if(a.opt&&*a.opt) { sb_ch(&cmdb,' '); sb_ch(&cmdb,'-'); sb_add(&cmdb,a.opt); } if(a.compile_only) sb_add(&cmdb," -c "); else sb_add(&cmdb," "); sb_add(&cmdb,qtmp); if(!a.compile_only){ for(i=0;i<a.objs.n;i++){ char*q=quote_arg(a.objs.v[i]); sb_ch(&cmdb,' '); sb_add(&cmdb,q); free(q); } sb_add(&cmdb," -pthread -lm"); } sb_add(&cmdb," -o "); sb_add(&cmdb,qout); cmd=sb_take(&cmdb); }
 rc=run_cmdline(cmd); if(rc==0 && a.debug && !a.compile_only) write_debug_files(out,&prog,&a); free(cmd); free(qtmp); free(qout); remove(tmpc); free(tmpc); return rc; }
static int cmotivepp_main(int argc, char **argv) { PPContext pp; char *root, *out=NULL, *input=NULL, *text; int i; memset(&pp,0,sizeof(pp)); root=find_root_from_argv0(argv[0]); pp.root=root; { char *lib=path_join(root,"lib"); sv_push(&pp.include_dirs,lib); free(lib); } for(i=1;i<argc;i++){ if(streq(argv[i],"-I")&&i+1<argc) sv_push(&pp.include_dirs,argv[++i]); else if(starts_with(argv[i],"-I")) sv_push(&pp.include_dirs,argv[i]+2); else if(streq(argv[i],"-D")&&i+1<argc){ char *d=argv[++i], *eq=strchr(d,'='); if(eq){*eq=0; pp_set_macro(&pp,d,eq+1); *eq='=';} else pp_set_macro(&pp,d,"1"); } else if(starts_with(argv[i],"-D")){ char *d=argv[i]+2, *eq=strchr(d,'='); if(eq){*eq=0; pp_set_macro(&pp,d,eq+1); *eq='=';} else pp_set_macro(&pp,d,"1"); } else if(streq(argv[i],"-o")&&i+1<argc) out=argv[++i]; else input=argv[i]; } if(!input){fprintf(stderr,"cmotivepp: no input\n"); return 2;} text=pp_process_file(&pp,input); if(!text)return 1; if(out) return write_file(out,text); fputs(text,stdout); return 0; }
static int syms_main(int argc, char **argv) { const char *out=NULL, *bin=NULL; int i; for(i=1;i<argc;i++){ if(streq(argv[i],"-o")&&i+1<argc) out=argv[++i]; else if(argv[i][0]!='-') bin=argv[i]; else if(streq(argv[i],"--metadata")&&i+1<argc) i++; } if(!out) out="cmotive_debugsymbols.syms"; { Str s; sb_init(&s); sb_add(&s,"CMotive debug symbols\n"); sb_printf(&s,"binary: %s\n",bin?bin:""); sb_add(&s,"debug_level: 0\noptimization: \n0x00000000 external-symbol-placeholder\n"); return write_file(out,s.s); } }
int main(int argc, char **argv) { char *base=path_basename(argv[0]); int rc; if(strstr(base,"cmotivepp")) rc=cmotivepp_main(argc,argv); else if(strstr(base,"CMotiveSymsToDebugFile")) rc=syms_main(argc,argv); else rc=cmotive_main(argc,argv); free(base); return rc; }
