#include "Kernel.h"
#include "Animate.h"
#include "Cels.h"
#include "Dialog.h"
#include "ErrMsg.h"
#include "Event.h"
#include "FarData.h"
#include "FileIO.h"
#include "Graphics.h"
#include "Menu.h"
#include "Mouse.h"
#include "PMachine.h"
#include "Picture.h"
#include "Resource.h"
#include "SaveGame.h"
#include "Selector.h"
#include "Text.h"
#include "Timer.h"
#include "Trig.h"
#include "Window.h"
#include <time.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __WINDOWS__
#pragma warning(disable : 4100) // unreferenced formal parameter
#endif

// Function code for PriCoord operation.
#define PTopOfBand 1

// KGraph function dispatch constants.
#define GLoadBits    1
#define GDetect      2
#define GSetPalette  3
#define GDrawLine    4
#define GFillArea    5
#define GDrawBrush   6
#define GSaveBits    7
#define GRestoreBits 8
#define GEraseRect   9
#define GPaintRect   10
#define GFillRect    11
#define GShowBits    12
#define GReAnimate   13
#define GInitPri     14

// SortNode used in Sort
typedef struct SortNode {
    Obj     *sortObject;
    intptr_t sortKey;
} SortNode;

typedef struct KNode {
    Node     link;
    intptr_t nVal;
} KNode;

int g_gameRestarted = 0;

#define ret(val) g_acc = ((uintptr_t)(val))

void KLoad(argList)
{
    Handle h = ResLoad((int)arg(1), (uint)arg(2));
    ret(h);
}

void KUnLoad(argList)
{
    switch (arg(1)) {
        case RES_MEM:
            UnloadBits((Handle)arg(2));
            break;

        default:
            ResUnLoad((int)arg(1), (size_t)arg(2));
            break;
    }
}

