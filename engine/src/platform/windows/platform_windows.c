#include "engine/platform/platform.h"
#include "engine/platform/gl_context.h"
#include "engine/core/log.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>

static const char* TAG = "platform_win32";
static const char* NDS_WNDCLASS_NAME = "NDSRuntimeWindowClass";

static HINSTANCE g_hinstance = NULL;
static HWND g_hwnd = NULL;
static HDC g_hdc = NULL;
static HGLRC g_gl_context = NULL;
static int g_quit_requested = 0;
static int g_key_state[PLATFORM_KEY_COUNT];
static int g_mouse_x = 0;
static int g_mouse_y = 0;
static int g_mouse_buttons[3];
static LARGE_INTEGER g_perf_frequency;

static platform_key win32_vk_to_platform_key(int vk)
{
    switch (vk) {
        case VK_ESCAPE: return PLATFORM_KEY_ESCAPE;
        case 'W': return PLATFORM_KEY_W;
        case 'A': return PLATFORM_KEY_A;
        case 'S': return PLATFORM_KEY_S;
        case 'D': return PLATFORM_KEY_D;
        case VK_SPACE: return PLATFORM_KEY_SPACE;
        default: return PLATFORM_KEY_UNKNOWN;
    }
}

static LRESULT CALLBACK nds_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
        case WM_CLOSE:
            g_quit_requested = 1;
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        case WM_KEYUP: {
            platform_key key = win32_vk_to_platform_key((int)wparam);
            if (key != PLATFORM_KEY_UNKNOWN) {
                g_key_state[key] = (msg == WM_KEYDOWN) ? 1 : 0;
            }
            return 0;
        }
        case WM_MOUSEMOVE:
            g_mouse_x = GET_X_LPARAM(lparam);
            g_mouse_y = GET_Y_LPARAM(lparam);
            return 0;
        case WM_LBUTTONDOWN: g_mouse_buttons[0] = 1; return 0;
        case WM_LBUTTONUP: g_mouse_buttons[0] = 0; return 0;
        case WM_RBUTTONDOWN: g_mouse_buttons[1] = 1; return 0;
        case WM_RBUTTONUP: g_mouse_buttons[1] = 0; return 0;
        case WM_MBUTTONDOWN: g_mouse_buttons[2] = 1; return 0;
        case WM_MBUTTONUP: g_mouse_buttons[2] = 0; return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (!g_gl_context) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                HBRUSH brush = CreateSolidBrush(RGB(24, 28, 33));
                FillRect(hdc, &rc, brush);
                DeleteObject(brush);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

nds_result platform_init(void)
{
    g_hinstance = GetModuleHandle(NULL);
    g_quit_requested = 0;
    memset(g_key_state, 0, sizeof(g_key_state));
    memset(g_mouse_buttons, 0, sizeof(g_mouse_buttons));
    if (!QueryPerformanceFrequency(&g_perf_frequency)) {
        NDS_LOGE(TAG, "QueryPerformanceFrequency failed");
        return NDS_ERR_INIT_FAILED;
    }

    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = nds_wnd_proc;
    wc.hInstance = g_hinstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = NDS_WNDCLASS_NAME;
    if (!RegisterClassExA(&wc)) {
        NDS_LOGE(TAG, "RegisterClassExA failed (%lu)", GetLastError());
        return NDS_ERR_INIT_FAILED;
    }
    NDS_LOGI(TAG, "platform_init ok");
    return NDS_OK;
}

void platform_shutdown(void)
{
    platform_gl_context_destroy();
    UnregisterClassA(NDS_WNDCLASS_NAME, g_hinstance);
    NDS_LOGI(TAG, "platform_shutdown ok");
}

nds_result platform_create_window(const platform_window_desc* desc)
{
    if (!desc) return NDS_ERR_INVALID_ARG;
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!desc->resizable) style &= ~(DWORD)(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT rect = {0, 0, desc->width, desc->height};
    AdjustWindowRect(&rect, style, FALSE);

    g_hwnd = CreateWindowExA(0, NDS_WNDCLASS_NAME,
        desc->title ? desc->title : "NDS Runtime", style,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left,
        rect.bottom - rect.top, NULL, NULL, g_hinstance, NULL);
    if (!g_hwnd) {
        NDS_LOGE(TAG, "CreateWindowExA failed (%lu)", GetLastError());
        return NDS_ERR_INIT_FAILED;
    }
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);
    g_hdc = GetDC(g_hwnd);
    if (!g_hdc) {
        NDS_LOGE(TAG, "GetDC failed");
        platform_destroy_window();
        return NDS_ERR_INIT_FAILED;
    }
    return NDS_OK;
}

