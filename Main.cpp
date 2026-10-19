#include "Window.h"

int main() {
    Window win("Black hole simulation", 700, 500);

    while(win.is_open()) {
        win.poll_events();
        win.clear(5,0,10);
        win.present();
    }

    return 0;
}