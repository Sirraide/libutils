#ifndef SCRIPT_REPL_H
#define SCRIPT_REPL_H

#include "./utils.h"

#include <functional>
struct Repl {
    U64                                      cursor{};
    std::string                              line;
    std::string                              prompt;
    U64                                      line_size{};
    U64                                      prompt_width_in_chars{};
    std::vector<std::pair<std::string, U64>> history{};
    U64                                      hist_index{};

    Repl(std::string prompt);

    virtual bool HandleCTRLC();
    virtual bool HandleCTRLD();

    void        ReadLine();
    void        WriteChar(const char* c, U64 len);
    static void WriteKeycode(const char* c, U64 n_read);
    void        MoveLeft();
    void        MoveRight();
    void        RedrawLine(bool save_excursion = true);
    static void EraseLine();
    static void MoveToStartOfLine();
    static void RawMoveTo(U64 pos);
    void        LogicalMoveTo(U64 pos);
    void        EraseCharAt(U64 pos);
    void        NewLine();
    static void Write(const std::string& str);
    void        ClearLinebuf();
};

#endif // SCRIPT_REPL_H
