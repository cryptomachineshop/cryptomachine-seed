#pragma once

// Development diagnostics are intentionally compile-time gated.
//
// Production builds set CRYPTOMACHINE_DEV_LOGGING=0. In that mode,
// logging calls preprocess to nothing, their format strings/arguments
// are not emitted by this header, and USB stdio is disabled by CMake.
//
// Never pass dice, entropy, mnemonic words/indices, hashes derived from
// secret input, or other sensitive session material to CM_DEV_LOG.
#ifndef CRYPTOMACHINE_DEV_LOGGING
#define CRYPTOMACHINE_DEV_LOGGING 0
#endif

#if     CRYPTOMACHINE_DEV_LOGGING != 0 &&     CRYPTOMACHINE_DEV_LOGGING != 1
#error "CRYPTOMACHINE_DEV_LOGGING must be 0 or 1"
#endif

#if CRYPTOMACHINE_DEV_LOGGING

#include <cstdio>

#define CM_DEV_LOG(...)     std::printf(__VA_ARGS__)

#else

#define CM_DEV_LOG(...)     do {                    } while (false)

#endif
