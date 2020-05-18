#include "keyboard.h"

void Keyboard::handle_keyup(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_q:
       held.q = false;
        break;
    case SDLK_w:
        held.w = false;
        break;
    case SDLK_e:
        held.e = false;
        break;
    case SDLK_a:
        held.a = false;
        break;
    case SDLK_s:
        held.s = false;
        break;
    case SDLK_d:
        held.d = false;
        break;
    case SDLK_i:
        held.i = false;
        break;
    case SDLK_k:
        held.k = false;
        break;
    case SDLK_SPACE:
        held.space = false;
        break;
    case SDLK_BACKQUOTE: // '`'
        held.tilde = false;
        break;
    case SDLK_RETURN:
        held.enter = false;
        break;
    }
}

void Keyboard::handle_keydown(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_q:
        down.q = !held.q;   // ignore key repeats
        held.q = true;
        break;
    case SDLK_w:
        down.w = !held.w;
        held.w = true;
        break;
    case SDLK_e:
        down.e = !held.e;
        held.e = true;
        break;
    case SDLK_a:
        down.a = !held.a;
        held.a = true;
        break;
    case SDLK_s:
        down.s = !held.s;
        held.s = true;
        break;
    case SDLK_d:
        down.d = !held.d;
        held.d = true;
        break;
    case SDLK_i:
        down.i = !held.i;
        held.i = true;
        break;
    case SDLK_k:
        down.k = !held.k;
        held.k = true;
        break;
    case SDLK_SPACE:
        down.space = !held.space;
        held.space = true;
        break;
    case SDLK_BACKQUOTE: // '`'
        down.tilde = !held.tilde;
        held.tilde = true;
        break;
    case SDLK_RETURN:
        down.enter = !held.enter;
        held.enter = true;
        break;
    }
}

void Keyboard::clear_keydowns()
{
    down = KeyboardData();
}