void platform_destroy_window(void)
{
    platform_gl_context_destroy();
    if (g_hdc && g_hwnd) {
        ReleaseDC(g_hwnd, g_hdc);
        g_hdc = NULL;
    }
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = NULL;
    }
}

void platform_get_window_size(int* out_width, int* out_height)
{
    if (!g_hwnd) {
        if (out_width) *out_width = 0;
        if (out_height) *out_height = 0;
        return;
    }
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    if (out_width) *out_width = rc.right - rc.left;
    if (out_height) *out_height = rc.bottom - rc.top;
}

void platform_present(void)
{
    platform_gl_swap_buffers();
}

int platform_poll_events(void)
{
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) g_quit_requested = 1;
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return g_quit_requested;
}

int platform_quit_requested(void) { return g_quit_requested; }
void platform_request_quit(void) { g_quit_requested = 1; }
int platform_is_key_down(platform_key key) {
    if (key < 0 || key >= PLATFORM_KEY_COUNT) return 0;
    return g_key_state[key];
}
void platform_get_mouse_position(int* out_x, int* out_y) {
    if (out_x) *out_x = g_mouse_x;
    if (out_y) *out_y = g_mouse_y;
}
int platform_is_mouse_button_down(int button_index) {
    if (button_index < 0 || button_index >= 3) return 0;
    return g_mouse_buttons[button_index];
}

platform_file* platform_file_open(const char* path, const char* mode) { return (platform_file*)fopen(path, mode); }
size_t platform_file_read(platform_file* file, void* buffer, size_t size) { return file ? fread(buffer, 1, size, (FILE*)file) : 0; }
size_t platform_file_write(platform_file* file, const void* buffer, size_t size) { return file ? fwrite(buffer, 1, size, (FILE*)file) : 0; }
long platform_file_size(platform_file* file) {
    if (!file) return -1;
    FILE* f = (FILE*)file;
    long current = ftell(f);
    if (current < 0 || fseek(f, 0, SEEK_END) != 0) return -1;
    long size = ftell(f);
    fseek(f, current, SEEK_SET);
    return size;
}
void platform_file_close(platform_file* file) { if (file) fclose((FILE*)file); }
int platform_file_exists(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
}
uint64_t platform_time_now_ns(void) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)((double)counter.QuadPart / (double)g_perf_frequency.QuadPart * 1000000000.0);
}
void platform_sleep_ms(uint32_t milliseconds) { Sleep(milliseconds); }

nds_result platform_gl_context_create(void)
{
    if (!g_hdc) return NDS_ERR_INIT_FAILED;
    if (g_gl_context) return NDS_OK;

    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cAlphaBits = 8;

    int format = ChoosePixelFormat(g_hdc, &pfd);
    if (!format || !SetPixelFormat(g_hdc, format, &pfd)) {
        NDS_LOGE(TAG, "failed to configure OpenGL pixel format");
        return NDS_ERR_INIT_FAILED;
    }
    g_gl_context = wglCreateContext(g_hdc);
    if (!g_gl_context || !wglMakeCurrent(g_hdc, g_gl_context)) {
        NDS_LOGE(TAG, "failed to create/make current OpenGL context");
        if (g_gl_context) { wglDeleteContext(g_gl_context); g_gl_context = NULL; }
        return NDS_ERR_INIT_FAILED;
    }
    NDS_LOGI(TAG, "OpenGL context created: %s", (const char*)glGetString(GL_VERSION));
    return NDS_OK;
}

void platform_gl_context_destroy(void)
{
    if (!g_gl_context) return;
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_gl_context);
    g_gl_context = NULL;
}

nds_result platform_gl_context_make_current(void)
{
    if (!g_hdc || !g_gl_context) return NDS_ERR_INIT_FAILED;
    return wglMakeCurrent(g_hdc, g_gl_context) ? NDS_OK : NDS_ERR_INIT_FAILED;
}

void platform_gl_swap_buffers(void)
{
    if (g_hdc && g_gl_context) SwapBuffers(g_hdc);
}

void* platform_gl_get_proc_address(const char* name)
{
    if (!name) return NULL;
    PROC proc = wglGetProcAddress(name);
    if (proc) return (void*)proc;
    HMODULE module = GetModuleHandleA("opengl32.dll");
    return module ? (void*)GetProcAddress(module, name) : NULL;
}

nds_result platform_audio_init(void) { return NDS_ERR_NOT_IMPLEMENTED; }
void platform_audio_shutdown(void) {}
nds_result platform_socket_init(void) { return NDS_ERR_NOT_IMPLEMENTED; }
void platform_socket_shutdown(void) {}
