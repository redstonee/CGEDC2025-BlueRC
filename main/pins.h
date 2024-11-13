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
    ROW1 = 27,
    ROW2 = 26,
    ROW3 = 25,
    ROW4 = 10,
    ROW5 = 11,
    ROW6 = 12,
};

static const ColPin ColPins[] = {ColPin::COL1, ColPin::COL2, ColPin::COL3, ColPin::COL4};
static const RowPin RowPins[] = {RowPin::ROW1, RowPin::ROW2, RowPin::ROW3, RowPin::ROW4, RowPin::ROW5, RowPin::ROW6};

#define TEST_DATASRC 1