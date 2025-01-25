#include "AppWindow.h"

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int cmdshow) {
  auto window = AppWindow::Create(hinst, cmdshow);
  window.Run();
}
