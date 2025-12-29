//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef ANSI_CODES_H
#define ANSI_CODES_H

/// ansi escape sequence character
#define ESC "\033"

// reset
#define ANSI_RESET ESC "[0m"

// text styles
#define ANSI_BOLD ESC "[1m"
#define ANSI_FAINT ESC "[2m"
#define ANSI_ITALIC ESC "[3m"
#define ANSI_UNDERLINE ESC "[4m"

// foreground Colors
#define ANSI_BLACK_FG ESC "[30m"
#define ANSI_RED_FG ESC "[31m"
#define ANSI_GREEN_FG ESC "[32m"
#define ANSI_YELLOW_FG ESC "[33m"
#define ANSI_BLUE_FG ESC "[34m"
#define ANSI_MAGENTA_FG ESC "[35m"
#define ANSI_CYAN_FG ESC "[36m"
#define ANSI_WHITE_FG ESC "[37m"

// background colors
#define ANSI_BLACK_BG ESC "[40m"
#define ANSI_RED_BG ESC "[41m"
#define ANSI_GREEN_BG ESC "[42m"
#define ANSI_YELLOW_BG ESC "[43m"
#define ANSI_BLUE_BG ESC "[44m"
#define ANSI_MAGENTA_BG ESC "[45m"
#define ANSI_CYAN_BG ESC "[46m"
#define ANSI_WHITE_BG ESC "[47m"

// more
#define ANSI_BOLD_GREEN ANSI_BOLD ANSI_GREEN_FG
#define ANSI_BOLD_MAGENTA ANSI_BOLD ANSI_MAGENTA_FG
#define ANSI_BOLD_RED ANSI_BOLD ANSI_RED_FG
#define ANSI_BOLD_YELLOW ANSI_BOLD ANSI_YELLOW_FG
#define ANSI_BOLD_CYAN ANSI_BOLD ANSI_CYAN_FG
#define ANSI_BOLD_BLUE ANSI_BOLD ANSI_BLUE_FG
#define ANSI_BOLD_WHITE ANSI_BOLD ANSI_WHITE_FG

#endif // ANSI_CODES_H
