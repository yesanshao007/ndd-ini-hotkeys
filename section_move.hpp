#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace ini_move {

struct Plan {
    std::size_t first = 0;
    std::size_t last = 0;
    std::string replacement;
    std::size_t caret = 0;
    // Original header byte offset -> header byte offset after the swap.
    std::vector<std::pair<std::size_t, std::size_t>> header_moves;
};

inline std::size_t remap_header(const Plan& plan, std::size_t old_offset) {
    for (const auto& moved : plan.header_moves) {
        if (moved.first == old_offset) return moved.second;
    }
    return old_offset;
}

inline bool is_header(const std::string& text, std::size_t first, std::size_t last) {
    std::size_t p = first;
    while (p < last && (text[p] == ' ' || text[p] == '\t')) ++p;
    if (p == last || text[p++] != '[') return false;
    const auto name_start = p;
    while (p < last && text[p] != ']') ++p;
    if (p == last || p == name_start) return false;
    ++p;
    while (p < last && (text[p] == ' ' || text[p] == '\t')) ++p;
    return p == last || text[p] == ';' || text[p] == '#';
}

inline std::vector<std::size_t> section_starts(const std::string& text) {
    std::vector<std::size_t> starts;
    std::size_t line_start = 0;
    while (line_start < text.size()) {
        auto line_end = text.find_first_of("\r\n", line_start);
        if (line_end == std::string::npos) line_end = text.size();
        auto header_start = line_start;
        if (line_start == 0 && text.size() >= 3 &&
            text.compare(0, 3, "\xEF\xBB\xBF", 3) == 0) {
            header_start = 3; // A UTF-8 BOM must remain at the file start.
        }
        if (is_header(text, header_start, line_end)) starts.push_back(header_start);
        if (line_end == text.size()) break;
        line_start = line_end + (text[line_end] == '\r' &&
                line_end + 1 < text.size() && text[line_end + 1] == '\n' ? 2 : 1);
    }
    return starts;
}

// Returns one adjacent swap in UTF-8 byte offsets. It never reparses or rewrites
// Key/Value lines, so comments, spacing, and mixed line endings remain intact.
inline bool plan_move(const std::string& text, std::size_t caret,
                      int direction, Plan& plan, std::string& error) {
    if (caret > text.size() || (direction != -1 && direction != 1)) {
        error = "Invalid caret or direction";
        return false;
    }
    if (text.find('\0') != std::string::npos) {
        error = "Binary content is not supported";
        return false;
    }
    const auto starts = section_starts(text);
    const auto upper = std::upper_bound(starts.begin(), starts.end(), caret);
    if (upper == starts.begin()) {
        error = "Caret is outside a Section";
        return false;
    }
    const auto current = static_cast<std::size_t>(upper - starts.begin() - 1);
    if (direction < 0 && current == 0) {
        error = "Already the first Section";
        return false;
    }
    if (direction > 0 && current + 1 == starts.size()) {
        error = "Already the last Section";
        return false;
    }

    const auto left_index = direction < 0 ? current - 1 : current;
    const auto first = starts[left_index];
    const auto middle = starts[left_index + 1];
    const auto last = left_index + 2 < starts.size() ? starts[left_index + 2] : text.size();
    std::string left = text.substr(first, middle - first);
    std::string right = text.substr(middle, last - middle);

    // When the final Section lacks an EOF newline, pass the preceding block's
    // separator to it. The moved-to-EOF block then keeps the no-newline state.
    if (!right.empty() && right.back() != '\r' && right.back() != '\n') {
        std::size_t eol_size = 0;
        if (left.size() >= 2 && left.compare(left.size() - 2, 2, "\r\n") == 0) {
            eol_size = 2;
        } else if (!left.empty() && (left.back() == '\r' || left.back() == '\n')) {
            eol_size = 1;
        }
        if (eol_size == 0) {
            error = "Missing separator between Sections";
            return false;
        }
        right.append(left, left.size() - eol_size, eol_size);
        left.resize(left.size() - eol_size);
    }

    plan.first = first;
    plan.last = last;
    plan.replacement = right + left;
    plan.header_moves = {{first, first + right.size()}, {middle, first}};
    plan.caret = direction < 0
        ? first + std::min(caret - middle, right.size())
        : first + right.size() + std::min(caret - first, left.size());
    return true;
}

} // namespace ini_move
