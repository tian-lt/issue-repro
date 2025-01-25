#pragma once

// std headers
#include <unordered_map>
#include <functional>
#include <string>
#include <exception>

// windows headers
#include <Windows.h>
#pragma comment(linker, "/manifestdependency:\"type='win32' "                  \
                        "name='Microsoft.Windows.Common-Controls' "            \
                        "version='6.0.0.0' "                                   \
                        "processorArchitecture='*' "                           \
                        "publicKeyToken='6595b64144ccf1df' "                   \
                        "language='*' "                                        \
                        "\"")
#pragma comment(lib, "WindowsApp.lib")

// ctrl headers
#include <Richedit.h>

namespace {

  constexpr auto WindowClassName = TEXT("RTFINSP_APP_CLASS");
  template <class T>
  void check_bool(T&& value) {
    if (static_cast<bool>(std::forward<T>(value)) == false) { std::terminate(); }
  }

  int ScalePx(int value) {
    auto dpi = GetDpiForSystem();
    return MulDiv(value, (int)dpi, USER_DEFAULT_SCREEN_DPI);
  }

} // namespace

class AppWindow {
private:
  AppWindow() = default;

public:
  static constexpr WORD UserCommandBase = 1000;
  static constexpr DPI_AWARENESS_CONTEXT DpiContext = DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2;

  void Run() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  static AppWindow Create(HINSTANCE hinst, int cmdshow) {
    AppWindow wnd;
    static auto clss = RegisterWindowClass(hinst);
    check_bool(clss);
    wnd.hwnd_ = CreateWindowEx(
      0, WindowClassName, TEXT("RTF Inspector"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, ScalePx(800), ScalePx(800), nullptr, nullptr, hinst, &wnd);
    check_bool(wnd.hwnd_);
    wnd.CreateBuiltinControls();
    UpdateWindow(wnd.hwnd_);
    ShowWindow(wnd.hwnd_, cmdshow);
    return wnd;
  }

  void UpdateCounter(int value) {
    SetWindowTextW(txtCounter_, std::to_wstring(value).c_str());
  }

  void UpdateText(const std::wstring& text) {
    SetWindowTextW(txtCounter_, text.c_str());
  }

  template <class F> void CreateCommand(WORD cmd, LPCTSTR displayName, F&& f) {
    check_bool(CreateWindow(
      TEXT("BUTTON"), displayName,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10,
      usrCtrlOffset_, 200, 30, hwnd_, (HMENU)cmd,
      (HINSTANCE)GetWindowLongPtr(hwnd_, GWLP_HINSTANCE), nullptr));
    cmds_[cmd] = [f = std::forward<F>(f)](AppWindow* wnd) { f(wnd); };
    usrCtrlOffset_ += 50;
  }

  HWND Handle() const noexcept { return hwnd_; }

private:
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam,
    LPARAM lparam) {
    switch (msg) {
    case WM_COMMAND:
      if (DispatchCommand(hwnd, LOWORD(wparam))) {
        return 0;
      }
      break;
    case WM_CREATE: {
      auto* creation = reinterpret_cast<CREATESTRUCT*>(lparam);
      SetWindowLongPtr(hwnd, GWLP_USERDATA,
        reinterpret_cast<LONG_PTR>(creation->lpCreateParams));
      return 0;
    }
    case WM_SIZE:
      GetThis(hwnd)->UpdateLayout();
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  static ATOM RegisterWindowClass(HINSTANCE hinst) {
    check_bool(LoadLibrary(TEXT("Msftedit.dll")));
    WNDCLASSEX wcex = { .cbSize = sizeof(WNDCLASSEX),
                       .style = CS_SAVEBITS | CS_DROPSHADOW | CS_HREDRAW |
                                CS_VREDRAW,
                       .lpfnWndProc = WndProc,
                       .hInstance = hinst,
                       .hCursor = LoadCursor(nullptr, IDC_ARROW),
                       .hbrBackground = (HBRUSH)COLOR_WINDOW,
                       .lpszClassName = WindowClassName };
    return RegisterClassEx(&wcex);
  }

  static bool DispatchCommand(HWND hwnd, WORD cmd) {
    auto wnd = GetThis(hwnd);
    auto entry = wnd->cmds_.find(cmd);
    if (entry != wnd->cmds_.cend()) {
      entry->second(wnd);
      return true;
    }
    return false;
  }

  static AppWindow* GetThis(HWND hwnd) {
    return reinterpret_cast<AppWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  void CreateBuiltinControls() {
    auto hinst = (HINSTANCE)GetWindowLongPtr(hwnd_, GWLP_HINSTANCE);
    txtCounter_ = CreateWindow(
      TEXT("STATIC"), TEXT("0"), WS_VISIBLE | WS_CHILD | SS_LEFT, 10, 10, 460,
      30, hwnd_, nullptr, hinst,
      nullptr);
    check_bool(txtCounter_);
    redit_ = CreateWindowEx(0, MSFTEDIT_CLASS, nullptr,
      ES_MULTILINE | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VSCROLL,
      0, 0, 0, 0,
      hwnd_, NULL, hinst, NULL);
    check_bool(redit_);
    edit_ = CreateWindowEx(0, TEXT("EDIT"), nullptr, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
      ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, 0, 0, hwnd_, nullptr, hinst, nullptr);
    check_bool(edit_);
    UpdateLayout();
  }

  void UpdateLayout() {
    RECT clientRc;
    check_bool(GetClientRect(hwnd_, &clientRc));
    auto totalHeight = clientRc.bottom - clientRc.top;
    auto totalWidth = clientRc.right - clientRc.left;

    auto row1 = 46;
    totalHeight -= row1; // row #1 for the static text area
    SetWindowPos(redit_, nullptr, 0, row1, totalWidth, totalHeight / 2, SWP_NOZORDER);
    SetWindowPos(edit_, nullptr, 0, totalHeight / 2 + row1, totalWidth, totalHeight / 2, SWP_NOZORDER);
  }

private:
  std::unordered_map<WORD, std::function<void(AppWindow*)>> cmds_;
  HWND hwnd_ = nullptr;
  HWND edit_ = nullptr;
  HWND redit_ = nullptr;
  HWND txtCounter_ = nullptr;
  int usrCtrlOffset_ = 50;
};
