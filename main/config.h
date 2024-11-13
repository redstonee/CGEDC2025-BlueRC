#pragma once

#include <stdint.h>

enum class ColPin
{
    COL1 = 1,
    COL2 = 2,
    COL3 = 3,
    COL4 = 4,

};

enum class RowPin
{
    ROW1 = 10,
    ROW2 = 11,
    ROW3 = 25,
    ROW4 = 26,
    ROW5 = 27,
    ROW6 = 12,
};

static const ColPin colPins[] = {ColPin::COL1, ColPin::COL2, ColPin::COL3, ColPin::COL4};
static const RowPin rowPins[] = {RowPin::ROW1, RowPin::ROW2, RowPin::ROW3, RowPin::ROW4, RowPin::ROW5, RowPin::ROW6};

#define TEST_DATASRC 0