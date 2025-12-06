#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

// Terminal UI for MLFQ Scheduler
namespace TerminalUI {

// Color codes for professional appearance
namespace Colors {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string DIM = "\033[2m";
    
    const string PRIMARY = "\033[0m";      // Default (no color)
    const string SUCCESS = "\033[32m";      // Green
    const string WARNING = "\033[93m";      // Bright Yellow
    const string ERROR = "\033[31m";        // Red
    const string INFO = "\033[36m";         // Cyan
    
    const string HIGHLIGHT = "\033[0m";  // Default (no color)
}

// Professional styling
namespace Style {
    inline string header(const string& text) {
        return Colors::BOLD + Colors::PRIMARY + text + Colors::RESET;
    }
    
    inline string highlight(const string& text) {
        return Colors::HIGHLIGHT + text + Colors::RESET;
    }
    
    inline string success(const string& text) {
        return Colors::SUCCESS + text + Colors::RESET;
    }
    
    inline string warning(const string& text) {
        return Colors::WARNING + text + Colors::RESET;
    }
    
    inline string error(const string& text) {
        return Colors::ERROR + text + Colors::RESET;
    }
    
    inline string info(const string& text) {
        return Colors::INFO + text + Colors::RESET;
    }
}

// Utility functions for clean formatting
inline void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        cout << "\033[2J\033[H";
    #endif
}

inline string pad(const string& text, int width, char fill = ' ') {
    if (static_cast<int>(text.length()) >= width) return text;
    return string(width - text.length(), fill) + text;
}

inline string padLeft(const string& text, int width, char fill = ' ') {
    if (static_cast<int>(text.length()) >= width) return text;
    return text + string(width - text.length(), fill);
}

inline string padCenter(const string& text, int width) {
    if (static_cast<int>(text.length()) >= width) return text;
    int leftPad = (width - text.length()) / 2;
    int rightPad = width - text.length() - leftPad;
    return string(leftPad, ' ') + text + string(rightPad, ' ');
}

// Professional box drawing functions
inline void drawSeparator(int width = 80, char ch = '-') {
    cout << string(width, ch) << endl;
}

inline void drawHeader(const string& title, int width = 80) {
    drawSeparator(width, '=');
    cout << Style::header(padCenter(title, width)) << endl;
    drawSeparator(width, '-');
}

inline void drawSubHeader(const string& title, int width = 80) {
    cout << "\n" << Style::info(title) << endl;
    drawSeparator(width, '-');
}

inline void drawSection(const string& title, int width = 80) {
    cout << "\n" << Style::header(" " + title + " ") << endl;
}

inline void drawRowSeparator(int width = 80) {
    cout << string(width, '-') << endl;
}

inline string formatKeyValue(const string& key, const string& value, int totalWidth = 60) {
    stringstream ss;
    ss << left << setw(30) << key << ": " << value;
    return ss.str();
}

// Progress bar with consistent styling
inline string progressBar(int current, int total, int width = 20) {
    if (total <= 0 || width <= 0) return string(width, ' ');
    
    double percentage = total > 0 ? static_cast<double>(current) / total : 0.0;
    int filled = static_cast<int>(percentage * width);
    
    string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) {
            bar += "█";
        } else {
            bar += "░";
        }
    }
    bar += "] " + to_string(current) + "/" + to_string(total);
    
    return bar;
}

// Clean column formatting
inline string formatColumn(const vector<string>& columns, const vector<int>& widths) {
    stringstream ss;
    for (size_t i = 0; i < columns.size() && i < widths.size(); ++i) {
        ss << left << setw(widths[i]) << columns[i];
    }
    return ss.str();
}

} // namespace TerminalUI

#endif // TERMINAL_UI_H