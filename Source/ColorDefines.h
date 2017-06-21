#pragma once
#include "Core.h"

#define COLOR_ENTRY(symbol, color, name) static const Color32 k##symbol = Color32(color);

namespace Color
{
#include "ListOfColors.inl"
}

#undef COLOR_ENTRY