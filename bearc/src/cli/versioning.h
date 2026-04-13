//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef CLI_VERSIONING_H
#define CLI_VERSIONING_H

#ifdef __i386__
#define BEARC_ARCH "x86"

#elif defined(__x86_64__)
#define BEARC_ARCH "intel x64"

#elif defined(__amd64__)
#define BEARC_ARCH "amd x64"

#elif defined(__arm__)
#define BEARC_ARCH "aarach32"

#elif defined(__aarch64__)
#define BEARC_ARCH "aarch64"

#else
#define BEARC_ARCH "unknown architecture"

#endif

#define BEARC_VERSION_FALLBACK "unversioned"

#ifdef BEARC_VERSION
#define BEARC_VERSION_STR BEARC_VERSION " " BEARC_ARCH
#else
#define BEARC_VERSION_STR BEARC_VERSION_FALLBACK " (" __DATE__ ", " __TIME__ ") " BEARC_ARCH
#endif
#endif
