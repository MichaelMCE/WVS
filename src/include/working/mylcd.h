
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer and text rendering API
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef _MYLCD_H_
#define _MYLCD_H_

#include "mylcdconfig.h"


#define libmylcdVERSIONmj	0.60
#define libmylcdVERSIONmi	0
#define libmylcdVERSION		"0.60.0"
#define mySELF				"Michael McElligott"

// draw pixel styles
#define LSP_CLEAR			0
#define LSP_SET				1
#define LSP_XOR				2
#define LSP_OR				3
#define LSP_AND				4

// frame size
#define LFRM_BPP_1			0		// 8 pixels per byte
#define LFRM_BPP_8			1		// 8bit RGB332. 1 byte per pixel
#define LFRM_BPP_12			2		// 12bit RGB444. 2 bytes per pixel, 4 bits unused
#define LFRM_BPP_15			3		// 16bit RGB555. 2 bytes per pixel, 1 bit unused
#define LFRM_BPP_16			4		// 16bit RGB565. 2 bytes per pixel
#define LFRM_BPP_24			5		// 24bit RGB888. 3 bytes per pixel
#define LFRM_BPP_32			6		// 24bit+Alpha RGBA8888. 4 bytes per pixel
#define LFRM_TYPES			(LFRM_BPP_32+1)	// number of BPP formats available

// use with lGetRGBMask()
#define LMASK_RED			0
#define LMASK_GREEN			1
#define LMASK_BLUE			2
#define LMASK_WHITE			3
#define LMASK_BLACK			4
#define LMASK_MAGENTA		5
#define LMASK_YELLOW		6
#define LMASK_CYAN			7

// copy modes for lCopyAreaScaled(), lCopyFrame() and lCopyAreaEx()
#define LCASS_CLEAR			0
#define LCASS_CPY			1
#define LCASS_XOR			2
#define LCASS_OR			3
#define LCASS_AND			4
#define LCASS_NXOR			5
#define LCASS_NOT			6

// text printing styles
#define LPRT_CLEAR			LCASS_CLEAR
#define LPRT_CPY			LCASS_CPY
#define LPRT_OR				LCASS_OR
#define LPRT_AND			LCASS_AND
#define LPRT_XOR			LCASS_XOR
#define LPRT_NOT			LCASS_NOT

// glyph rendering orientation, some modes are not suitable with all fonts
#define LTR_0				0
#define LTR_90				1
#define LTR_90VFLIP			2
#define LTR_180				5
#define LTR_270				3
#define LTR_270VFLIP		4
#define LTR_HFLIP			6
#define LTR_VFLIP			7
#define LTR_DEFAULT			LTR_0

// lDrawMask mode
#define LMASK_CLEAR			0
#define LMASK_OR			1
#define LMASK_XOR			2
#define LMASK_AND			3
#define LMASK_CPYSRC		4
#define LMASK_CPYMASK		5

// lMoveArea mode
#define LMOV_CLEAR			0		// clear all
#define LMOV_SET			1		// set all
#define LMOV_LOOP			2		// loop back to beginning
#define LMOV_BIN			3		// dump pixels

// scroll flags
#define SCR_LOOP			0x01	// set scroller to loop back mode.
#define SCR_AUTOREFRESH		0x02	// set scroller to auto lRefresh() directly on each lUpdateScroll() call
#define SCR_LEFT			1		// direction of scroll - left
#define SCR_RIGHT			2		// ^ right
#define SCR_UP				3		// ^ up
#define SCR_DOWN			4		// ^ down.

//image file types
#define IMG_RAW				0
#define IMG_JPG				1		// read only
#define IMG_PNG				2
#define IMG_BMP				3
#define IMG_TGA				4
#define IMG_PGM				5		// 1bpp only

// flip frame direction
#define FF_HORIZONTAL		0x01
#define FF_VERTICAL			0x02
#define FF_VERTANDHORIZ		0x04	// diagonally

// driver status
#define LDRV_CLOSED			0x00	// is not opened
#define LDRV_READY			0x01	// is open
#define LDRV_DISENGAGED		0x02	// display is opened but paused

#define LDRV_DISPLAY		1		// driver enumeration mode
#define LDRV_PORT			2

// progress bar border style
#define PB_BORDER_NONE		0x0100	// see progressbar.c or wa.c for more details
#define PB_BORDER_HBOX		0x0200
#define PB_BORDER_VBOX		0x0300
#define PB_BORDER_HLINE		0x0400
#define PB_BORDER_VLINE		0x0500
// progress bar indicator style
#define PB_MARKER_NONE		0x0001
#define PB_MARKER_HFILL		0x0002
#define PB_MARKER_VFILL		0x0003
#define PB_MARKER_HLINE		0x0004
#define PB_MARKER_VLINE		0x0005
#define PB_MARKER_HBAR		0x0006
#define PB_MARKER_VBAR		0x0007

// Print rendering flags
#define PF_DONTRENDER		0x000001	// disable rendering to frame
#define PF_CLIPDRAW			0x000002	// clips glyph rendering to the bounding rect
#define PF_CLIPTEXTH		0x000004	// sets whether function will render part of character on or over rect area
#define PF_CLIPTEXTV		0x000008	// ie, render glyph only if fits within frame. PF_CLIPDRAW must be set.
#define PF_CLIPWRAP			0x000010	// wrap text. setting this flag will result in PF_CLIPDRAW being set.
#define PF_NOESCAPE			0x000020	// disable '\n' line escaping
#define PF_NODEFAULTCHAR	0x000040	// do not use font defined default glyph in place of a missing char or unsupported code point
#define PF_FIXEDWIDTH		0x000080	// use fixed width
#define PF_USELASTX			0x000100	// use x end point for next starting point
#define PF_USELASTY			0x000200	// use y end point for next starting point
#define PF_NEWLINE			PF_USELASTY
#define PF_USELASTXY		PF_USELASTX|PF_USELASTY
#define PF_RESETX			0x000400	// reset x start point to bx1 bounding point
#define PF_RESETY			0x000800	// reset y start point to by1 bounding point
#define PF_RESETXY			PF_RESETX|PF_RESETY
#define PF_LEFTJUSTIFY		0x001000	// text is left justified by default
#define PF_MIDDLEJUSTIFY	0x002000	// middle justify line,	requires PF_CLIPWRAP
#define PF_RIGHTJUSTIFY		0x004000	// right justify line,	requires PF_CLIPWRAP
#define PF_DISABLEAUTOWIDTH 0x008000	// disables wide character (bdf) auto width calculation - is overridden by PF_FIXEDWIDTH
#define PF_GLYPHBOUNDINGBOX	0x010000	// enclose each glyph within a rectangle, is not overridden by 'PF_DONTRENDER'
#define PF_TEXTBOUNDINGBOX	0x020000	// draw a dotted border over printed area
#define PF_INVERTTEXTRECT	0x040000	// invert text rectangle
#define PF_GETTEXTBOUNDS	0x080000	// used internally with lGetTextMetricsList()
#define PF_DONTFORMATBUFFER 0x100000	// disable print formatting
#define PF_WORDWRAP			0x200000	// enable word wraping
#define PF_INVERTGLYPH1		0x400000
#define PF_INVERTGLYPH2		0x800000
#define PF_VERTICALLTR		0x1000000
#define PF_VERTICALRTL		0x2000000	// render top down from right to left
#define PF_HORIZONTALRTL	0x4000000	// render from right to left
                      		         	// intended for East Asian (CJK) languages.
#define PF_INVERTTEXT		PF_INVERTTEXTRECT


