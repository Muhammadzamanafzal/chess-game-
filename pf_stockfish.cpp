#include "pf_stockfish.h"
#include <cstdio>
#include <cstring>
int bestmove_parser(const char* mv, int& sri, int& sci, int& dri, int& dci, char& PawnPromoteTo) {
    if (!mv) {
        return 0;
    }

    if (strcmp(mv, "(none)") == 0) return 0;
    size_t L = strlen(mv);

    if (L != 4 && L != 5) return 0;
    char f1 = mv[0], r1 = mv[1];
    char f2 = mv[2], r2 = mv[3];

    if (f1 < 'a' || f1 > 'h') return 0;
    if (f2 < 'a' || f2 > 'h') return 0;
    if (r1 < '1' || r1 > '8') return 0;
    if (r2 < '1' || r2 > '8') return 0;

    int n1 = r1 - '0';
    int n2 = r2 - '0';

    sri = 8 - n1;
    sci = f1 - 'a';
    dri = 8 - n2;
    dci = f2 - 'a';

    if (L == 5) {
        char p = mv[4];

        if (p != 'q' && p != 'r' && p != 'b' && p != 'n') return 0;
        PawnPromoteTo = p;
    }
    else {
        PawnPromoteTo = '-';
    }

    return 1;
}


#ifdef _WIN32
#include <windows.h>
static HANDLE g_hChildStd_IN_Wr = NULL;
static HANDLE g_hChildStd_OUT_Rd = NULL;
static PROCESS_INFORMATION g_pi;
static int g_started = 0;


static int eng_send(const char* s) {
    if (!g_started) return 0;
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s\n", s);
    DWORD written = 0;
    return WriteFile(g_hChildStd_IN_Wr, buf, (DWORD)strlen(buf), &written, NULL) != 0;
}


static int eng_readline(char* out, int out_sz) {
    static char internal[8192];
    static int len = 0;

    while (1) {

        for (int i = 0; i < len; i++) {
            if (internal[i] == '\n') {
                int take = i + 1;
                if (take >= out_sz) take = out_sz - 1;
                memcpy(out, internal, take);
                out[take] = '\0';

                memmove(internal, internal + (i + 1), len - (i + 1));
                len -= (i + 1);
                return 1;
            }
        }


        DWORD avail = 0;
        if (!PeekNamedPipe(g_hChildStd_OUT_Rd, NULL, 0, NULL, &avail, NULL)) return 0;


        char tmp[512];
        DWORD readBytes = 0;
        BOOL ok = ReadFile(g_hChildStd_OUT_Rd, tmp, sizeof(tmp) - 1, &readBytes, NULL);
        if (!ok || readBytes == 0) return 0;

        if (len + (int)readBytes >= (int)sizeof(internal)) {

            len = 0;
        }
        memcpy(internal + len, tmp, readBytes);
        len += (int)readBytes;
    }
}

static int eng_wait_for(const char* token) {
    char line[512];
    while (eng_readline(line, (int)sizeof(line))) {


        if (strstr(line, token)) return 1;
    }
    return 0;
}

int stockfish_init() {
    if (g_started) return 1;

    ZeroMemory(&g_pi, sizeof(g_pi));

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE childStdOutRd = NULL, childStdOutWr = NULL;
    HANDLE childStdInRd = NULL, childStdInWr = NULL;

    if (!CreatePipe(&childStdOutRd, &childStdOutWr, &sa, 0)) return 0;
    if (!SetHandleInformation(childStdOutRd, HANDLE_FLAG_INHERIT, 0)) return 0;

    if (!CreatePipe(&childStdInRd, &childStdInWr, &sa, 0)) return 0;
    if (!SetHandleInformation(childStdInWr, HANDLE_FLAG_INHERIT, 0)) return 0;

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = childStdOutWr;
    si.hStdOutput = childStdOutWr;
    si.hStdInput = childStdInRd;
    si.dwFlags |= STARTF_USESTDHANDLES;


    char cmd[] = "stockfish.exe";

    BOOL ok = CreateProcessA(
        NULL, cmd,
        NULL, NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL, NULL,
        &si, &g_pi
    );


    CloseHandle(childStdOutWr);
    CloseHandle(childStdInRd);

    if (!ok) {
        CloseHandle(childStdOutRd);
        CloseHandle(childStdInWr);
        return 0;
    }

    g_hChildStd_OUT_Rd = childStdOutRd;
    g_hChildStd_IN_Wr = childStdInWr;
    g_started = 1;


    eng_send("uci");
    if (!eng_wait_for("uciok")) return 0;

    eng_send("isready");
    if (!eng_wait_for("readyok")) return 0;

    return 1;
}

