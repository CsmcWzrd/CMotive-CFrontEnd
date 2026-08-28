// CMotive exception unwinding runtime contract.
//
// The generated C currently embeds the setjmp/longjmp frame push so setjmp is
// evaluated in the caller's stack frame.  This native-side file mirrors the
// public ABI shape for the future C++ front-end/runtime.
#include <csetjmp>
#include <cstdio>
#include <cstdlib>

namespace cmotive::exceptions {
struct ExceptionFrame {
    std::jmp_buf env;
    const char *message = nullptr;
    ExceptionFrame *prev = nullptr;
};

thread_local ExceptionFrame *current_frame = nullptr;

void push(ExceptionFrame *frame) {
    frame->message = nullptr;
    frame->prev = current_frame;
    current_frame = frame;
}

void pop(ExceptionFrame *frame) {
    if (current_frame == frame) current_frame = frame->prev;
}

[[noreturn]] void throw_text(const char *message) {
    ExceptionFrame *frame = current_frame;
    if (!frame) {
        std::fprintf(stderr, "CMotive unhandled exception: %s\n", message ? message : "<null>");
        std::exit(70);
    }
    frame->message = message ? message : "CMotive exception";
    longjmp(frame->env, 1);
}
} // namespace cmotive::exceptions