//		option					  index
#define lOPT_PCD8544_CS				0	// CS_1:CS_2:CS_3.  Set chip select line. See 'pcd8544_sio.h'
#define lOPT_PCD8544_HFLIP			1	// 0:1
#define lOPT_PCD8544_INVERT 		2	// 0:1
#define lOPT_PCD8544_STRUCT			3

#define lOPT_PCF8814_CS				0	// Unused: TODO: Set chip select lines, see 'pcd8814_spi.h' for details
#define lOPT_PCF8814_HFLIP			1	// 0:1
#define lOPT_PCF8814_INVERT			2	// 0:1
#define lOPT_PCF8814_STRUCT			3

#define lOPT_PCF8833_CONTRAST		1	// contrast range: 0-63

#define lOPT_S1D15G10_CONTRAST		1	// contrast range: coarse 0-63, fine 0-7. set via ((coarse<<8)|fine)

#define lOPT_S1D15G14_CONTRAST		1	// contrast range: 0-127

#define lOPT_USB13700EXP_STRUCT		0
#define lOPT_USB13700EXP_FRMLENGTH	1	// frameLength
#define lOPT_USB13700EXP_CLKPOL		2	// clock polarity
#define lOPT_USB13700EXP_CLKPHASE	3	// clock phase
#define lOPT_USB13700EXP_BITCLOCK	4	// bitclock
#define lOPT_USB13700EXP_PRESCALER	5	// prescaler. Bitrate is calculated using formula: 60000000/(prescaler*[bitclock+1])

#define lOPT_USBD480_STRUCT			0
#define lOPT_USBD480_BRIGHTNESS		1	// set backlight brightness level between 0 and 255
#define lOPT_USBD480_TOUCHPOSITION	2	// wait for and get a touch report. this method blocks. use lOPT_USBD480_TOUCHCB for non-blocking
#define lOPT_USBD480_TOUCHCB		3	// setup touch input callback. view examples/touch.c for details
#define lOPT_USBD480_TOUCHCBUSERPTR	4

#define lOPT_SED1565_HFLIP			0	// 0:1

#define lOPT_LEDCARD_MODE			0	// FIXED:SCROLL 0:1

#define lOPT_KS0108_DELAY			0	// delay between writes to controller, use if experiencing corruption. default is 1
										// eg; drv->dd->setOption(drv, lOPT_KS0108_DELAY, 3)
#define lOPT_T6963C_DELAY			0	// as above

#define lOPT_SED1335_DELAY			0	// as above
#define lOPT_SED1335_XTAL			1	// sed1335 crystal speed. Adjusting XTAL speed will restart controller
#define lOPT_SED1335_TLAYER			2	// total layers. defaults to 1. Adjusting TLAYER's will cause a controller restart
#define lOPT_SED1335_ALAYER			3	// active layer. defaults to first layer, ie; layer '0'

#define lOPT_U13700D_STRUCT 		0	// get pointer to internal structure.

#define lOPT_G15_STRUCT				0	// get pointer to TMYLCDG15 struct. g15/g15display.h for details
#define lOPT_G15_SOFTKEYCB			1	// soft button call back, see wa.c or irc.c for an example of use
#define lOPT_G15_PRIORITY			2	// sync/async update mode
#define lOPT_G15_DEVICE				3

#define lOPT_G19_STRUCT				0	// get pointer to TMYLCDG19 struct. g19/g19display.h for details
#define lOPT_G19_SOFTKEYCB			1	// soft button call back, see wa.c or irc.c for an example of use
#define lOPT_G19_PRIORITY			2	// sync/async update mode

#define lOPT_G15LU_STRUCT			0	// get pointer to TMYLCDG15 struct, see irc.c and g15/g15display.h for more details
#define lOPT_G15LU_SOFTKEYCB		1	// soft button call back, see wa.c or irc.c for an example of use
#define lOPT_G15LU_MLEDS			2
#define lOPT_G15LU_KBBRIGHTNESS	 	3
#define lOPT_G15LU_LCDBRIGHTNESS	4
#define lOPT_G15LU_LCDCONTRAST		5

#define lOPT_DDRAW_STRUCT			0	// get pointer to internal structure.
#define lOPT_SDL_STRUCT				0	// as above

#define lOPT_FT245_BAUDRATE			0	// adjust FT245 baudrate
#define lOPT_FT245_DIVISOR			1	// set devisor for non standard baudrates. Refer to FTDI's FT245BM datasheet for details
#define lOPT_FT245_BITBANG			2	// switches bit bang mode on or off (0/1)
#define lOPT_FT245_LATENCY			3	// set receive buffer latency timer from 2ms to 255ms in 1ms intervals
#define lOPT_FT245_IODIR			4	// set direction of the 8bit data bus under bit bang mode. See FT245BM	datasheet for details

#define lOPT_FTDI_STRUCT			0	// TODO:
#define lOPT_FTDI_DEVICE			1	// TODO: usb ftdi device index. 0=first device, 1=second and so on
#define lOPT_FTDI_BAUDRATE			2	// adjust chips baudrate
#define lOPT_FTDI_BITBANG			3	// switches bit bang mode on or off (0/1)
#define lOPT_FTDI_LATENCY			4	// set receive buffer latency timer from 2ms to 255ms in 1ms intervals
#define lOPT_FTDI_IODIR				5	// set direction of the 8bit data bus under bit bang mode. See FT245BM	datasheet for details

#define CAP_STATE_OFF				0
#define CAP_STATE_ON				1

enum _HWCAPS
{
	CAP_NONE = 0,			// unused
	CAP_BACKBUFFER,			// Use back buffer and optimised frame transfers if controller supports partial area updates
	CAP_HTMLCHARREF,		// HTML character and entity references supported, ie; '&#36215;', '&amp;', etc..
	CAP_PNG,				// PNG is supported
	CAP_BMP,				// Bitmap supported
	CAP_JPG,				// Jpeg load supported, write unsupported.
	CAP_TGA,				// TGA supported
	CAP_BDF_FONT,			// BDF fonts supported
	CAP_BITMAP_FONT,		// Image bitmap font routines compiled in
	CAP_PGM,				// PGM supported
	CAP_DRAW,				// Primitive API compiled in
	CAP_ROTATE,				// Rotate primitives
	CAP_FONTS,				// Build font engine
	CAP_CDECODE,			// Build character decoding
	CAP_PIXELEXPORTS,		// Pixel operation exports created. (ie; frm->pixel->set() is available as lSetPixel())
	CAP_APISYNC,			// API Synchronization enabled
	CAP_PTHREADS,			// Using pthreads (pthreadGC2.dll) in place of MS Windows synchronization API
	CAP_PRINT,				// Print API enabled
	CAP_SCROLL,				// Scroll API enabled
	CAP_BIG5,				// Internal Big5 compiled in
	CAP_SJISX0213,			// Japanese supported (compiled in only)
	CAP_HZGB2312,			// HZ-GB2312 support built
	CAP_NULLDISPLAY,		// NULL display is compiled and selectable
	CAP_NULLPORT,			// NULL port driver is compiled
	CAP_DDRAW,				// DirectDraw virtual display is compiled
	CAP_SDL,				// SDL virtual display compiled
	CAP_PCD8544,			// PCD8544 from the Nokia mobile phone series (eg, 3210)
	CAP_PCF8814,			// PCF8514 from the Nokia mobile phone series (eg, 3510 mono)
	CAP_PCF8833,			// PCF8833 support
	CAP_S1D15G10,			// S1D15G10 support
	CAP_S1D15G14,			// Nokia 3510i (colour)
	CAP_SED1565,			// SED1565 controller supported
	CAP_SED1335,			// SED1330/13305/1335/1336 display driver
	CAP_T6963C,				// Toshiba T6963 controller supported
	CAP_KS0108,				// KS0108/HD61203 controller supported
	CAP_G15LIBUSB,			// Display on the Logitech G15 keyboard accessed through libusb.
	CAP_G15DISPLAY,			// Display on the Logitech G15 keyboard.
	CAP_G19DISPLAY,			// Display on the Logitech G19 keyboard.
	CAP_USBD480DLL,			// USBD480 available and accessed through USBD480_lib.dll
	CAP_USBD480LIBUSB,		// USBD480 access through libusb available
	CAP_USBD480LIBUSBHID,	// USBD480 access through libusb and HID (touch input) available
	CAP_USB13700DLL,		// USB13700 through display_lib_USB13700.dll
	CAP_USB13700LIBUSB,		// USB13700 through libusb13700
	CAP_USB13700EXP,		// USB13700 expansion port. port0 for control and port1 for SPI
	CAP_LEDCARD,			// A PIC driven, serially controlled 40x12 LED display
	CAP_WINIO,				// WinIO port driver with winio.dll compiled in, requires winio.sys
	CAP_OPENPARPORT,		// Open ParPort (Win2k only?) supported
	CAP_DLPORTIO,			// DLPortIo LINX driver supported, requires dlportio.dll/sys
	CAP_SERIAL,				// Serial port driver compiled in
	CAP_FT245,				// FTDI FT245BM USB driver. FTDIChip.com's (MS Windows) D2XX driver should be preinstalled before using
	CAP_FTDI,				// FT245BM and other FTDI chips through libFTDI
	CAP_MMX,				// MMX replacement routines for memcpy (and other stuff) compiled in.
	CAP_DEBUG,				// Compiled in debug mode - is not runtime switchable
	CAP_DEBUG_FILEIO,		// ^^ display file open, close read and write requests
	CAP_DEBUG_MEMUSAGE,		// list all calls to free, calloc, realloc, malloc, wcsdup and strdup. refer to memory.h and mylcdmemory.h
	CAPS_TOTAL
};