void stockfish_quit() {
    if (!g_started) return;

    eng_send("quit");


    CloseHandle(g_hChildStd_IN_Wr);
    CloseHandle(g_hChildStd_OUT_Rd);


    WaitForSingleObject(g_pi.hProcess, 2000);
    CloseHandle(g_pi.hProcess);
    CloseHandle(g_pi.hThread);

    g_hChildStd_IN_Wr = NULL;

    g_hChildStd_OUT_Rd = NULL;
    g_started = 0;
}


int stockfish_bestmove(const char* fen, int movetime_ms, char* out_move) {
    if (!g_started || !fen || !out_move) return 0;
    out_move[0] = '\0';

    char cmd[2048];

    snprintf(cmd, sizeof(cmd), "position fen %s", fen);
    if (!eng_send(cmd)) return 0;

    snprintf(cmd, sizeof(cmd), "go movetime %d", movetime_ms);
    if (!eng_send(cmd)) return 0;


    char line[512];
    while (eng_readline(line, (int)sizeof(line))) {

        if (strncmp(line, "bestmove ", 9) == 0) {
            char mv[32] = { 0 };
            if (sscanf_s(line + 9, "%31s", mv, (unsigned)_countof(mv)) == 1) {
                strcpy_s(out_move,32, mv);
                return 1;
            }
            return 0;
        }
    }
    return 0;
}

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>

static FILE* g_in = NULL;
static FILE* g_out = NULL;
static pid_t g_pid = -1;

static int eng_send(const char* s) {
    if (!g_in) return 0;
    fprintf(g_in, "%s\n", s);
    fflush(g_in);
    return 1;
}

static int eng_readline(char* out, int out_sz) {
    if (!g_out) return 0;
    return fgets(out, out_sz, g_out) != NULL;
}

static int eng_wait_for(const char* token) {
    char line[512];
    while (eng_readline(line, (int)sizeof(line))) {

        if (strstr(line, token)) return 1;
    }
    return 0;
}

int stockfish_init() {
    if (g_in || g_out) return 1;

    int to_engine[2];
    int from_engine[2];

    if (pipe(to_engine) != 0) return 0;
    if (pipe(from_engine) != 0) return 0;

    g_pid = fork();
    if (g_pid < 0) return 0;

    if (g_pid == 0) {
        dup2(to_engine[0], STDIN_FILENO);
        dup2(from_engine[1], STDOUT_FILENO);
        dup2(from_engine[1], STDERR_FILENO);

        close(to_engine[0]); close(to_engine[1]);
        close(from_engine[0]); close(from_engine[1]);


        execl("./stockfish", "./stockfish", (char*)NULL);
        _exit(1);
    }

    close(to_engine[0]);
    close(from_engine[1]);

    g_in = fdopen(to_engine[1], "w");
    g_out = fdopen(from_engine[0], "r");
    if (!g_in || !g_out) return 0;

    setvbuf(g_in, NULL, _IONBF, 0);

    eng_send("uci");
    if (!eng_wait_for("uciok")) return 0;

    eng_send("isready");
    if (!eng_wait_for("readyok")) return 0;

    return 1;
}

void stockfish_quit() {
    if (!g_in || !g_out) return;

    eng_send("quit");
    fclose(g_in);
    fclose(g_out);
    g_in = NULL;
    g_out = NULL;

    if (g_pid > 0) {
        int status = 0;
        waitpid(g_pid, &status, 0);
        g_pid = -1;
    }
}

int stockfish_bestmove(const char* fen, int movetime_ms, char* out_move) {
    if (!g_in || !g_out || !fen || !out_move) return 0;
    out_move[0] = '\0';

    char cmd[2048];

    snprintf(cmd, sizeof(cmd), "position fen %s", fen);
    if (!eng_send(cmd)) return 0;

    snprintf(cmd, sizeof(cmd), "go movetime %d", movetime_ms);
    if (!eng_send(cmd)) return 0;

    char line[512];
    while (eng_readline(line, (int)sizeof(line))) {
        if (strncmp(line, "bestmove ", 9) == 0) {
            char mv[32] = { 0 };
            if (sscanf(line + 9, "%31s", mv) == 1) {
                strcpy(out_move, mv);
                return 1;
            }
            return 0;
        }
    }
    return 0;
}
#endif