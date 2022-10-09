// ANSI ESCAPE CODES: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
internal void color_text(char *text, u8 r, u8 g, u8 b) {
    printf("\033[38;2;%d;%d;%dm%s\033[0m", r, g, b, text);
}

#define BOLD(text_print) printf("\033[1m"); text_print; printf("\033[22m")
#define ITALIC(text_print) printf("\033[3m"); text_print; printf("\033[23m")
#define UNDERLINE(text_print) printf("\033[4m"); text_print; printf("\033[24m")

#define _RED(text) color_text(text, 255, 0, 0)
#define _YELLOW(text) color_text(text, 255, 255, 0)
#define _DEBUG(text) ITALIC(UNDERLINE(BOLD(color_text(text, 255, 185, 0))))
#define _ERROR(text) {BOLD(RED("ERROR: ")); printf("%s\n", text);}
#define ASSERT(text, file, line) {BOLD(_RED("\nASSERT FAILED: ")); printf("%s[%d]: %s\n", file, line, text);}

#if DEVELOPMENT
    #define assert(expression, message) if (!(expression)) {ASSERT(message, __FILE__, __LINE__);(*(u8 *)0) = 0;}

    #define invalid_code_path assert(false, "INVALID CODE PATH")
    #define invalid_code_path_msg(message) assert(false, message)

    #define invalid_default_case default: {invalid_code_path;} break
    #define invalid_default_case_msg(message) default: {invalid_code_path_msg(message);} break

    #define not_implemented assert(false, "NOT IMPLEMENTED")
#else
    #define assert(expression, message)

    #define invalid_code_path
    #define invalid_code_path_msg(...)

    #define invalid_default_case
    #define invalid_default_case_msg(...)

    #define not_implemented
#endif


void printf(char *string) { printf("%s", string); }
void printf(f32 number)   { printf("%.2f", number); }
void printf(f64 number)   { printf("%.2f", number); }
void printf(u8 number)    { printf("%u", number); }
void printf(u16 number)   { printf("%u", number); }
void printf(u32 number)   { printf("%u", number); }
void printf(u64 number)   { printf("%llu", number); }
void printf(s8 number)    { printf("%d", number); }
void printf(s16 number)   { printf("%d", number); }
void printf(s32 number)   { printf("%d", number); }
void printf(s64 number)   { printf("%lld", number); }
void printf(bool value)   { printf("%s", value ? "true" : "false"); }

#define debug(var) {DEBUG(#var); putchar(':'); putchar(' '); printf(var); putchar('\n');}
