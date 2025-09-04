#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define OS_WINDOWS
#elif defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#define OS_MACOS
#else
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#define OS_LINUX
#endif

int parse_value(const char* arg, const char* prefix,
                int current, int* is_relative, int* has_value) {
    if (strstr(arg, prefix) != arg) return current;
    const char* value_str = arg + strlen(prefix);
    *has_value = 1;

    if (strlen(value_str) == 0) {
        *is_relative = 0;
        return current;
    }
    if (strcmp(value_str, "+") == 0) {
        *is_relative = 1;
        return 1;
    }
    if (strcmp(value_str, "-") == 0) {
        *is_relative = 1;
        return -1;
    }
    if ((value_str[0] == '+' || value_str[0] == '-') && isdigit(value_str[1])) {
        *is_relative = 1;
        return atoi(value_str);
    }
    if (isdigit(value_str[0]) ||
        (value_str[0] == '-' && isdigit(value_str[1]))) {
        *is_relative = 0;
    return atoi(value_str);
        }

        *has_value = 0;
        return current;
                }

                void get_screen_size(int* width, int* height) {
                    #if defined(OS_WINDOWS)
                    *width  = GetSystemMetrics(SM_CXSCREEN);
                    *height = GetSystemMetrics(SM_CYSCREEN);
                    #elif defined(OS_MACOS)
                    CGRect r = CGDisplayBounds(CGMainDisplayID());
                    *width  = (int)r.size.width;
                    *height = (int)r.size.height;
                    #elif defined(OS_LINUX)
                    Display* d = XOpenDisplay(NULL);
                    if (d) {
                        *width  = DisplayWidth(d, 0);
                        *height = DisplayHeight(d, 0);
                        XCloseDisplay(d);
                    } else {
                        *width = 1920;
                        *height = 1080;
                    }
                    #endif
                }

                void get_mouse_position(int* x, int* y) {
                    #if defined(OS_WINDOWS)
                    POINT pt;
                    GetCursorPos(&pt);
                    *x = pt.x;  *y = pt.y;
                    #elif defined(OS_MACOS)
                    CGEventRef e = CGEventCreate(NULL);
                    CGPoint p = CGEventGetLocation(e);
                    CFRelease(e);
                    *x = (int)p.x;  *y = (int)p.y;
                    #elif defined(OS_LINUX)
                    Display* d = XOpenDisplay(NULL);
                    if (!d) { *x = 0; *y = 0; return; }
                    Window root, child;
                    int rx, ry, wx, wy;
                    unsigned int mask;
                    if (XQueryPointer(d,
                        DefaultRootWindow(d),
                                      &root, &child,
                                      &rx, &ry,
                                      &wx, &wy,
                                      &mask)) {
                        *x = rx;  *y = ry;
                                      }
                                      XCloseDisplay(d);
                                      #endif
                }

                void move_mouse(int x, int y,
                                int is_rel_x, int is_rel_y,
                                int has_x, int has_y) {
                    int cx, cy;
                    get_mouse_position(&cx, &cy);

                    int nx = has_x ? (is_rel_x ? cx + x : x) : cx;
                    int ny = has_y ? (is_rel_y ? cy + y : y) : cy;

                    int sw, sh;
                    get_screen_size(&sw, &sh);
                    if (nx < 0)   nx = 0;
                    if (ny < 0)   ny = 0;
                    if (nx >= sw) nx = sw - 1;
                    if (ny >= sh) ny = sh - 1;

                    #if defined(OS_WINDOWS)
                    SetCursorPos(nx, ny);
                    #elif defined(OS_MACOS)
                    CGEventRef mv = CGEventCreateMouseEvent(
                        NULL,
                        kCGEventMouseMoved,
                        CGPointMake(nx, ny),
                                                            kCGMouseButtonLeft);
                    CGEventPost(kCGHIDEventTap, mv);
                    CFRelease(mv);
                    #elif defined(OS_LINUX)
                    Display* d = XOpenDisplay(NULL);
                    if (!d) return;
                    XTestFakeMotionEvent(d, 0, nx, ny, CurrentTime);
                    XFlush(d);
                    XCloseDisplay(d);
                    #endif
                                }

                                void click_mouse(int left, int right, int middle, int forward, int back) {
                                    #if defined(OS_WINDOWS)
                                    if (left)   { mouse_event(MOUSEEVENTF_LEFTDOWN, 0,0,0,0);
                                        mouse_event(MOUSEEVENTF_LEFTUP,   0,0,0,0); }
                                        if (right)  { mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
                                            mouse_event(MOUSEEVENTF_RIGHTUP,  0,0,0,0); }
                                            if (middle) { mouse_event(MOUSEEVENTF_MIDDLEDOWN,0,0,0,0);
                                                mouse_event(MOUSEEVENTF_MIDDLEUP, 0,0,0,0); }
                                                if (forward){ mouse_event(MOUSEEVENTF_XDOWN,     0,0, XBUTTON2,0);
                                                    mouse_event(MOUSEEVENTF_XUP,       0,0, XBUTTON2,0); }
                                                    if (back)   { mouse_event(MOUSEEVENTF_XDOWN,     0,0, XBUTTON1,0);
                                                        mouse_event(MOUSEEVENTF_XUP,       0,0, XBUTTON1,0); }
                                                        #elif defined(OS_MACOS)
                                                        int mx, my;  get_mouse_position(&mx, &my);
                                                        CGPoint pt = CGPointMake(mx, my);
                                                        CGEventRef down, up;
                                                        if (left) {
                                                            down = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventLeftMouseDown,
                                                                                           pt,
                                                                                           kCGMouseButtonLeft);
                                                            up   = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventLeftMouseUp,
                                                                                           pt,
                                                                                           kCGMouseButtonLeft);
                                                            CGEventPost(kCGHIDEventTap, down);
                                                            CGEventPost(kCGHIDEventTap, up);
                                                            CFRelease(down);
                                                            CFRelease(up);
                                                        }
                                                        if (right) {
                                                            down = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventRightMouseDown,
                                                                                           pt,
                                                                                           kCGMouseButtonRight);
                                                            up   = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventRightMouseUp,
                                                                                           pt,
                                                                                           kCGMouseButtonRight);
                                                            CGEventPost(kCGHIDEventTap, down);
                                                            CGEventPost(kCGHIDEventTap, up);
                                                            CFRelease(down);
                                                            CFRelease(up);
                                                        }
                                                        if (middle) {
                                                            down = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventOtherMouseDown,
                                                                                           pt,
                                                                                           kCGMouseButtonCenter);
                                                            up   = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventOtherMouseUp,
                                                                                           pt,
                                                                                           kCGMouseButtonCenter);
                                                            CGEventSetIntegerValueField(down,
                                                                                        kCGMouseEventButtonNumber, 2);
                                                            CGEventSetIntegerValueField(up,
                                                                                        kCGMouseEventButtonNumber, 2);
                                                            CGEventPost(kCGHIDEventTap, down);
                                                            CGEventPost(kCGHIDEventTap, up);
                                                            CFRelease(down);
                                                            CFRelease(up);
                                                        }
                                                        if (forward || back) {
                                                            int btn = forward ? 4 : 3;
                                                            down = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventOtherMouseDown,
                                                                                           pt,
                                                                                           kCGMouseButtonLeft);
                                                            up   = CGEventCreateMouseEvent(NULL,
                                                                                           kCGEventOtherMouseUp,
                                                                                           pt,
                                                                                           kCGMouseButtonLeft);
                                                            CGEventSetIntegerValueField(down,
                                                                                        kCGMouseEventButtonNumber, btn);
                                                            CGEventSetIntegerValueField(up,
                                                                                        kCGMouseEventButtonNumber, btn);
                                                            CGEventPost(kCGHIDEventTap, down);
                                                            CGEventPost(kCGHIDEventTap, up);
                                                            CFRelease(down);
                                                            CFRelease(up);
                                                        }
                                                        #elif defined(OS_LINUX)
                                                        Display* d = XOpenDisplay(NULL);
                                                        if (!d) return;
                                                        if (left)   {
                                                            XTestFakeButtonEvent(d, 1, True,  CurrentTime);
                                                            XTestFakeButtonEvent(d, 1, False, CurrentTime);
                                                        }
                                                        if (right)  {
                                                            XTestFakeButtonEvent(d, 3, True,  CurrentTime);
                                                            XTestFakeButtonEvent(d, 3, False, CurrentTime);
                                                        }
                                                        if (middle) {
                                                            XTestFakeButtonEvent(d, 2, True,  CurrentTime);
                                                            XTestFakeButtonEvent(d, 2, False, CurrentTime);
                                                        }
                                                        if (forward){
                                                            XTestFakeButtonEvent(d, 8, True,  CurrentTime);
                                                            XTestFakeButtonEvent(d, 8, False, CurrentTime);
                                                        }
                                                        if (back)   {
                                                            XTestFakeButtonEvent(d, 9, True,  CurrentTime);
                                                            XTestFakeButtonEvent(d, 9, False, CurrentTime);
                                                        }
                                                        XFlush(d);
                                                        XCloseDisplay(d);
                                                        #endif
                                }

                                void scroll_mouse(int up, int down) {
                                    #if defined(OS_WINDOWS)
                                    if (up)   mouse_event(MOUSEEVENTF_WHEEL, 0,0,  up * WHEEL_DELTA, 0);
                                    if (down) mouse_event(MOUSEEVENTF_WHEEL, 0,0, -down * WHEEL_DELTA, 0);
                                    #elif defined(OS_MACOS)
                                    if (up)
                                        CGEventPost(kCGHIDEventTap,
                                                    CGEventCreateScrollWheelEvent(NULL,
                                                                                  kCGScrollEventUnitLine,
                                                                                  1, up));
                                        if (down)
                                            CGEventPost(kCGHIDEventTap,
                                                        CGEventCreateScrollWheelEvent(NULL,
                                                                                      kCGScrollEventUnitLine,
                                                                                      1, -down));
                                            #elif defined(OS_LINUX)
                                            Display* d = XOpenDisplay(NULL);
                                        if (!d) return;
                                        for (int i = 0; i < up;   ++i) {
                                            XTestFakeButtonEvent(d, 4, True,  CurrentTime);
                                            XTestFakeButtonEvent(d, 4, False, CurrentTime);
                                        }
                                        for (int i = 0; i < down; ++i) {
                                            XTestFakeButtonEvent(d, 5, True,  CurrentTime);
                                            XTestFakeButtonEvent(d, 5, False, CurrentTime);
                                        }
                                        XFlush(d);
                                        XCloseDisplay(d);
                                        #endif
                                }

                                void print_usage(const char* prog) {
                                    printf("Usage: %s [options]\n", prog);
                                    printf("Options:\n");
                                    printf("  -x=[value]         Move mouse on X axis\n");
                                    printf("  -y=[value]         Move mouse on Y axis\n");
                                    printf("  --click-left       Click left button\n");
                                    printf("  --click-right      Click right button\n");
                                    printf("  --click-middle     Click middle button\n");
                                    printf("  --click-forward    Click forward side button\n");
                                    printf("  --click-back       Click back side button\n");
                                    printf("  --move-up=<N>      Scroll wheel up by N\n");
                                    printf("  --move-down=<N>    Scroll wheel down by N\n");
                                    printf("  --get, -g          Get current mouse position\n");
                                    printf("  -h, --help         Show this help\n");
                                }

                                int main(int argc, char* argv[]) {
                                    int  x = 0,  y = 0;
                                    int  is_rel_x = 0, is_rel_y = 0;
                                    int  has_x    = 0, has_y    = 0;
                                    int  click_l  = 0, click_r  = 0;
                                    int  click_m  = 0, click_f  = 0, click_b = 0;
                                    int  move_up  = 0, move_down = 0;
                                    int  get_pos  = 0;
                                    int  invalid_arg = 0;

                                    if (argc == 1) {
                                        print_usage(argv[0]);
                                        return 0;
                                    }

                                    for (int i = 1; i < argc; ++i) {
                                        if (strcmp(argv[i], "-h") == 0 ||
                                            strcmp(argv[i], "--help") == 0) {
                                            print_usage(argv[0]);
                                        return 0;
                                            }
                                            else if (strcmp(argv[i], "--get") == 0 ||
                                                strcmp(argv[i], "-g")    == 0) {
                                                get_pos = 1;
                                                }
                                                else if (strcmp(argv[i], "--click-left")   == 0) click_l = 1;
                                                else if (strcmp(argv[i], "--click-right")  == 0) click_r = 1;
                                                else if (strcmp(argv[i], "--click-middle") == 0) click_m = 1;
                                                else if (strcmp(argv[i], "--click-forward")== 0) click_f = 1;
                                                else if (strcmp(argv[i], "--click-back")   == 0) click_b = 1;
                                                else if (strstr(argv[i], "--move-up=")    == argv[i])
                                                    move_up   = atoi(argv[i] + strlen("--move-up="));
                                        else if (strstr(argv[i], "--move-down=")  == argv[i])
                                            move_down = atoi(argv[i] + strlen("--move-down="));
                                        else if (strstr(argv[i], "-x=") == argv[i]) {
                                            x = parse_value(argv[i], "-x=", x, &is_rel_x, &has_x);
                                        }
                                        else if (strstr(argv[i], "-y=") == argv[i]) {
                                            y = parse_value(argv[i], "-y=", y, &is_rel_y, &has_y);
                                        }
                                        else {
                                            invalid_arg = 1;
                                        }
                                    }

                                    if (invalid_arg) {
                                        print_usage(argv[0]);
                                        return 0;
                                    }

                                    if (get_pos) {
                                        int cx, cy;
                                        get_mouse_position(&cx, &cy);
                                        printf("%d %d\n", cx, cy);
                                        return 0;
                                    }

                                    if (has_x || has_y) {
                                        move_mouse(x, y, is_rel_x, is_rel_y, has_x, has_y);
                                    }

                                    if (move_up || move_down) {
                                        scroll_mouse(move_up, move_down);
                                    }

                                    if (click_l || click_r || click_m || click_f || click_b) {
                                        click_mouse(click_l, click_r, click_m, click_f, click_b);
                                    }

                                    return 0;
                                }
