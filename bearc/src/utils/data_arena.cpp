//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "utils/data_arena.hpp"
#include "utils/arena.h"
#include <stddef.h>
DataArena::DataArena(size_t chunk_size) : arena_(arena_create(chunk_size)) {}

DataArena::~DataArena() { arena_destroy(&this->arena_); }

size_t DataArena::chunk_size() const noexcept { return this->arena_.chunk_size; }

void DataArena::log_debug_info() { arena_log_debug_info(&this->arena_); }

arena_t* DataArena::arena() { return &this->arena_; }
