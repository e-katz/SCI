#pragma once
#ifndef VOLLOAD_H
#define VOLLOAD_H

#include "ResTypes.h"
#include "Types.h"

// Init global resource list.
void InitResource(void);

Handle DoLoad(int resType, size_t resNum);

#endif // VOLLOAD_H