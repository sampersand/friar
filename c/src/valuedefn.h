#pragma once

// This file is needed because many other files (such as `array.h`) rely upon `value`
// being defined. However, `value.h` relies upon `array` and friends being defined for
// functions like creation and casting.

/*
The value type.

Note that friar has reference counting semantics.
*/
typedef unsigned long long value;

// since we do bit packing, we need this minimum alignment.
#define VALUE_ALIGNMENT _Alignas(8)

#define VFALSE 0
#define VNULL 1
#define VTRUE 2
#define VUNDEF 3
