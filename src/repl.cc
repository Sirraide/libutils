#include "../include/repl.h"

#include <clocale>
#include <functional>
#include <termios.h>
#include <unistd.h>

termios state{};

enum struct XtermCSI : char {
    UArrow = 'A',
    DArrow = 'B',
    RArrow = 'C',
    LArrow = 'D',
    Home   = 'H',
    End    = 'F',
};

enum struct VTSequence : char {
    Home = '1',
    Del  = '3',
    End  = '8',
};

enum struct Key : char {
    Del = 127,
};

void ResetTerminal() {
    tcsetattr(STDIN_FILENO, 0, &state);
}

Repl::Repl(std::string _prompt) : prompt(std::move(_prompt)) {
    prompt_width_in_chars = _prompt.size();
    setlocale(LC_ALL, "");
    termios t{};
    tcgetattr(STDIN_FILENO, &t);
    state = t;
    cfmakeraw(&t);
    t.c_iflag |= ONLCR;
    tcsetattr(STDIN_FILENO, 0, &t);
    atexit(ResetTerminal);
}

void Repl::Write(const std::string& str) {
    write(STDOUT_FILENO, str.data(), str.size());
}

void Repl::EraseLine() {
    Write("\033[2K");
}

void Repl::MoveLeft() {
    cursor--;
    Write("\033[D");
}

void Repl::MoveRight() {
    cursor++;
    Write("\033[C");
}

void Repl::MoveToStartOfLine() {
    Write("\033[G");
}

void Repl::RawMoveTo(U64 pos) {
    Write("\033[" + std::to_string(pos) + "G");
}

void Repl::LogicalMoveTo(U64 pos) {
    RawMoveTo(pos + 1 + prompt_width_in_chars);
    cursor = pos;
}

void Repl::EraseCharAt(U64 pos) {
    line.erase(pos, 1);
    line_size--;
}

void Repl::ClearLinebuf() {
    line      = "";
    line_size = 0;
}

void Repl::RedrawLine(bool save_excursion) {
    EraseLine();
    MoveToStartOfLine();
    Write(prompt);
    Write(line);
    if (!save_excursion) {
        cursor = line_size;
    }
    LogicalMoveTo(cursor);
}

void Repl::NewLine() {
    if (!line.empty() && (history.empty() || history.back().first != line))
        history.emplace_back(line, line_size);
    hist_index = 0;
    Write("\r\n");
    line_size = 0;
    cursor    = 0;
    LogicalMoveTo(0);
}

void Repl::WriteChar(const char* c, U64 len) {
    line_size++;
    line.insert(cursor, c, len);
    cursor++;
    if (cursor < line_size) RedrawLine();
    else write(STDOUT_FILENO, c, len);
}

void Repl::ReadLine() {
    char c[256];

    MoveToStartOfLine();
    line.clear();
    line_size = 0;
    cursor    = 0;
    EraseLine();
    Write(prompt);

    for (;;) {
        U64 n_read = U64(read(STDIN_FILENO, &c, sizeof c));

        if (n_read > 2 && *c == '\033' && c[1] == '[') {
            switch (c[2]) {
                case char(XtermCSI::LArrow):
                    if (cursor < 1) break;
                    MoveLeft();
                    break;
                case char(XtermCSI::RArrow):
                    if (cursor >= line_size) break;
                    MoveRight();
                    break;
                case char(XtermCSI::UArrow): {
                    if (hist_index == history.size()) break;
                    auto& [txt, sz] = history[history.size() - ++hist_index];
                    line            = txt;
                    line_size       = sz;
                    RedrawLine(false);
                    break;
                }
                case char(XtermCSI::DArrow): {
                    if (hist_index == 0 || --hist_index == 0) {
                        ClearLinebuf();
                        RedrawLine(false);
                        break;
                    }
                    auto& [txt, sz] = history[history.size() - hist_index];
                    line            = txt;
                    line_size       = sz;
                    RedrawLine(false);
                    break;
                }
                case char(XtermCSI::Home):
                home:
                    LogicalMoveTo(0);
                    break;
                case char(XtermCSI::End):
                end:
                    LogicalMoveTo(line_size);
                    break;
                case char(VTSequence::Home)... char(VTSequence::End):
                    if (n_read > 3 && c[3] == '~') {
                        switch (VTSequence(c[2])) {
                            case VTSequence::Home: goto home;
                            case VTSequence::End: goto end;
                            case VTSequence::Del:
                                if (cursor >= line_size) break;
                                EraseCharAt(cursor);
                                RedrawLine();
                                break;
                            default: goto _default;
                        }
                    } else goto _default;
                    break;
                default:
                _default:
                    WriteKeycode(c, n_read);
            }
            continue;
        }

        switch (*c) {
            case char(Key::Del):
                if (cursor == 0) break;
                MoveLeft();
                EraseCharAt(cursor);
                RedrawLine();
                break;
            case CTRL('C'):
                if (HandleCTRLC()) return;
                break;
            case CTRL('D'):
                if (HandleCTRLD()) return;
                break;
            case CTRL('Q'): {
                Write("\033[50G\033[K\"");
                Write(line);
                Write("\" : " + std::to_string(line_size) + " : " + std::to_string(cursor) + " : " + std::to_string(hist_index));
                LogicalMoveTo(cursor);
                break;
            }
            case '\r':
            case '\n':
                NewLine();
                return;
            default:
                WriteChar(c, n_read);
        }
    }
}

void Repl::WriteKeycode(const char* c, U64 n_read) {
    std::string char_code;
    for (U64 i = 0; i < n_read; i++) {
        char_code += std::to_string(int(c[i]));
        char_code += "|";
    }
    Write(char_code);
}

bool Repl::HandleCTRLD() {
    Write("\r\n");
    exit(130);
    __builtin_unreachable();
}

bool Repl::HandleCTRLC() {
    NewLine();
    MoveToStartOfLine();
    Write(prompt);
    line.clear();
    line_size = 0;
    cursor    = 0;
    LogicalMoveTo(cursor);
    return false;
}
