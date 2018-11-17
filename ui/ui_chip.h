#pragma once
/*#
    # ui_chip.h

    A generic micro-chip renderer using Dear ImGui

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    You need to include the following headers before including the
    *implementation*:

        - imgui.h

    All strings provided to ui_chip_init() must remain alive until
    ui_chip_discard() is called!

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_CHIP_MAX_PINS (64)

/* a pin or pingroup description */
typedef struct {
    const char* name;           /* the pin's name */
    int slot;                   /* the pin "slot index", 0 is top-left */
    uint64_t mask;              /* the pin mask when this pin "lights up" */
} ui_chip_pin_t;

/* init params for ui_chip_init() */
typedef struct {
    const char* name;           /* the chip's name */
    int num_slots;              /* the number of pin slots */
    ui_chip_pin_t pins[UI_CHIP_MAX_PINS];   /* the pin descriptions */
} ui_chip_desc_t;

/* chip visualization state */
typedef struct {
    const char* name;
    uint32_t line_color;
    uint32_t text_color;
    uint32_t pin_color;
    int num_slots;
    ui_chip_pin_t pins[UI_CHIP_MAX_PINS];   
} ui_chip_t;

void ui_chip_init(ui_chip_t* chip, ui_chip_desc_t* desc);
void ui_chip_draw(ui_chip_t* chip, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif

/*-- IMPLEMENTATION (include in C++ source) ----------------------------------*/
#ifdef CHIPS_IMPL
#ifndef __cplusplus
#error "implementation must be compiled as C++"
#endif
#include <string.h> /* memset */
#include <stdio.h>  /* sscanf, sprintf (ImGui memory editor) */
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

void ui_chip_init(ui_chip_t* c, ui_chip_desc_t* desc) {
    CHIPS_ASSERT(c && desc);
    CHIPS_ASSERT(desc->name && desc->num_slots > 0);
    memset(c, 0, sizeof(ui_chip_t));
    c->name = desc->name;
    c->line_color = 0xFFFFFFFF;
    c->text_color = 0xFFFFFFFF;
    c->pin_color = 0xFF00FF00;
    c->num_slots = desc->num_slots;
    for (int i = 0; i < UI_CHIP_MAX_PINS; i++) {
        c->pins[i] = desc->pins[i];
    }
}

void ui_chip_draw(ui_chip_t* c, uint64_t pins) {
    ImDrawList* l = ImGui::GetWindowDrawList();
    const ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    const ImVec2 canvas_area = ImGui::GetContentRegionAvail();
    const float w = 64.0f;
    const float h = (c->num_slots / 2) * 16.0f;
    const float x0 = canvas_pos.x + (canvas_area.x * 0.5f) - (w * 0.5f);
    const float y0 = canvas_pos.y + (canvas_area.y * 0.5f) - (h * 0.5f);
    const float x1 = x0 + w;
    const float y1 = y0 + h;
    const float xm = (x0 + x1) / 2;
    const float ym = (y0 + y1) / 2;
    const float slot_height = 16.0f;
    const float pw = 12.0f;
    const float ph = 12.0f;

    l->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), c->line_color);
    ImVec2 ts = ImGui::CalcTextSize(c->name);
    l->AddText(ImVec2(xm-(ts.x/2), ym-(ts.y/2)), c->text_color, c->name);

    float px, py, tx, ty;
    for (int i = 0; i < c->num_slots; i++) {
        const ui_chip_pin_t* pin = &c->pins[i];
        if (!pin->name) {
            break;
        }
        const int slot = pin->slot;
        py = y0 + slot_height*0.5f - ph*0.5f;
        ts = ImGui::CalcTextSize(pin->name);
        if (slot < (c->num_slots / 2)) {
            /* left side */
            py += slot * slot_height;
            px = x0 - pw;
            tx = px - ts.x - 4;
        }
        else {
            /* right side */
            py += (slot - (c->num_slots/2)) * slot_height;
            px = x1;
            tx = px + pw + 4;
        }
        ty = py + (ph * 0.5f) - (ts.y * 0.5f);
        if (pins & pin->mask) {
            l->AddRectFilled(ImVec2(px, py), ImVec2(px+pw, py+ph), c->pin_color);
        }
        l->AddRect(ImVec2(px, py), ImVec2(px+pw, py+ph), c->line_color);
        l->AddText(ImVec2(tx, ty), c->text_color, pin->name);
    }
}

#endif /* CHIPS_IMPL */