void KLock(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KScriptID(argList)
{
    Obj *obj =
      GetDispatchAddr((uint)arg(1), (argCount == 1) ? 0U : (uint)arg(2));
    ret(obj);
}

void KDisposeScript(argList)
{
    if (argCount == 2) {
        ret(arg(2));
    }

    DisposeScript(arg(1));
}

void KClone(argList)
{
    Obj *theClone;
    int  numArgs;

    // Get a clone of the object.
    theClone = Clone((Obj *)arg(1));

    // Set any properties.
    for (numArgs = (int)argCount - 1; numArgs > 0; numArgs -= 2) {
        args += 2;
        SetProperty(theClone, (uint)arg(0), arg(1));
    }

    ret(theClone);
}

void KDisposeClone(argList)
{
    DisposeClone((Obj *)arg(1));
}

void KIsObject(argList)
{
    bool b = IsObject((Obj *)arg(1));
    ret(b);
}

void KRespondsTo(argList)
{
    bool b = RespondsTo((Obj *)arg(1), (uint)arg(2));
    ret(b);
}

void KDrawPic(argList)
{
    RGrafPort *oldPort;
    bool       clear = true;
    uint       palette;

    RGetPort(&oldPort);
    RSetPort(&g_picWind->port);

    // Get optional show style if present.
    if (argCount >= 2) {
        g_showPicStyle = (uint)arg(2);
        // Get optional overlay boolean.
        if (argCount >= 3) {
            clear = (arg(3) != 0);

            if (argCount >= 4) {
                palette = (uint)arg(4);
            }
        }
    }

    if (!RFrontWindow(g_picWind)) {
        RBeginUpdate(g_picWind);
        // g_showPicStyle contains mirroring bits.
        DrawPic(ResLoad(RES_PIC, arg(1)), clear, g_showPicStyle);

        REndUpdate(g_picWind);
    } else {
        DrawPic(ResLoad(RES_PIC, arg(1)), clear, g_showPicStyle);
        g_picNotValid = 1;
    }

    RSetPort(oldPort);
}

void KShow(argList)
{
    RGrafPort *oldPort;

    RGetPort(&oldPort);
    RSetPort(&g_picWind->port);
    g_picNotValid = 2;
    g_showMap     = (uint)arg(1);
    RSetPort(oldPort);
}

void KPicNotValid(argList)
{
    ret(g_picNotValid);
    if (argCount != 0) {
        g_picNotValid = (uint)arg(1);
    }
}

void KSetNowSeen(argList)
{
    Obj *him = (Obj *)arg(1);

    GetCelRect((View *)ResLoad(RES_VIEW, GetProperty(him, s_view)),
               (uint)GetProperty(him, s_loop),
               (uint)GetProperty(him, s_cel),
               (int)GetProperty(him, s_x),
               (int)GetProperty(him, s_y),
               (int)GetProperty(him, s_z),
               (RRect *)GetPropAddr(him, s_nsTop));
}

void KNumLoops(argList)
{
    uint numLoops;
    Obj *him = (Obj *)arg(1);

    numLoops = GetNumLoops((View *)ResLoad(RES_VIEW, GetProperty(him, s_view)));
    ret(numLoops);
}

void KNumCels(argList)
{
    uint numCels;
    Obj *him = (Obj *)arg(1);

    numCels = GetNumCels((View *)ResLoad(RES_VIEW, GetProperty(him, s_view)),
                         GetProperty(him, s_loop));
    ret(numCels);
}

void KCelWide(argList)
{
    int width =
      GetCelWide((View *)ResLoad(RES_VIEW, arg(1)), (uint)arg(2), (uint)arg(3));
    ret(width);
}

void KCelHigh(argList)
{
    int height =
      GetCelHigh((View *)ResLoad(RES_VIEW, arg(1)), (uint)arg(2), (uint)arg(3));
    ret(height);
}

void KDrawCel(argList)
{
    RRect     r;
    View     *view;
    uint      loop, cel;
    RPalette *palette;

    view    = (View *)ResLoad(RES_VIEW, arg(1));
    loop    = (uint)arg(2);
    cel     = (uint)arg(3);
    palette = (argCount >= 7) ? (RPalette *)arg(7) : NULL;

    r.left   = (int16_t)arg(4);
    r.top    = (int16_t)arg(5);
    r.right  = r.left + (int16_t)GetCelWide(view, loop, cel);
    r.bottom = r.top + (int16_t)GetCelHigh(view, loop, cel);
    DrawCel(view, loop, cel, &r, (uint)arg(6), palette);

    if (g_picNotValid == 0) {
        ShowBits(&r, VMAP);
    }
}

void KIsItSkip(argList)
{
    View *view;

    view = (View *)ResLoad(RES_VIEW, arg(1));
    ret(RIsItSkip(view, (uint)arg(2), (uint)arg(3), (int)arg(4), (int)arg(5)));
}

void KAnimate(argList)
{
    if (argCount == 2) {
        Animate((List *)arg(1), (bool)arg(2));
    } else {
        Animate(NULL, false);
    }
}

void KAddToPic(argList)
{
    AddToPic((List *)arg(1));
    g_picNotValid = 2;
}

void KPalette(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGraph(argList)
{
    switch ((int)arg(1)) {
        case GLoadBits:

            //   LoadBits();
        case GDetect:
            ret(256);
            break;
    }
}

void KShakeScreen(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KShiftScreen(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDrawControl(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KHiliteControl(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KEditControl(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KTextSize(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDisplay(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGetEvent(argList)
{
    REventRecord evt;
    ushort       type;

    type = (ushort)arg(1);
    if ((type & leaveEvt) != 0) {
        ret(REventAvail(type, &evt));
    } else {
        ret(RGetNextEvent(type, &evt));
    }

#if 0
    if (evt.type == keyDown)
    {
        if ('A' <= evt.message && evt.message <= 'Z')
        {
            if (g_theGameObj != NULL)
            {
                if (GetProperty(g_theGameObj, s_parseLang) == JAPANESE)
                {
                    evt.message += 32;
                }
            }
        }
    }
#endif

    EventToObj(&evt, (Obj *)arg(2));
}

// ARGS: top, left, bottom, right, title, type, priority, color, back.
// Open the window with NO draw (vis false), then set port values.
void KNewWindow(argList)
{
    RRect    r;
    RWindow *wind;

    r.top    = (int16_t)arg(1);
    r.left   = (int16_t)arg(2);
    r.bottom = (int16_t)arg(3);
    r.right  = (int16_t)arg(4);

    wind =
      RNewWindow(&r, (const char *)arg(5), (uint)arg(6), (uint)arg(7), false);

    // Set port characteristics based on remainder of args.
    g_rThePort->fgColor = (uint)arg(8);
    g_rThePort->bkColor = (uint)arg(9);
    RDrawWindow(wind);
    ret(wind);
}

void KDisposeWindow(argList)
{
    bool eraseOnly;

    eraseOnly = (argCount == 2 && arg(2) != 0);
    RDisposeWindow((RWindow *)arg(1), eraseOnly);
}

void KGetPort(argList)
{
    RGrafPort *oldPort;

    RGetPort(&oldPort);
    ret(oldPort);
}

// Calls to KSetPort with an argument count of 6 or more are assumed to be calls
// to SetPortRect.
void KSetPort(argList)
{
    if (argCount >= 6) {
        g_picWind->port.portRect.top    = (int16_t)arg(1);
        g_picWind->port.portRect.left   = (int16_t)arg(2);
        g_picWind->port.portRect.bottom = (int16_t)arg(3);
        g_picWind->port.portRect.right  = (int16_t)arg(4);
        g_picWind->port.origin.v        = (int16_t)arg(5);
        g_picWind->port.origin.h        = (int16_t)arg(6);
        if (argCount >= 7) {
            InitPicture();
        }
    } else {
        RGrafPort *port;
        if (arg(1) == 0) {
            port = RGetWmgrPort();
        }
        // TODO: is this actually 0xffffffff or 0x0000ffff?
        else if (arg(1) == (uintptr_t)-1) {
            port = g_menuPort;
        } else {
            port = (RGrafPort *)arg(1);
        }
        RSetPort(port);
    }
}

void KHaveMouse(argList)
{
    ret(g_haveMouse);
}

void KSetCursor(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KJoystick(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGetSaveFiles(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSaveGame(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KRestoreGame(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KRestartGame(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGameIsRestarting(argList)
{
    ret(g_gameRestarted);
    if (argCount != 0 && arg(1) == FALSE) {
        g_gameRestarted = 0;
    }
}

void KDoSound(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDoAudio(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDoSync(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGlobalToLocal(argList)
{
    RGlobalToLocal((RPoint *)IndexedPropAddr((Obj *)arg(1), evY));
}

void KLocalToGlobal(argList)
{
    RLocalToGlobal((RPoint *)IndexedPropAddr((Obj *)arg(1), evY));
}

void KMapKeyToDir(argList)
{
    Obj         *sciEvent;
    REventRecord event;

    sciEvent = (Obj *)arg(1);
    ObjToEvent(sciEvent, &event);
#ifndef NOT_IMPL
    MapKeyToDir(&event);
#endif
    EventToObj(&event, sciEvent);
    ret(sciEvent);
}

void KDrawMenuBar(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KMenuSelect(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KAddMenu(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGetMenu(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSetMenu(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDrawStatus(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KParse(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSaid(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSetSynonyms(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KNewList(argList)
{
    List *list;

    list = (List *)malloc(sizeof(List));
    InitList(list);
    ret(list);
}

void KDisposeList(argList)
{
    List *list;
    Node *kNode;

    list = (List *)arg(1);

    while (!EmptyList(list)) {
        kNode = FirstNode(list);
        DeleteNode(list, kNode);
        free(kNode);
    }

    free(list);
}

void KNewNode(argList)
{
    KNode *node;

    node       = (KNode *)malloc(sizeof(KNode));
    node->nVal = (intptr_t)arg(1);
    ret(node);
}

void KFirstNode(argList)
{
    Node *node = (arg(1) != 0) ? FirstNode((List *)arg(1)) : NULL;
    ret(node);
}

void KLastNode(argList)
{
    Node *node = (arg(1) != 0) ? LastNode((List *)arg(1)) : NULL;
    ret(node);
}

void KEmptyList(argList)
{
    bool b = (arg(1) == 0) || EmptyList((List *)arg(1));
    ret(b);
}

void KNextNode(argList)
{
    Node *node = NextNode((Node *)arg(1));
    ret(node);
}

void KPrevNode(argList)
{
    Node *node = PrevNode((Node *)arg(1));
    ret(node);
}

void KNodeValue(argList)
{
    ret(((KNode *)arg(1))->nVal);
}

void KAddAfter(argList)
{
    Node *node = AddKeyAfter(
      (List *)arg(1), (Node *)arg(2), (Node *)arg(3), (intptr_t)arg(4));
    ret(node);
}

void KAddToFront(argList)
{
    Node *node =
      AddKeyToFront((List *)arg(1), (Node *)arg(2), (intptr_t)arg(3));
    ret(node);
}

void KAddToEnd(argList)
{
    Node *node = AddKeyToEnd((List *)arg(1), (Node *)arg(2), (intptr_t)arg(3));
    ret(node);
}

void KFindKey(argList)
{
    Node *node = FindKey((List *)arg(1), (intptr_t)arg(2));
    ret(node);
}

void KDeleteKey(argList)
{
    Node *theNode;

    theNode = DeleteKey((List *)arg(1), (intptr_t)arg(2));
    if (theNode != NULL) {
        free(theNode);
    }
    ret(theNode != NULL);
}

void KRandom(argList)
{
    static uint s_seed = 0;
    uint        low, high, range;
    int         rnd;

    if (argCount == 2) {
        if (s_seed == 0) {
            s_seed = (uint)time(NULL);
        }

        low   = (uint)arg(1);
        high  = (uint)arg(2);
        range = high - low + 1;

        rnd = (rand() % range) + low;
        ret(rnd);
    } else if (argCount == 1) {
        // Set seed to argument.
        s_seed = (uint)arg(1);
        srand(s_seed);
        ret(FALSE);
    } else {
        // Return seed value.
        ret(s_seed);
    }
}

void KAbs(argList)
{
    ret(abs((int)arg(1)));
}

void KSqrt(argList)
{
    ret((uint)round(sqrt((double)abs((int)arg(1)))));
}

void KGetAngle(argList)
{
    RPoint sp, dp;
    int    angle;

    sp.h  = (int16_t)arg(1);
    sp.v  = (int16_t)arg(2);
    dp.h  = (int16_t)arg(3);
    dp.v  = (int16_t)arg(4);
    angle = RPtToAngle(&sp, &dp);
    ret(angle);
}

void KGetDistance(argList)
{
    int    dx, dy;
    double dySq;

    dx = abs((int)arg(3) - (int)arg(1));
    dy = abs((int)arg(4) - (int)arg(2));

    if (argCount > 4) {
        double cs = cos(((double)(intptr_t)arg(5)) / (180.0 / M_PI));
        dySq      = round((double)dy / cs);
        dySq      = dySq * dySq;
    } else {
        dySq = (double)(dy * dy);
    }

    ret((uint)round(sqrt((double)(dx * dx) + dySq)));
}

void KSinMult(argList)
{
    double res = sin(((double)(intptr_t)arg(1)) / (180.0 / M_PI));
    ret((int)round(((double)(intptr_t)arg(2)) * res));
}

void KCosMult(argList)
{
    double res = cos(((double)(intptr_t)arg(1)) / (180.0 / M_PI));
    ret((int)round(((double)(intptr_t)arg(2)) * res));
}

void KSinDiv(argList)
{
    double res = sin(((double)(intptr_t)arg(1)) / (180.0 / M_PI));
    ret((int)round(((double)(intptr_t)arg(2)) / res));
}

void KCosDiv(argList)
{
    double res = cos(((double)(intptr_t)arg(1)) / (180.0 / M_PI));
    ret((int)round(((double)(intptr_t)arg(2)) / res));
}

void KATan(argList)
{
    ret(ATan((int)arg(1), (int)arg(2), (int)arg(3), (int)arg(4)));
}

void KWait(argList)
{
    static uint s_lastTick = 0;
    uint        ticks, sysTicks;

    ticks = (uint)arg(1);

    if (ticks != 0) {
        WaitUntil(ticks + s_lastTick);
    }

    sysTicks = RTickCount();
    ret(sysTicks - s_lastTick);
    s_lastTick = sysTicks;
}

void KGetTime(argList)
{
    uint time;

    if (argCount == 1) {
        time = SysTime((int)arg(1));
    } else {
        time = RTickCount();
    }
    ret(time);
}

void KStrEnd(argList)
{
    const char *str = (const char *)arg(1);
    if (str != NULL) {
        str += strlen(str);
    }
    ret(str);
}

void KStrCat(argList)
{
    ret(strcat((char *)arg(1), (const char *)arg(2)));
}

void KStrCmp(argList)
{
    if (argCount == 2) {
        ret(strcmp((const char *)arg(1), (const char *)arg(2)));
    } else {
        ret(strncmp((const char *)arg(1), (const char *)arg(2), arg(3)));
    }
}

void KStrLen(argList)
{
    ret(strlen((const char *)arg(1)));
}

void KStrCpy(argList)
{
    if (argCount == 2) {
        ret(strcpy((char *)arg(1), (const char *)arg(2)));
    } else {
        // A positive length does standard string op.
        if ((int)arg(3) > 0) {
            ret(strncpy((char *)arg(1), (const char *)arg(2), arg(3)));
        } else {
            // A negative length does mem copy without NULL terminator.
            ret(memcpy(
              (void *)arg(1), (const void *)arg(2), (size_t)(-(int)arg(3))));
        }
    }
}

void KFormat(argList)
{
    char       *text;
    const char *format;
    size_t      i, n;
    uintptr_t   theArg;
    char        c, *strArg, theStr[20], buffer[2000], temp[500];

    text   = (char *)arg(1);
    format = (const char *)arg(2);

    //
    // The following code is quite a kludge. It relies on the fact that a
    // valid Script address will be greater than 1000 to let us distinguish
    // between a near string address and the module number of a far string.
    // If we're dealing with a far string, we copy it into 'buffer' to make
    // it near for formatting purposes.
    //
    if ((uintptr_t)format < 1000) {
        format = GetFarText((uint)arg(2), (uint)arg(3), buffer);
        n = 4;
    } else {
        n = 3;
    }

    // Format message.
    while ((c = *format++) != '\0') {
        if (c != '%') {
            *text++ = c;
        } else {
            // Build a string to pass to sprintf.
            theStr[0] = '%';
            for (i = 1; i < sizeof(theStr); ++i) {
                c = *format++;
                theStr[i] = c;
                if (isalpha(c)) {
                    theStr[i + 1] = 0;
                    theArg = arg(n++);
                    if (c == 's') {
                        if (theArg < 1000) {
                            strArg = GetFarText(theArg, arg(n++), temp);
                        } else {
                            strArg = (char *)theArg;
                        }
                        text += sprintf(text, theStr, strArg);
                    } else {
                        text += sprintf(text, theStr, theArg);
                    }
                    break;
                }
            }
        }
    }

    *text = 0;
    ret(arg(1));
}

void KStrAt(argList)
{
    char *sp;

    sp = (char *)arg(1) + (int)arg(2);
    ret(*sp);

    // If a third argument is present, set the byte to that value.
    if (argCount == 3) {
        *sp = (char)arg(3);
    }
}

void KStrSplit(argList)
{
    char       *dst = (char *)arg(1);
    const char *src = (const char *)arg(2);

    if (argCount >= 3 && arg(3) != 0) {
        size_t      len;
        const char *end;

        end = strstr(src, (const char *)arg(3));

        if (end != NULL) {
            len = end - src;
        } else {
            len = strlen(src);
        }

        memcpy(dst, src, len);
        dst[len] = '\0';
    } else {
        strcpy(dst, src);
    }
    ret(dst);
}

void KGetCWD(argList)
{
    ret(getcwd((char *)arg(1), 68));
}

void KGetFarText(argList)
{
    ret(GetFarText((uint)arg(1), (uint)arg(2), (char *)arg(3)));
}

void KReadNumber(argList)
{
    ret(atoi((const char *)arg(1)));
}

void KBaseSetter(argList)
{
    Obj  *actor;
    int   theY;
    View *view;

    actor = (Obj *)arg(1);
    view  = (View *)ResLoad(RES_VIEW, IndexedProp(actor, actView));

    GetCelRect(view,
               (uint)IndexedProp(actor, actLoop),
               (uint)IndexedProp(actor, actCel),
               (int)IndexedProp(actor, actX),
               (int)IndexedProp(actor, actY),
               (int)IndexedProp(actor, actZ),
               (RRect *)IndexedPropAddr(actor, actBR));

    theY                            = 1 + (int)IndexedProp(actor, actY);
    IndexedProp(actor, actBRBottom) = (uintptr_t)theY;
    IndexedProp(actor, actBR) =
      (uintptr_t)(theY - (int)IndexedProp(actor, actYStep));
}

void KDirLoop(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KCantBeHere(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KOnControl(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KInitBresen(argList)
{
    Obj *motion, *client;
    bool xAxis;
    int  DX, DY, toX, toY, dx, dy, incr, i1, i2, di;
    int  tdx, tdy, watchDog;
    int  skipFactor;

    motion     = (Obj *)arg(1);
    client     = (Obj *)IndexedProp(motion, motClient);
    skipFactor = (argCount >= 2) ? arg(2) : 1;

    toX = IndexedProp(motion, motX);
    toY = IndexedProp(motion, motY);

    tdx = IndexedProp(client, actXStep) * skipFactor;
    tdy = IndexedProp(client, actYStep) * skipFactor;

    watchDog = (tdx > tdy) ? tdx : tdy;
    watchDog *= 2;

    // Get distances to be moved.
    DX = toX - IndexedProp(client, actX);
    DY = toY - IndexedProp(client, actY);

    // Compute basic step sizes.
    while (true) {
        dx = tdx;
        dy = tdy;

        if (abs(DX) >= abs(DY)) {
            // Then motion will be along the x-axis.
            xAxis = true;
            if (DX < 0) {
                dx = -dx;
            }
            dy = (DX) ? dx * DY / DX : 0;
        } else {
            // Our major motion is along the y-axis.
            xAxis = false;
            if (DY < 0) {
                dy = -dy;
            }
            dx = (DY) ? dy * DX / DY : 0;
        }

        // Compute increments and decision variable.
        i1   = (xAxis) ? 2 * (dx * DY - dy * DX) : 2 * (dy * DX - dx * DY);
        incr = 1;
        if ((xAxis && DY < 0) || (!xAxis && DX < 0)) {
            i1   = -i1;
            incr = -1;
        }

        i2 = i1 - 2 * ((xAxis) ? DX : DY);
        di = i1 - ((xAxis) ? DX : DY);

        if ((xAxis && DX < 0) || (!xAxis && DY < 0)) {
            i1 = -i1;
            i2 = -i2;
            di = -di;
        }

        // Limit x step to avoid over stepping Y step size.
        if (xAxis && tdx > tdy) {
            if (tdx && abs(dy + incr) > tdy) {
                if (!(--watchDog)) {
                    RAlert(E_BRESEN);
                    exit(1);
                }
                --tdx;
                continue;
            }
        }
#if 0
        // DON'T modify X
        else
        {
            if (tdy && abs(dx + incr) > tdx)
            {
                --tdy;
                continue;
            }
        }
#endif

        break;
    }

    // Set the various variables we've computed.
    IndexedProp(motion, motDX)    = dx;
    IndexedProp(motion, motDY)    = dy;
    IndexedProp(motion, motI1)    = i1;
    IndexedProp(motion, motI2)    = i2;
    IndexedProp(motion, motDI)    = di;
    IndexedProp(motion, motIncr)  = incr;
    IndexedProp(motion, motXAxis) = xAxis;
}

void KDoBresen(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDoAvoider(argList) {}

void KAvoidPath(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSetJump(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KSetDebug(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KInspectObj(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KShowSends(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KShowObjs(argList) {}

void KShowFree(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KMemoryInfo(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KStackUsage(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KCheckFreeSpace(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KValidPath(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KProfiler(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KDeviceInfo(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KGetSaveDir(argList)
{
    ret(g_saveDir);
}

void KCheckSaveGame(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KFlushResources(argList)
{
#if 0
    newRoomNum = arg(1);

    if (trackResUse) {
        while (!PurgeLast())
            ;
    }
#endif
}

void KMemorySegment(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KIntersections(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KMemory(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KListOps(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}

void KCoordPri(argList)
{
    if (argCount >= 2 && arg(1) == PTopOfBand) {
        ret(PriCoord(arg(2)));
    } else {
        ret(CoordPri(arg(1)));
    }
}

void KFileIO(argList)
{
#ifdef __WINDOWS__
    static WIN32_FIND_DATAA findFileData;
    static HANDLE           hFind = INVALID_HANDLE_VALUE;
#else
#error Not impl
#endif

    char *buf;
    int   mode;
    int   fd;
    bool  res;

    switch ((int)arg(1)) {
        case fileOpen:
            buf  = (char *)arg(2);
            mode = (int)arg(3);
            if (mode == F_TRUNC) {
                mode = O_TRUNC | O_CREAT | O_RDWR;
            } else if (mode == F_APPEND) {
                mode = O_APPEND | O_CREAT | O_RDWR;
            } else {
                mode = O_RDONLY;
            }
            fd = open(buf, O_BINARY | mode);
            ret(fd);
            break;

        case fileClose:
            ret(close((int)arg(2)) == 0);
            break;

        case fileRead:
            ret(read((int)arg(2), (void *)arg(3), (uint)arg(4)));
            break;

        case fileWrite:
            ret(write((int)arg(2), (const void *)arg(3), (uint)arg(4)));
            break;

        case fileUnlink:
            ret(unlink((const char *)arg(2)) == 0);
            break;

        case fileFGets:
            ret(fgets_no_eol((char *)arg(2), (int)arg(3), (int)arg(4)));
            break;

        case fileFPuts:
            buf = (char *)arg(3);
            ret(write((int)arg(2), buf, strlen(buf)));
            break;

        case fileSeek:
            ret(lseek((int)arg(2), (long)arg(3), (int)arg(4)));
            break;

        case fileFindFirst:
            if (hFind != INVALID_HANDLE_VALUE) {
                FindClose(hFind);
            }
            hFind = FindFirstFileA((const char *)arg(2), &findFileData);
            res   = (hFind != INVALID_HANDLE_VALUE);
            if (res) {
                strcpy((char *)arg(3), findFileData.cFileName);
            }
            ret(res);
            break;

        case fileFindNext:
            if (hFind != INVALID_HANDLE_VALUE) {
                res = FindNextFileA(hFind, &findFileData) != FALSE;
                if (res) {
                    strcpy((char *)arg(2), findFileData.cFileName);
                } else {
                    FindClose(hFind);
                    hFind = INVALID_HANDLE_VALUE;
                }
            } else {
                res = false;
            }
            ret(res);
            break;

        case fileExists:
            ret(fexists((const char *)arg(2)));
            break;

        case fileRename:
            ret(rename((const char *)arg(2), (const char *)arg(3)));
            break;

        case fileCopy:
            ret(fcopy((const char *)arg(2), (const char *)arg(3)));
            break;

        default:
            break;
    }
}

// A simple bubble sort.
static void bsort(SortNode *theList, uint size)
{
    int      i, j;
    SortNode tmp;

    size--;
    for (i = 0; i < (int)size; i++) {
        for (j = (int)size; j > i; j--) {
            if (theList[j].sortKey < theList[j - 1].sortKey) {
                // Swap.
                tmp            = theList[j];
                theList[j]     = theList[j - 1];
                theList[j - 1] = tmp;
            }
        }
    }
}

void KSort(argList)
{
    Obj      *him;
    Node     *it;
    SortNode *sortArray;

    List  *theList, *retKList;
    Obj   *retList;
    KNode *kNode;
    uint   size, i;

    theList = (List *)GetProperty((Obj *)arg(1), s_elements);
    retList = (Obj *)arg(2);
    size    = (uint)GetProperty((Obj *)arg(1), s_size);
    if (size != 0) {
        sortArray = (SortNode *)malloc(size * sizeof(SortNode));

        // Set the keys for each SortNode based on passed scoring function.
        for (it = FirstNode(theList), i = 0; it != NULL;
             i++, it = NextNode(it)) {
            him = (Obj *)(FromNode(it, KNode)->nVal);
            sortArray[i].sortObject = him;
            sortArray[i].sortKey =
              (intptr_t)InvokeMethod(him, s_perform, 1, arg(3));
        }

        // The actual sort.
        bsort(sortArray, size);

        retKList = (List *)malloc(sizeof(List));
        InitList(retKList);

        // Stuff objects into return List.
        for (i = 0; i < size; ++i) {
            kNode       = (KNode *)malloc(sizeof(KNode));
            kNode->nVal = (intptr_t)sortArray[i].sortObject;
            AddKeyToEnd(retKList, ToNode(kNode), sortArray[i].sortObject);
        }

        SetProperty(retList, s_elements, (uintptr_t)retKList);
        SetProperty(retList, s_size, size);

        free(sortArray);
    }
}

void KMessage(argList)
{
#ifndef NOT_IMPL
#error Not finished
#endif
}