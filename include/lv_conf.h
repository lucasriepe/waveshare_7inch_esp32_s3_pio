/**
 * @file lv_conf.h
 * LVGL v9.5.0 Konfiguration (Werte entsprechen den LVGL-Defaults).
 */
#ifndef LV_CONF_H
#define LV_CONF_H

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16

/*=========================
   STDLIB WRAPPER SETTINGS
 *=========================*/
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_MEM_SIZE (64 * 1024U)

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DEF_REFR_PERIOD  33
#define LV_DPI_DEF 130

/*=================
 * OPERATING SYSTEM
 *=================*/
#define LV_USE_OS   LV_OS_NONE

/*========================
 * RENDERING CONFIGURATION
 *========================*/
#define LV_DRAW_BUF_STRIDE_ALIGN                1
#define LV_DRAW_BUF_ALIGN                       4
#define LV_USE_DRAW_SW 1
#define LV_DRAW_SW_DRAW_UNIT_CNT    1
#define LV_DRAW_SW_COMPLEX          1

/*=====================
 * LOG CONFIGURATION
 *=====================*/
#define LV_USE_LOG 0

/*====================
 * ASSERTS
 *====================*/
#define LV_USE_ASSERT_NULL          1
#define LV_USE_ASSERT_MALLOC        1

/*==================
 * FONTS
 *==================*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*=====================
 * THEMES
 *=====================*/
#define LV_USE_THEME_DEFAULT 1

/*================
 * WIDGETS
 *================*/
#define LV_USE_BAR 1
#define LV_USE_BUTTON 1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CHECKBOX 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMAGE 1
#define LV_USE_LABEL 1
#define LV_USE_OBJECT 1
#define LV_USE_SWITCH 1

/*==================
 * DEVICES
 *==================*/
#define LV_USE_INDEV 1
#define LV_USE_TOUCHSCREEN 1

/*===================
 * EXTRAS
 *===================*/
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*================
 * MISCELLANEOUS
 *================*/
#define LV_USE_OBSERVER 1
#define LV_USE_BINDLOCK 1

#endif /*LV_CONF_H*/
