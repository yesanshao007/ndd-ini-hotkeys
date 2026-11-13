#include "section_move.hpp"
#include <cassert>
#include <string>

static std::string move(const std::string& text, std::size_t caret, int direction) {
    ini_move::Plan plan;
    std::string error;
    assert(ini_move::plan_move(text, caret, direction, plan, error));
    return text.substr(0, plan.first) + plan.replacement + text.substr(plan.last);
}

int main() {
    const std::string crlf = "[A]\r\nx=1\r\n[B]\r\ny=2\r\n";
    const std::string exchanged = "[B]\r\ny=2\r\n[A]\r\nx=1\r\n";
    assert(move(crlf, crlf.find("[B]"), -1) == exchanged);
    assert(move(crlf, crlf.find("[A]"), 1) == exchanged);
    assert(move("[A]\r\nx=1\r\n[B]\r\ny=2", 1, 1) ==
           "[B]\r\ny=2\r\n[A]\r\nx=1");
    assert(move("\xEF\xBB\xBF[A]\nx=1\n[B]\ny=2\n", 4, 1) ==
           "\xEF\xBB\xBF[B]\ny=2\n[A]\nx=1\n");
    assert(move("; pre\n[A]\n; note\nx=1\n[B] # c\ny=2\n", 12, 1) ==
           "; pre\n[B] # c\ny=2\n[A]\n; note\nx=1\n");
    assert(move("[中文]\rkey=值\r[B]\rkey=2\r", 1, 1) ==
           "[B]\rkey=2\r[中文]\rkey=值\r");

    ini_move::Plan plan;
    std::string error;
    assert(ini_move::plan_move(crlf, crlf.find("y=2") + 1, -1, plan, error));
    assert(plan.caret == exchanged.find("y=2") + 1);
    assert(ini_move::plan_move(crlf, crlf.find("x=1") + 1, 1, plan, error));
    assert(plan.caret == exchanged.find("x=1") + 1);
    const std::string four = "[A]\r\nx=1\r\n[B]\r\ny=22\r\n[C]\r\nz=333\r\n[D]\r\nw=4444";
    std::string reordered = four;
    for (int step = 0; step < 3; ++step) {
        assert(ini_move::plan_move(reordered, reordered.find("[D]"), -1, plan, error));
        assert(plan.replacement.size() == plan.last - plan.first);
        const std::string next = reordered.substr(0, plan.first) +
                                 plan.replacement + reordered.substr(plan.last);
        for (const auto old_header : ini_move::section_starts(reordered)) {
            const auto new_header = ini_move::remap_header(plan, old_header);
            assert(reordered.substr(old_header, 3) == next.substr(new_header, 3));
        }
        reordered = next;
    }
    assert(reordered.find("[D]") == 0);
    assert(!ini_move::plan_move(crlf, 1, -1, plan, error));
    assert(!ini_move::plan_move(crlf, crlf.find("[B]"), 1, plan, error));
    const std::string binary("[A]\n\0[B]\n", 9);
    assert(!ini_move::plan_move(binary, 1, 1, plan, error));
}
