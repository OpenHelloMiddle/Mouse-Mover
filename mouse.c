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
#include <unistd.h>
#define OS_LINUX
#endif

int parse_value(const char* arg, const char* prefix, int current, int* is_relative, int* has_value) {
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

    if (isdigit(value_str[0]) || (value_str[0] == '-' && isdigit(value_str[1]))) {
        *is_relative = 0;
        return atoi(value_str);
    }

    *has_value = 0;
    return current;
}

void get_screen_size(int* width, int* height) {
    #if defined(OS_WINDOWS)
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
    #elif defined(OS_MACOS)
    CGRect mainDisplayBounds = CGDisplayBounds(CGMainDisplayID());
    *width = (int)mainDisplayBounds.size.width;
    *height = (int)mainDisplayBounds.size.height;
    #elif defined(OS_LINUX)
    Display* display = XOpenDisplay(NULL);
    if (display) {
        *width = DisplayWidth(display, 0);
        *height = DisplayHeight(display, 0);
        XCloseDisplay(display);
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
    *x = pt.x;
    *y = pt.y;

    #elif defined(OS_MACOS)
    CGEventRef event = CGEventCreate(NULL);
    CGPoint point = CGEventGetLocation(event);
    CFRelease(event);
    *x = (int)point.x;
    *y = (int)point.y;

    #elif defined(OS_LINUX)
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        *x = 0;
        *y = 0;
        return;
    }

    Window root, child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;

    if (XQueryPointer(display, DefaultRootWindow(display),
        &root, &child, &root_x, &root_y,
        &win_x, &win_y, &mask)) {
        *x = root_x;
    *y = root_y;
        }

        XCloseDisplay(display);
        #endif
}

void move_mouse(int x, int y, int is_relative_x, int is_relative_y, int has_x, int has_y) {
    int cur_x, cur_y;

    get_mouse_position(&cur_x, &cur_y);

    int new_x = has_x ? (is_relative_x ? cur_x + x : x) : cur_x;
    int new_y = has_y ? (is_relative_y ? cur_y + y : y) : cur_y;

    int width, height;
    get_screen_size(&width, &height);
    if (new_x < 0) new_x = 0;
    if (new_y < 0) new_y = 0;
    if (new_x >= width) new_x = width - 1;
    if (new_y >= height) new_y = height - 1;

    #if defined(OS_WINDOWS)
    SetCursorPos(new_x, new_y);

    #elif defined(OS_MACOS)
    CGEventRef move = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved,
                                              CGPointMake(new_x, new_y),
                                              kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, move);
    CFRelease(move);

    #elif defined(OS_LINUX)
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        return;
    }

    XTestFakeMotionEvent(display, 0, new_x, new_y, CurrentTime);
    XFlush(display);
    XCloseDisplay(display);
    #endif
}

void print_usage(const char* program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  -x=[value]    Move mouse on X axis. Value can be:\n");
    printf("                + : Move right 1 pixel\n");
    printf("                - : Move left 1 pixel\n");
    printf("                +N : Move right N pixels\n");
    printf("                -N : Move left N pixels\n");
    printf("                N : Move to absolute position N\n");
    printf("                (empty) : Don't move on X axis\n");
    printf("  -y=[value]    Move mouse on Y axis. Same values as -x\n");
    printf("  -h, --help    Show this help message\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -x=+ -y=+        # Move right and down 1 pixel\n", program_name);
    printf("  %s -x=+100 -y=-50   # Move right 100 pixels, up 50 pixels\n", program_name);
    printf("  %s -x=100 -y=200    # Move to absolute position (100,200)\n", program_name);
    printf("  %s -x=- -y=+        # Move left 1 pixel, down 1 pixel\n", program_name);
    printf("  %s -x= -y=100       # X doesn't move, Y moves to 100\n", program_name);
}

int main(int argc, char* argv[]) {
    int x = 0, y = 0;
    int is_relative_x = 0, is_relative_y = 0;
    int has_x = 0, has_y = 0;
    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_usage(argv[0]);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        x = parse_value(argv[i], "-x=", x, &is_relative_x, &has_x);
        y = parse_value(argv[i], "-y=", y, &is_relative_y, &has_y);
    }

    move_mouse(x, y, is_relative_x, is_relative_y, has_x, has_y);

    return 0;
}