// languages available
enum _CMT
{
	CMT_NONE = 0,				// don't perform character remapping, straight byte for byte relationship. ie, no conversion takes place
	CMT_UTF8 = 3001,			// UTF8
	CMT_UTF16,					// UTF16 is available only through the 'lxxxList()' API, or through lPrintxx() with PF_DONTFORMATBUFFER set. Uses BOM to determine endian.
	CMT_UTF16LE,				// as above, little endian. BOM is ignored.
	CMT_UTF16BE,				// as above, big endian. BOM is ignored.
	CMT_UTF32,					// UTF32 is available only through the 'lxxxList()' API (ucs4)
	CMT_BIG5,					// Traditional Chinese, Taiwan, Hong Kong SAR (CP950)
	CMT_GBK,					// Simplified Chinese, mainland China (CP936)
	CMT_GB18030,				// Chinese standard similar to GBK, mainland China
	CMT_HZ_GB2312,				// HZ GB.2312.1980
	CMT_EUC_CN,					// Extended Unix Code for Simplified Chinese using GB2312
	CMT_EUC_TW,					// Extended Unix Code for Traditional Chinese
	CMT_EUC_KR,					// Korean, EUC-KR (CP949)
	CMT_ISO2022_KR,				// Korean ISO-2022-KR (KSX1001)
	CMT_ISO2022_JP,				// Japanese ISO-2022-JP, single/double byte encoding
	CMT_JISX0213,				// Shift JIS, X 0213 planes 1 and 2
	CMT_JISX0208,				// EUC-JP (JIS-X-0208 code set 1)
	CMT_JISX0201,				// Japanese 8bit standard
	CMT_ISO_2022_JP_EUC_SJIS,	// Japanese encoding - auto detect between ISO-2022-JP, EUC-JP and Shift JIS
	CMT_TIS620,					// 8bit national Thai standard
	CMT_GB1988,					// 7bit Chinese variant of ASCII
	CMT_SYMBOL,
	CMT_DINGBATS,
	CMT_KOI8_R,
	CMT_KOI8_U,
	CMT_CP437,
	CMT_CP737,
	CMT_CP775,
	CMT_CP850,
	CMT_CP852,
	CMT_CP855,
	CMT_CP857,
	CMT_CP860,
	CMT_CP861,
	CMT_CP862,
	CMT_CP863,
	CMT_CP864,
	CMT_CP865,
	CMT_CP866,
	CMT_CP869,
	CMT_CP874,
	CMT_CP1250,
	CMT_CP1251,
	CMT_CP1252,
	CMT_CP1253,
	CMT_CP1254,
	CMT_CP1255,
	CMT_CP1257,
	CMT_CP1258,
	CMT_ISO8859_1,
	CMT_ISO8859_2,		// Latin alphabet no. 2 (Latin-2)
	CMT_ISO8859_3,		// South European (iso-ir-109, Latin-3)
	CMT_ISO8859_4,		// North European (iso-ir-110, Latin-3)
	CMT_ISO8859_5,		// Cyrillic languages (iso-ir-144)
	CMT_ISO8859_6,		// Arabic (RTL (bidi)text is unsupported within myLCD)
	CMT_ISO8859_7,		// Greek (iso-ir-126, ELOT 928)
	CMT_ISO8859_8,		// Hebrew (RTL (bidi) text unsupported within myLCD)
	CMT_ISO8859_9,		// Turkish (iso-ir-148, latin5)
	CMT_ISO8859_10,		// Nordic languages (iso-ir-157, latin6)
	CMT_ISO8859_11,		// Thai (tis620+nbsp)
	CMT_ISO8859_13,		// Baltic languages (Baltic rim, Latin-7)
	CMT_ISO8859_14,		// Celtic languages (iso-ir-199, Latin-8)
	CMT_ISO8859_15,		// (updated table similar to iso-8859-1, includes 'euro', Latin-9)
	CMT_ISO8859_16,		// South-Eastern European, French, Italian and Irish Gaelic (iso-ir-226, Latin-10)

	CMT_UNICODE	= CMT_UTF16,
	CMT_ASCII	= CMT_NONE,
	CMT_ISO8859 = CMT_ISO8859_1,
	CMT_GB2312	= CMT_HZ_GB2312,
	CMT_CP936	= CMT_GBK,
	CMT_CP950	= CMT_BIG5,
	CMT_EUC_JP	= CMT_JISX0208,
	CMT_SHIFTJIS= CMT_JISX0213,
	CMT_AUTO_JP = CMT_ISO_2022_JP_EUC_SJIS
};


