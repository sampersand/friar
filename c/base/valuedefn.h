#pragma once

// This file is needed because many other files (such as `array.h`) rely upon `value`
// being defined. However, `value.h` relies upon `array` and friends being defined for
// functions like creation and casting.

/*
`value`s are actually encoded using bit-masking.
00...000 = VFALSE
00...001 = VNULL
00...010 = VTRUE
00...011 = VUNDEF
XX...100 = number
XX...000 = string
XX...001 = function
XX...010 = ary
*/
typedef unsigned long long value;

#define VFALSE 0
#define VNULL 1
#define VTRUE 2
#define VUNDEF 3
