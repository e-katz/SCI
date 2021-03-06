#ifndef SCI_KERNEL_CELS_H
#define SCI_KERNEL_CELS_H

#include "sci/Kernel/GrTypes.h"
#include "sci/Kernel/Palette.h"
#include "sci/Kernel/View.h"

// Entry point for drawing non-scaled views.
void DrawCel(View        *view,
             uint         loop,
             uint         cel,
             const RRect *r,
             uint         priority,
             RPalette    *pal);

void DrawBitMap(int       x,
                int       y,
                bool      mirror,
                Cel      *cel,
                uint      priority,
                RPalette *pal);

// Return number of loops in this view.
uint GetNumLoops(View *view);

// Find out how many cels the loop has.
uint GetNumCels(View *view, uint loopNum);

// Return width of this cel.
int GetCelWide(View *view, uint loopNum, uint celNum);

// Return height of this cel.
int GetCelHigh(View *view, uint loopNum, uint celNum);

// Produce a rectangle in passed structure for desired cel.
void GetCelRect(View  *view,
                uint   loopNum,
                uint   celNum,
                int    x,
                int    y,
                int    z,
                RRect *rect);
void GetCelRectNative(View      *view,
                      uint       loopNum,
                      uint       celNum,
                      int        x,
                      int        y,
                      int        z,
                      uintptr_t *nrect);

// This routine takes a pointer to the view pointer, the loop, and the cel
// and returns the cel pointer.
Cel *GetCelPointer(View *view, uint loop, uint cel);

bool RIsItSkip(View *view, uint loopNum, uint celNum, int vOffset, int hOffset);

#endif // SCI_KERNEL_CELS_H