// registered BDF fonts
enum _FONTDEFSW
{
	lBaseFontW = 2001,
	LFTW_WENQUANYI9PT = lBaseFontW,
	LFTW_WENQUANYI12PT,
	LFTW_UNICODE,			// UNICODE.bdf contains largest number of glyphs, around 44k
	LFTW_MONAU16,			// http://monafont.sourceforge.net/index-e.html
	LFTW_MONAU14,			// Shift_JIS is best displayed with Mona (MS GOTHIC)
	LFTW_MONA8X16A,
	LFTW_MONA7X14A,
	LFTW_18x18KO,
	LFTW_B24,
	LFTW_B16B,	// 2010
	LFTW_B16,
	LFTW_B14,
	LFTW_B12,
	LFTW_B10,
	LFTW_4x6,
	LFTW_5x7,	// 2016
	LFTW_5x8,
	LFTW_6x8_CP437,
	LFTW_6x9,
	LFTW_6x10,
	LFTW_6x12,
	LFTW_CLR6X12,
	LFTW_6x13,
	LFTW_6x13B,
	LFTW_6x13O,
	LFTW_7x13,
	LFTW_7x13B,
	LFTW_7x13O,
	LFTW_7x14,
	LFTW_7x14B,
	LFTW_8x13,
	LFTW_9x18,	// 2032
	LFTW_9x18B,
	LFTW_10x20,
	LFTW_13x26,
	LFTW_NIMBUS14,
	LFTW_SNAP,		// 2037
	LFTW_ROUGHT18,	// 2038
	LFTW_HELVB18,
	LFTW_COMICSANS20,
	LFTW_HELVR08,
	LFTW_HELVR10,
	LFTW_HELVR12,	// 2043
	LFTW_HELVR14,
	LFTW_HELVR18,
	LFTW_HELVR24,
	LFTW_COURR08,
	LFTW_COURR10,
	LFTW_COURR12,
	LFTW_COURR14,
	LFTW_COURR18,
	LFTW_COURR24,
	LFTW_HERIR08,
	LFTW_HERIR10,
	LFTW_HERIR12,
	LFTW_HERIR14,
	LFTW_HERIR18,
	LFTW_HERIR24,
	LFTW_LUTRS08,
	LFTW_LUTRS10,
	LFTW_LUTRS12,
	LFTW_LUTRS14,
	LFTW_LUTRS18,
	LFTW_LUTRS19,
	LFTW_LUTRS24,
	LFTW_ETL14,
	LFTW_ETL16,
	LFTW_ETL24,
	LFTW_CU12,
	LFTW_CUALT12,
	LFTW_XSYMB1_12,
	LFTW_XSYMB0_12,
	LFTW_NCENR10,
	LFTW_TIMR10,
	LFTW_UTRG__10,
	LFTW_PROOF9X16,
	LFTW_SCREEN8X16,
	LFTW_KOI5X8,
	LFTW_KOI7X14,
	LFTW_KOI9X18,
	LFTW_KOI10X20,
	LFTW_KOI12X24B,
	LFTW_END
};


// registered TGA fonts
enum _FONTDEFSA
{
	lBaseFontA = 1001,
	LFT_SMALLFONTS7X7 = lBaseFontA,
	LFT_LONG6X13,
	LFT_COMICSANSMS7X8,
	LFT_COURIERNEWCE8,
	LFT_COMICSANSMS8X9,
	LFT_GUNGSUHCHE8X8,
	LFT_F3,
	LFT_GEORGIA9,
	LFT_SIMPSONS8,
	LFT_XFILES12,
	LFT_GOTHAMNIGHTS,
	LFT_BLACKADDERII,
	LFT_ARIAL,
	LFT_COURIER10,
	LFT_F0,
	LFT_COMICSANSMS15X13,
	LFT_NTR9,
	LFT_FONT0,
	LFT_ARIALBOLD,
	LFT_COURIER14,
	LFT_ARIALBLACK9X10,
	LFT_ARIALITALIC,
	LFT_BASICFONT,
	LFT_F2,
	LFT_FONT1,
	LFT_QUAKEA,
	LFT_QUAKEB,
	LFT_DOTUMCHE24X24,
	LFT_SIMPSONS14,
	LFT_LANSBURY,
	LFT_SFGRANDEZZA,
	LFT_JAPANESE_KANJI,
	LFT_DRAGONFLY,
	LFT_RODDY,
	LFT_MISSCLUDE,
	LFT_VIT,
	LFT_VITBOLD,
	LFT_BLOCKUP,
	LFT_LIQUIS,
	LFT_CHINATOWN,
	LFT_CHOWFUN,
	LFT_WONTON,
	LFT_SANTAMONL,
	LFT_SANTAMON,
	LFT_MARATHILEKHANI,
	LFT_KORV,
	LFT_XFILES20,
	LFT_CHARMING,
	LFT_GEORGIA24,
	LFT_GEORGIA24I,
	LFT_LUCKY,
	LFT_TRUMANIA,
	LFT_ANGLICANTEXT,
	LFT_COMICSANSMS48X48,
	LFT_GUNGSUHCHE24X36,
	LFT_ARAKAWA,
	LFT_TETRICIDE,
	LFT_COURIERNEWCE56,
	LFT_INTERDIMENSIONAL12,
	LFT_INTERDIMENSIONAL16,
	LFT_INTERDIMENSIONAL18,
	LFT_INTERDIMENSIONAL,
	LFT_PORSCHE911,
	LFT_F1,
	LFT_LDW20,
	LFT_LDW16,
	LFT_LDW10,

#if (__USE_TGAFONTPACK1__)
	LFT_MODEM_16,
	LFT_MODEM_18,
	LFT_MODEM_20,
	LFT_MODEM_24,
	LFT_MODEM_32,
	LFT_MODEM_32B,
	LFT_04B2_10,
	LFT_04B2_16,
	LFT_04B2_20,
	LFT_AKBAR_16,
	LFT_AKBAR_18,
	LFT_AKBAR_20,
	LFT_AKBAR_24,
	LFT_AKBAR_32,
	LFT_ARIAL_16,
	LFT_ARIAL_18,
	LFT_ARIAL_20,
	LFT_ARIAL_24,
	LFT_ARIAL_32,
	LFT_ASIANDINGS_20,
	LFT_ASIANDINGS_24,
	LFT_ASIANDINGS_32,
	LFT_BATANG_16,
	LFT_BATANG_18,
	LFT_BATANG_20,
	LFT_BATANG_24,
	LFT_BATANG_32,
	LFT_BATANGCHE_16,
	LFT_BATANGCHE_18,
	LFT_BATANGCHE_20,
	LFT_BATANGCHE_24,
	LFT_BATANGCHE_32,
	LFT_COMICSANS_16,
	LFT_COMICSANS_18,
	LFT_COMICSANS_20,
	LFT_COMICSANS_24,
	LFT_COMICSANS_32,
	LFT_COURIERNEW_16,
	LFT_COURIERNEW_16B,
	LFT_COURIERNEW_18,
	LFT_COURIERNEW_18B,
	LFT_COURIERNEW_20,
	LFT_COURIERNEW_20B,
	LFT_COURIERNEW_24,
	LFT_COURIERNEW_24B,
	LFT_COURIERNEW_32,
	LFT_COURIERNEW_32B,
	LFT_DOTUM_16,
	LFT_DOTUM_18,
	LFT_DOTUM_20,
	LFT_DOTUM_24,
	LFT_DOTUM_32,
	LFT_DOTUMCHE_16,
	LFT_DOTUMCHE_18,
	LFT_DOTUMCHE_20,
	LFT_DOTUMCHE_24,
	LFT_DOTUMCHE_32,
	LFT_GEORGIA_16,
	LFT_GEORGIA_18,
	LFT_GEORGIA_20,
	LFT_GEORGIA_24,
	LFT_GEORGIA_32,
	LFT_GULIM_16,
	LFT_GULIM_18,
	LFT_GULIM_20,
	LFT_GULIM_24,
	LFT_GULIM_32,
	LFT_GUNGSUH_16,
	LFT_GUNGSUH_18,
	LFT_GUNGSUH_20,
	LFT_GUNGSUH_24,
	LFT_GUNGSUH_32,
	LFT_GUNGSUHCHE_16,
	LFT_GUNGSUHCHE_18,
	LFT_GUNGSUHCHE_20,
	LFT_GUNGSUHCHE_24,
	LFT_GUNGSUHCHE_32,
	LFT_HANSHAND_16,
	LFT_HANSHAND_18,
	LFT_HANSHAND_20,
	LFT_HANSHAND_24,
	LFT_HANSHAND_32,
	LFT_MSGOTHIC_16,
	LFT_MSGOTHIC_18,
	LFT_MSGOTHIC_20,
	LFT_MSGOTHIC_24,
	LFT_MSGOTHIC_32,
	LFT_SHUI_16,
	LFT_SHUI_18,
	LFT_SHUI_20,
	LFT_SHUI_24,
	LFT_SHUI_32,
	LFT_ARIAL_10,
	LFT_ARIAL_10B,
	LFT_ARIAL_12,
	LFT_ARIAL_12B,
	LFT_ICONS_16,
	LFT_ICONS_22,
	LFT_ICONS_32,
	LFT_ICONS2_16,
	LFT_ICONS2_22,
	LFT_ICONS2_32,
	LFT_IMPACT_14,
	LFT_IMPACT_16,
	LFT_IMPACT_18,
	LFT_IMPACT_20,
	LFT_IMPACT_24,
	LFT_IMPACT_32,
	LFT_LINOTYPE_10,
	LFT_LINOTYPE_12,
	LFT_LINOTYPE_14,
	LFT_LINOTYPE_16,
	LFT_LINOTYPE_18,
	LFT_LINOTYPE_20,
	LFT_LINOTYPE_24,
	LFT_LINOTYPE_32,
	LFT_LINOTYPE_8,
	LFT_LUCIDACONSOLE_10,
	LFT_LUCIDACONSOLE_12,
	LFT_LUCIDACONSOLE_14,
	LFT_LUCIDACONSOLE_16,
	LFT_LUCIDACONSOLE_18,
	LFT_LUCIDACONSOLE_20,
	LFT_LUCIDACONSOLE_24,
	LFT_LUCIDACONSOLE_32,
	LFT_LUCIDASANS_10,
	LFT_LUCIDASANS_12,
	LFT_LUCIDASANS_14,
	LFT_LUCIDASANS_16,
	LFT_LUCIDASANS_18,
	LFT_LUCIDASANS_20,
	LFT_LUCIDASANS_24,
	LFT_LUCIDASANS_32,
	LFT_SCRIPT_24,
	LFT_SCRIPT_24B,
	LFT_SCRIPT_32,
	LFT_SMALLFONTS_5,
	LFT_SMALLFONTS_6,
	LFT_SMALLFONTS_7,
	LFT_SYMBOL_10,
	LFT_SYMBOL_12,
	LFT_SYMBOL_14,
	LFT_SYMBOL_16,
	LFT_SYMBOL_18,
	LFT_SYMBOL_20,
	LFT_SYMBOL_24,
	LFT_SYMBOL_32,
	LFT_SYSTEM_16,
	LFT_SYSTEM_8,
	LFT_TAHOMA_10,
	LFT_TAHOMA_12,
	LFT_TAHOMA_14,
	LFT_TAHOMA_16,
	LFT_TAHOMA_18,
	LFT_TAHOMA_20,
	LFT_TAHOMA_24,
	LFT_TAHOMA_32,
	LFT_TAHOMA_8,
	LFT_TERMINAL_12,
	LFT_TERMINAL_14,
	LFT_TERMINAL_18,
	LFT_TERMINAL_24,
	LFT_TERMINAL_32,
	LFT_TERMINAL_8,
	LFT_TERMINAL_9,
	LFT_TIMESROMAN_10,
	LFT_TIMESROMAN_11,
	LFT_TIMESROMAN_12,
	LFT_TIMESROMAN_14,
	LFT_TIMESROMAN_16,
	LFT_TIMESROMAN_18,
	LFT_TIMESROMAN_20,
	LFT_TIMESROMAN_24,
	LFT_TIMESROMAN_32,
	LFT_TIMESROMAN_8,
	LFT_TIMESROMAN_9,
	LFT_TREBUCHET_10,
	LFT_TREBUCHET_12,
	LFT_TREBUCHET_14,
	LFT_TREBUCHET_16,
	LFT_TREBUCHET_18,
	LFT_TREBUCHET_20,
	LFT_TREBUCHET_24,
	LFT_TREBUCHET_32,
	LFT_TREBUCHET_8,
	LFT_FIXEDSYS_8,
	LFT_ROMAN_32,
#endif
	LFT_END
};


