//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef UTILS_LOG_HPP
#define UTILS_LOG_HPP

#include <iostream>

#ifdef DEBUG_BUILD
#define ERR(x) std::cerr << ansi_bold_red() << "debug error" << ": " << x << ansi_reset() << '\n'
#endif

#endif
