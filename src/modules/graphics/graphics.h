#include <stdbool.h>

#pragma once

struct WindowFlags;

bool lovrGraphicsInit(bool debug);
void lovrGraphicsDestroy(void);
void lovrGraphicsCreateWindow(struct WindowFlags* flags);