#ifdef __cplusplus
extern "C" {
#endif

#include "mylcdtypes.h"

// returns pointer to version string
MYLCD_EXPORT void *
lVersion();

// Initialise library
// Pass font and map file path if not default location as defined by lFontPath lCharacterMapPath.
// eg; lOpenLibrary(L"path/to/font/directories/", L"path/to/mapfiles");
MYLCD_EXPORT int
lOpenLibrary (const wchar_t *fontPath, const wchar_t *mapPath);

// Shutdown library
MYLCD_EXPORT int
lCloseLibrary ();

// Get a device description handle
// returns a device handle
MYLCD_EXPORT THWD *
lOpen ();

// Closes all devices associated with 'hw' and free's resources
// returns 1 on success, 0 failure
MYLCD_EXPORT int
lClose (THWD *hw);

// Activate and initialize display and/or device
// displayName is the device to activate. eg: "PCD8544".
// portName = name of port driver connecting above driver to hardware. eg: "WinIO", or "NULL" if none.
// width/height = actual pixel width/height of display
// returns display ID on success, 0 on failure
// 'displayName' and 'portName' are case sensitive.
// handle to display rect should be maintained (be valid) throughout session
MYLCD_EXPORT lDISPLAY
lSelectDevice (THWD *hw, const char *displayName, const char *portName, int width, int height, int lfrm_bpp_nn, int data, TRECT *rect);

// Close a previously opened device
// It is not necessary to call lCloseDevice() as device(s) are automatically shutdown at lClose()
MYLCD_EXPORT int
lCloseDevice (THWD *hw, const lDISPLAY did);

// Set a display dependant option (lOPT_xxx_xxxx)
// returns 0 on failure, 1 on success
MYLCD_EXPORT int
lSetDisplayOption (THWD *hw, lDISPLAY did, int opt_option, intptr_t *value);

// Get a display dependant option (lOPT_xxxx)
// returns 0 on failure, 1 on success
MYLCD_EXPORT int
lGetDisplayOption (THWD *hw, lDISPLAY did, int opt_option, intptr_t *value);

// returns handle to a previously activated display
// returns NULL on failure
// hint: use 'drv->pd->writeX' and '->read' to directly access comm port if supported by driver
MYLCD_EXPORT TDRIVER *
lDisplayIDToDriver (THWD *hw, lDISPLAY did);

// Suspend display updates
MYLCD_EXPORT int
lPauseDisplay (THWD *hw, lDISPLAY did);

// Resume a suspended display.
MYLCD_EXPORT int
lResumeDisplay (THWD *hw, lDISPLAY did);

// List compiled in drivers. See examples/enumdrv.c
MYLCD_EXPORT TREGDRV *
lEnumerateDriversBegin (int ledrv_type);
MYLCD_EXPORT int
lEnumerateDriverNext (TREGDRV *d);
MYLCD_EXPORT int
lEnumerateDriverEnd (TREGDRV *d);

// Return reference ID of an activated driver
// refer to examples/usbd480BL.c for usage example
MYLCD_EXPORT lDISPLAY
lDriverNameToID (THWD *hw, const char *name, int ldrv_type);

// Create a new surface (frame) attached to root device tree 'hw'
// returns handle to new surface or NULL on error
// Release surface by calling lDeleteFrame()
MYLCD_EXPORT TFRAME *
lNewFrame (THWD *hw, int width, int height, ubyte lfrm_bpp_nn);

// Release frame
MYLCD_EXPORT int
lDeleteFrame (TFRAME *frm);

// Resize frame
// Set keepdata to 1 to retain as far as possible the original contents.
// returns 0 on error, positive on success
MYLCD_EXPORT int
lResizeFrame (TFRAME *frm, int newWidth, int newHeight, int keepdata);

// Create a duplicate of a frame including pixel buffer
// Release frame with lDeleteFrame()
MYLCD_EXPORT TFRAME *
lCloneFrame (TFRAME *src);

// Clear display screen
// Also clears the back buffer.
MYLCD_EXPORT int
lClearDisplay (THWD *hw);

// Clear frame
MYLCD_EXPORT int
lClearFrame (TFRAME *frm);

#define lClearFrameArea(a,b,c,d,e) lDrawRectangleFilled(a,b,c,d,e, a->hw->render->backGround)

// Load image on to frame
// Frame is resized to fit image
MYLCD_EXPORT int
lLoadImage (TFRAME *frm, const wchar_t *filename, int img_type, int lsp_style);

// Write frame to file
MYLCD_EXPORT int
lSaveImage (TFRAME *frm, const wchar_t *filename, int img_type, int width, int height);

// Returns a surface with specified text rendered
// use lDeleteFrame() to release
MYLCD_EXPORT TFRAME *
lNewString (THWD *hw, int lfrm_bpp_nn, int pf_flags, int font, const char *formatstring, ...);

// basic text rendering api
// string	null terminated buffer
// x,y		location in frame to print
// font		fontid
// style	draw method
MYLCD_EXPORT int
lPrint (TFRAME *frm, const char *string, int x, int y, int font, int lprt_style);

// as above but with formatted printing
MYLCD_EXPORT int
lPrintf (TFRAME *frm, int x, int y, int font, int lprt_style, const char *format, ...);

// extended rendering api with formatted text
// rect			rectangle defining rendering area and bounding box
// flags		PF_ render flags
// style		LPRT_ draw method flags
MYLCD_EXPORT int
lPrintEx (TFRAME *frm, TLPRINTR *rect, int font, int pf_flags, int lprt_style, const char *formatstring, ...);

// set glyph render direction
// bdf fonts only
// check example trenderfilter.c for more details
// returns 1 on sucess, 0 if failed to set render func
MYLCD_EXPORT int
lSetRenderDirection (THWD *hw, int ltr_mode);

// scroll frame 'src' through 'des'
// see examples 'clock.c', 'scroll.c', 'wa.c' and 'winamp/rssfeed.c' for more details
// returns handle scroll
// delete with lDeleteScroll()
MYLCD_EXPORT TLSCROLLEX *
lNewScroll (TFRAME *src, TFRAME *des);

// scroll source by one unit (pixel)
MYLCD_EXPORT int
lUpdateScroll (TLSCROLLEX *s);

// Draws scroll to distination from internal buffer, does not update position
MYLCD_EXPORT int
lDrawScroll (TLSCROLLEX *s);

// Release scroll resources
MYLCD_EXPORT int
lDeleteScroll (TLSCROLLEX *s);

// Send frame to display(s)
// If back buffering is enabled and supported then delta updates are performed.
// lRefresh() blocks until operation is complete
MYLCD_EXPORT int
lRefresh (TFRAME *frm);

// Send a frame area to display
// not supported by every controller. eg; USBD480 and G15 will redirect to lRefresh()
MYLCD_EXPORT int
lRefreshArea (TFRAME *frm, const int x1, const int y1, const int x2, const int y2);

// As with lRefresh() but non-blocking (operation is handled through a dedicated thread).
// mode: 0:buffer is copied, 1:buffers are swapped.
MYLCD_EXPORT int
lRefreshAsync (TFRAME *frm, int mode);

// Send buffer directly to display(s) backbuffer. No pixel conversions are applied.
MYLCD_EXPORT int
lUpdate (const THWD *hw, const void *buffer, const size_t bufferLen);


// get colour mask(s) of frame
MYLCD_EXPORT int
lGetRGBMask (const TFRAME *frame, const int lmask_colour);


// clear frame and set background as 'colour'
MYLCD_EXPORT int
lClearFrameClr (TFRAME *frm, int colour);


// get minimum bounding rect that would contain image within frame
MYLCD_EXPORT int
lGetImageBoundingBox (TFRAME *frame, TLPOINTEX *p);

// draws 'src' image with a 'mask' on to 'des'
// mode: LMASK_operation flags
// see example 'mask1/2.c' for more details
MYLCD_EXPORT int
lDrawMask (TFRAME *src, TFRAME *mask, TFRAME *des, int maskXOffset, int maskYOffset, int lmask_mode);

// draw mask'd src with alpha on to des
MYLCD_EXPORT int
lDrawMaskA (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int desXoffset, const int desYoffset, const float alpha);

MYLCD_EXPORT int
lDrawMaskAEx (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset,\
			  const int srcX1, const int srcY1, const int srcX2, const int srcY2, const float alpha);

// draws a progressbar
// see examples 'progressbar.c' for more details
MYLCD_EXPORT int
lDrawPBar (TFRAME *frm, int x, int y, int width, int height, float cursor_pos_pc, int pb_flags, int lsp_style);

// miscellaneous primitives
MYLCD_EXPORT int
lDrawPolyLine (TFRAME *frm, T2POINT *lppt, int tLines, int lsp_style);

MYLCD_EXPORT int
lDrawPolyLineEx (TFRAME *frm, TLPOINTEX *pt, int tLines, int lsp_style);

MYLCD_EXPORT int
lDrawPolyLineTo (TFRAME *frm, T2POINT *lppt, int tnodes, int lsp_style);

MYLCD_EXPORT int
lDrawPolyLineDottedTo (TFRAME *frm, T2POINT *lppt, int tnodes, int lsp_style);

MYLCD_EXPORT int
lDrawCircle (TFRAME *frm, int xcenter, int ycenter,int radius, int lsp_style);

MYLCD_EXPORT int
lDrawCircleFilled (TFRAME *frm, int xcenter, int ycenter,int radius, int lsp_style);

MYLCD_EXPORT int
lDrawEllipse (TFRAME *frm, int x, int y, int r1, int r2, int lsp_style);

MYLCD_EXPORT int
lDrawArc (TFRAME *frm, int x, int y, int r1, int r2, float a1, float a2, int lsp_style);

MYLCD_EXPORT int
lDrawEnclosedArc (TFRAME *frm, int x, int y, int r1, int r2, float a1, float a2, int lsp_style);

MYLCD_EXPORT int
lDrawLine (TFRAME *frm, int x1, int y1, int x2, int y2, int lsp_style);

MYLCD_EXPORT int
lDrawLineDotted (TFRAME *frm, int x1, int y1, int x2, int y2, int lsp_style);

MYLCD_EXPORT int
lDrawRectangle (TFRAME *frm, int x1, int y1, int width, int height, int lsp_style);

MYLCD_EXPORT int
lDrawRectangleFilled (TFRAME *frm, int x1, int y1, int width, int height, int lsp_style);

MYLCD_EXPORT int
lDrawRectangleDotted (TFRAME *frm, int x1, int y1, int width, int height, int lsp_style);

MYLCD_EXPORT int
lDrawRectangleDottedFilled (TFRAME *frm, int x1, int y1, int width, int height, int lsp_style);

MYLCD_EXPORT int
lDrawTriangle (TFRAME *frame, int x1, int y1, int x2, int y2, int x3, int y3, int colour);

MYLCD_EXPORT int
lDrawTriangleFilled (TFRAME *frame, int x1, int y1, int x2, int y2, int x3, int y3, int colour);

MYLCD_EXPORT int
lFloodFill (TFRAME *frame, int x, int y, int fillColour);

MYLCD_EXPORT int
lEdgeFill (TFRAME *frame, int x, int y, int fillColour, int edgeColour);


// moves an area of frame within frame by 'n' pixels
// lmov_mode sets what should happen to the area thats just been uncovered by the move
// ie, if one was moving left this would be the right hand side the move
// options are to loop from, dump, set all or clear pixels.
// see example 'anitest.c' for more details
MYLCD_EXPORT int
lMoveAreaLeft (TFRAME *frm, int x1, int y1, int x2, int y2, int n_pixels, int lmov_mode);

MYLCD_EXPORT int
lMoveAreaRight (TFRAME *frm, int x1, int y1, int x2, int y2, int n_pixels, int lmov_mode);

MYLCD_EXPORT int
lMoveAreaUp (TFRAME *frm, int x1, int y1, int x2, int y2, int n_pixels, int lmov_mode);

MYLCD_EXPORT int
lMoveAreaDown (TFRAME *frm, int x1, int y1, int x2, int y2, int n_pixels, int lmov_mode);

// basic frame copying
MYLCD_EXPORT int
lCopyFrame (TFRAME *from, TFRAME *to, int lcass_mode);

MYLCD_EXPORT int
lCopyArea (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2);

MYLCD_EXPORT int
lCopyAreaA (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, float alpha);

MYLCD_EXPORT int
lCopyAreaEx (TFRAME *from, TFRAME *to, int dx, int dy, int x1, int y1, int x2, int y2, int scaleh, int scalev, int lcass_mode);

MYLCD_EXPORT int
lCopyAreaScaled (TFRAME *from, TFRAME *to, int src_x, int src_y, int src_width, int src_height, int dest_x, int dest_y, int dest_width, int dest_height, int lcass_mode);

// invert frame
MYLCD_EXPORT int
lInvertFrame (TFRAME *frm);

// invert an area of frame
MYLCD_EXPORT int
lInvertArea (TFRAME *frm, int x1, int y1, int width, int height);

// flip frame vertically, horizontally or both.
// destination frame _must_ be the the same size or larger than source
MYLCD_EXPORT int
lFlipFrame (TFRAME *src, TFRAME *des, int ff_flag);

// basic rotation api
// view example 'anitest' for more details
MYLCD_EXPORT int
lRotate (TFRAME *src, TFRAME *des, int desx, int desy, double angle);

// rotates 'src' frame on to 'des' on a 3D plane
// see example 'rotatetest.c' for more details
MYLCD_EXPORT int
lRotateFrameEx (TFRAME *src, TFRAME *des, float xang, float yang, float zang, float flength, float zoom, int destx, int desty);

MYLCD_EXPORT int
lRotateFrameR90 (TFRAME *frm);

MYLCD_EXPORT int
lRotateFrameL90 (TFRAME *frm);

// calculates rotation of individual pixels by 'angle' degrees
MYLCD_EXPORT void
lRotateX (const float angle, const float y, const float z, float *yr, float *zr);

MYLCD_EXPORT void
lRotateY (const float angle, const float x, const float z, float *xr, float *zr);

MYLCD_EXPORT void
lRotateZ (const float angle, const float x, const float y, float *xr, float *yr);

MYLCD_EXPORT void
lxyzToPoint (float x, float y, float z, float flength, float camz, int x0, int y0, int *ptx, int *pty);

// TGA font enumeration api
// (single bitmap image fonts, library can support PNG or BMP bitmaps if requested)
// see example 'tgatest.c' for more details
MYLCD_EXPORT TENUMFONT *
lEnumFontsBegin ();

MYLCD_EXPORT int
lEnumFontsNext (TENUMFONT *enf);

MYLCD_EXPORT void
lEnumFontsDelete (TENUMFONT *enf);

// BDF font enumeration api
// see example 'bdftest.c' for more details
MYLCD_EXPORT TENUMFONT *
lEnumWFontsBegin ();

MYLCD_EXPORT int
lEnumWFontsNext (TENUMFONT *enf);

MYLCD_EXPORT void
lEnumWFontsDelete (TENUMFONT *enf);

// get internal font handle
// returns a handle to either a TFONT or TWFONT struct
MYLCD_EXPORT void *
lFontIDToFont (int fontid);

// Register a bitmap font (TGA, PGM, BMP or PNG).
// font sohuld have no more than 256 chars
// see source file 'fonts.c' for more details
MYLCD_EXPORT int
lRegisterFont (const wchar_t *filename, TFONT *font);

// release font and free its resources
// if font is built then it will be unavailable for use
MYLCD_EXPORT int
lUnregisterFont (int fontid);

// Register a BDF font
// see source file 'fonts.c' for more details
MYLCD_EXPORT int
lRegisterWFont (const wchar_t *filename, TWFONT *font);

// release font and free its resources
// if font or glyphs from font are built then these wil be
// unavailable for use after this call
MYLCD_EXPORT int
lUnregisterWFont (int fontid);

// delete font data and glyphs from memory
// see examples 'tgatest.c' and 'bdftest.c' for more details
// font remains registered
MYLCD_EXPORT int
lFlushFont (int fontid);

// merge glyphs from secondary font in to primary font
// view clock.c for an example of use
MYLCD_EXPORT int
lMergeWFont (int fontid_prim, int fontid_sec);

// unregister fonts releasing all resources
MYLCD_EXPORT int
lDeleteFonts();

// set current language encoding
MYLCD_EXPORT int
lSetCharacterEncoding (int cmt_id);
// get current language encoding
MYLCD_EXPORT int
lGetCharacterEncoding ();

// cycle through supported encodings, returning encoding alias, id (CMT_nnn) and filepath (if applicable)
// refer to examples/enumlang.c for more details
MYLCD_EXPORT int
lEnumLanguageTables (char **alias, int *cmt_id, wchar_t **filepath);

// look up encoding id from alias
// aliases are assigned in cmap.c
MYLCD_EXPORT int
lEncodingAliasToID (const char *alias);

// return handle to glyph pointed to by either 'str' or 'wc'
// do not free or returned handle or else bad things will happen
MYLCD_EXPORT TWCHAR *
lGetGlyph (const THWD *hw, const char *str, UTF32 wc, int fontid);

// get char block sise
MYLCD_EXPORT int
lGetCharMetrics (const char *str, int fontid, int *width, int *height);

// returns dimensions of text if rendered
// see 'wa.c' for an exmaple of use.
MYLCD_EXPORT int
lGetTextMetrics (const char *str, int pf_flags, int fontid, int *width, int *height);

// will also build font
MYLCD_EXPORT int
lGetFontMetrics (int fontid, int *width, int *height, int *ascent, int *descent, int *tchars);

// return number of code points within string, is similar to 'strlen()'
MYLCD_EXPORT int
lCountCharacters (const char *str);

// extract code points from string
// glist buffer size should be sizeof(UTF32)*total
// returns number of characters added to list
MYLCD_EXPORT int
lCreateCharacterList (const char *str, UTF32 *glist, int total);

// removes precached characters and those previously found to be missing (from font) from glist
// sorts code points in to descending order
// returns number of individual uncached characters remaining
MYLCD_EXPORT int
lStripCharacterList (UTF32 *glist, int gtotal, TWFONT *font);

// preload characters from unicode code point 'min' to 'max'
// returns number of glyphs loaded
MYLCD_EXPORT int
lCacheCharacterRange (UTF32 min, UTF32 max, int fontid);

// cache chars from buffer
// performs operation on supplied buffer (glist) which is stripped of invalid chars and sorted
// function will not reload a glyph thats already been cached
// returns number of glyphs loaded
MYLCD_EXPORT int
lCacheCharacterList (UTF32 *glist, int gtotal, int fontid);

// load glyphs from glist in to memory
// performs operation on a copy of input buffer, glist is unmodified.
// returns number of glyphs loaded
MYLCD_EXPORT int
lCacheCharacterBuffer (const UTF32 *glist, int gtotal, int fontid);

// Decode and load characters from string buffer, must be null terminated
// returns number of glyph's built
MYLCD_EXPORT int
lCacheCharacters (const char *str, int fontid);

// Load font completely in to memory
// returns number of glyph's built
MYLCD_EXPORT int
lCacheCharactersAll (int fontid);

// returns all code points currently in memory
// glist array should should be large enough for code points (gtotal*sizeof(UTF32) bytes)
// on success returns total code points
// returns -total if total cached glyphs is greater than gtotal
// see bdfdump.c for an example of use
MYLCD_EXPORT int
lGetCachedGlyphs (UTF32 *glist, int gtotal, int fontid);

// as with 'lGetTextMetrics()' but performs operation on an already decoded and cached buffer.
// font or part off must be pre-cached before calling.
// returns index to last computed char
// see bdfdump.c for an example of use
MYLCD_EXPORT int
lGetTextMetricsList (const UTF32 *glist, int first, int last, int pf_flags, int fontid, TLPRINTR *rect);

// renders text
// returns index to last char rendered
// see bdfdump.c for an example of use
MYLCD_EXPORT int
lPrintList (TFRAME *frm, const UTF32 *glist, int firstInList, int totalToPrint, TLPRINTR *rect, int font, int flags, int lprt_style);

// convert code point or char reference within string buffer to unicode code point
// returns number of chars read from 'str'
// note: CMT_AUTO_JP/CMT_ISO_2022_JP_EUC_SJIS is unsupported with this API, instead use 'lDecodeCharacterBuffer()'
MYLCD_EXPORT int
lDecodeCharacterCode (const char *str, UTF32 *chrout);

// as above, decodes a maximum of gtotal code points from buffer in to glist
// returns number of bytes read from buffer
MYLCD_EXPORT int
lDecodeCharacterBuffer (const char *buffer, UTF32 *glist, int gtotal);

// sets space between adjacent horizontal glyphs, in pixels
// returns current value
MYLCD_EXPORT int
lSetFontCharacterSpacing (int fontid, int pixels);

MYLCD_EXPORT int
lGetFontCharacterSpacing (int fontid);

// set number of vertical pixels to skip between line wraps.
// returns current value
MYLCD_EXPORT int
lSetFontLineSpacing (int fontid, int pixels);

// returns number of rows in pixels added when wrapping text.
MYLCD_EXPORT int
lGetFontLineSpacing (int fontid);

// enable/disable HTML character and entity references, ie; &#36215;, &amp;, etc..
MYLCD_EXPORT void
lHTMLCharRefEnable();
MYLCD_EXPORT void
lHTMLCharRefDisable();

// enable/disable character combining
MYLCD_EXPORT void
lCombinedCharDisable ();
MYLCD_EXPORT void
lCombinedCharEnable ();

// wait for 'time' milliseconds
MYLCD_EXPORT void
lSleep (int time);

// Set library specific behaviour. Currently only CAP_BACKBUFFER is implemented via this mechanism
MYLCD_EXPORT int
lSetCapabilities (THWD *hw, unsigned int cap_flag, int value);

// Returns 1 if cap_flag is compiled in. eg; CAP_BIG5 is by default not compiled in.
// Otherwise returns 0
// View mylcdconfig.h for details of compile time options
MYLCD_EXPORT int
lGetCapabilities (THWD *hw, unsigned int flag);

// Utility function to aid converting from 1bpp to other formats.
MYLCD_EXPORT unsigned int
lFrame1BPPToRGB (TFRAME *frame, void *des, int des_lfrm_bpp_nn, const int clrLow, const int clrHigh);

// convert and copy from one BPP format to another in to a user buffer
// does not support 1bpp source format, use lFrame1BPPToRGB() instead.
MYLCD_EXPORT pConverterFn
lGetConverter (int src_lfrm_bpp_nn, int des_lfrm_bpp_nn);
/* eg;
pConverterFn fn = lGetConverter(LFRM_BPP_32, LFRM_BPP_16);
if (fn){
	unsigned char *buffer = malloc(surface->height*surface->width*2);
	fn(surface, buffer);	// do convert and copy
	....
}
*/

// Set/Get how pixel write operations are performed per surface.
// Default is LSP_SET.
// Affects: Primitive, Print, Copy, Move and Clear APIs.
// Has no effect with 1bpp frames.
MYLCD_EXPORT void
lSetPixelWriteMode (TFRAME *frm, ubyte lsp_xxx);
MYLCD_EXPORT ubyte
lGetPixelWriteMode (TFRAME *frm);

// Set foreground colour (ink) of print.
// 1bpp frames (LFRM_BPP_1) should  pass LSP_xxx in place of colour component.
MYLCD_EXPORT int
lSetForegroundColour (THWD *hw, int colour);
MYLCD_EXPORT int
lGetForegroundColour (THWD *hw);

// Set/Get background colour (paper) of print.
// 1bpp frames (LFRM_BPP_1) should pass LSP_xxx in place of colour component.
MYLCD_EXPORT int
lSetBackgroundColour (THWD *hw, int colour);
MYLCD_EXPORT int
lGetBackgroundColour (THWD *hw);


#if __BUILD_PIXELPRIMITIVEEXPORTS__

// draw a pixel
MYLCD_EXPORT int
lSetPixel (const TFRAME *frm, const int x, const int y, const int colour);

// return pixel colour
MYLCD_EXPORT int
lGetPixel (const TFRAME *frm, const int x, const int y);

// get pixel without checking x,y are within the frame boundary
MYLCD_EXPORT int
lGetPixel_NB (const TFRAME *frm, const int x, const int y);

// set pixel without checking x,y are within the frame boundary
MYLCD_EXPORT int
lSetPixel_NB (const TFRAME *frm, const int x, const int y, const int value);

MYLCD_EXPORT int
lSetPixel_NBr (const TFRAME *frm, const int x, const int row, const int value);

MYLCD_EXPORT int
lGetPixel_NBr (const TFRAME *frm, const int x, const int row);

MYLCD_EXPORT int
lSetPixelf (const TFRAME *frm, const int x, const int y, float r, float g, float b);

MYLCD_EXPORT int
lGetPixelf (const TFRAME *frm, const int x, const int y, float *r, float *g, float *b);

MYLCD_EXPORT void *
lGetPixelAddress (const TFRAME *frm, const int x, const int y);

#else

#define lSetPixel(f, x, y, c)			(f->pixel->set(f, x, y, c))
#define lSetPixel_NB(f, x, y, c)		(f->pixel->set_NB(f, x, y, c))
#define lSetPixel_NBr(f, x, row, c)		(f->pixel->set_NBr(f, x, row, c))
#define lSetPixelf(f, x, y, r, g, b)	(f->pixel->setf(f, x, y, r, g, b))
#define lGetPixel(f, x, y)				(f->pixel->get(f, x, y))
#define lGetPixel_NB(f, x, y)			(f->pixel->get_NB(f, x, y))
#define lGetPixel_NBr(f, x, row)		(f->pixel->get_NBr(f, x, row))
#define lGetPixelf(f, x, y, r, g, b)	(f->pixel->getf(f, x, y, r, g, b))
#define lGetPixelAddress(f, x, y)		(f->pixel->getAddr(f, x, y))

#endif

#ifdef __cplusplus
}
#endif

#endif
