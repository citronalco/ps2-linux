/* SPDX-License-Identifier: GPL-2.0 */
/*
 * console_struct.h
 *
 * Data structure describing single virtual console except for data
 * used by vt.c.
 *
 * Fields marked with [#] must be set by the low-level driver.
 * Fields marked with [!] can be changed by the low-level driver
 * to achieve effects such as fast scrolling by changing the origin.
 */

#ifndef _LINUX_CONSOLE_STRUCT_H
#define _LINUX_CONSOLE_STRUCT_H

#include <linux/tty.h>
#include <linux/wait.h>
#include <linux/vt.h>
#include <linux/workqueue.h>
#include <uapi/linux/kd.h>

struct uni_pagedir;
struct uni_screen;

#define NPAR 16

/*
 * Example: vc_data of a console that was scrolled 3 lines down.
 *
 *                              Console buffer
 * vc_screenbuf ---------> +----------------------+-.
 *                         | initializing W       |  \
 *                         | initializing X       |   |
 *                         | initializing Y       |    > scroll-back area
 *                         | initializing Z       |   |
 *                         |                      |  /
 * vc_visible_origin ---> ^+----------------------+-:
 * (changes by scroll)    || Welcome to linux     |  \
 *                        ||                      |   |
 *           vc_rows --->< | login: root          |   |  visible on console
 *                        || password:            |    > (vc_screenbuf_size is
 * vc_origin -----------> ||                      |   |   vc_size_row * vc_rows)
 * (start when no scroll) || Last login: 12:28    |  /
 *                        v+----------------------+-:
 *                         | Have a lot of fun... |  \
 * vc_pos -----------------|--------v             |   > scroll-front area
 *                         | ~ # cat_             |  /
 * vc_scr_end -----------> +----------------------+-:
 * (vc_origin +            |                      |  \ EMPTY, to be filled by
 *  vc_screenbuf_size)     |                      |  / vc_video_erase_char
 *                         +----------------------+-'
 *                         <---- 2 * vc_cols ----->
 *                         <---- vc_size_row ----->
 *
 * Note that every character in the console buffer is accompanied with an
 * attribute in the buffer right after the character. This is not depicted
 * in the figure.
 */
struct vc_data {
	struct tty_port port;			/* Upper level data */

	unsigned short	vc_num;			/* Console number */
	unsigned int	vc_cols;		/* [#] Console size */
	unsigned int	vc_rows;
	unsigned int	vc_size_row;		/* Bytes per row */
	unsigned int	vc_scan_lines;		/* # of scan lines */
	unsigned int	vc_cell_height;		/* CRTC character cell height */
	unsigned long	vc_origin;		/* [!] Start of real screen */
	unsigned long	vc_scr_end;		/* [!] End of real screen */
	unsigned long	vc_visible_origin;	/* [!] Top of visible window */
	unsigned int	vc_top, vc_bottom;	/* Scrolling region */
	const struct consw *vc_sw;
	unsigned short	*vc_screenbuf;		/* In-memory character/attribute buffer */
	unsigned int	vc_screenbuf_size;
	unsigned char	vc_mode;		/* KD_TEXT, ... */
	/* attributes for all characters on screen */
	unsigned char	vc_attr;		/* Current attributes */
	unsigned char	vc_def_color;		/* Default colors */
	unsigned char	vc_color;		/* Foreground & background */
	unsigned char	vc_s_color;		/* Saved foreground & background */
	unsigned char	vc_ulcolor;		/* Color for underline mode */
	unsigned char   vc_itcolor;
	unsigned char	vc_halfcolor;		/* Color for half intensity mode */
	/* cursor */
	unsigned int	vc_cursor_type;
	unsigned short	vc_complement_mask;	/* [#] Xor mask for mouse pointer */
	unsigned short	vc_s_complement_mask;	/* Saved mouse pointer mask */
	unsigned int	vc_x, vc_y;		/* Cursor position */
	unsigned int	vc_saved_x, vc_saved_y;
	unsigned long	vc_pos;			/* Cursor address */
	/* fonts */	
	unsigned short	vc_hi_font_mask;	/* [#] Attribute set for upper 256 chars of font or 0 if not supported */
	struct console_font vc_font;		/* Current VC font set */
	unsigned short	vc_video_erase_char;	/* Background erase character */
	/* VT terminal data */
	unsigned int	vc_state;		/* Escape sequence parser state */
	unsigned int	vc_npar,vc_par[NPAR];	/* Parameters of current escape sequence */
	/* data for manual vt switching */
	struct vt_mode	vt_mode;
	struct pid 	*vt_pid;
	int		vt_newvt;
	wait_queue_head_t paste_wait;
	/* mode flags */
	unsigned int	vc_charset	: 1;	/* Character set G0 / G1 */
	unsigned int	vc_s_charset	: 1;	/* Saved character set */
	unsigned int	vc_disp_ctrl	: 1;	/* Display chars < 32? */
	unsigned int	vc_toggle_meta	: 1;	/* Toggle high bit? */
	unsigned int	vc_decscnm	: 1;	/* Screen Mode */
	unsigned int	vc_decom	: 1;	/* Origin Mode */
	unsigned int	vc_decawm	: 1;	/* Autowrap Mode */
	unsigned int	vc_deccm	: 1;	/* Cursor Visible */
	unsigned int	vc_decim	: 1;	/* Insert Mode */
	/* attribute flags */
	unsigned int	vc_intensity	: 2;	/* 0=half-bright, 1=normal, 2=bold */
	unsigned int    vc_italic:1;
	unsigned int	vc_underline	: 1;
	unsigned int	vc_blink	: 1;
	unsigned int	vc_reverse	: 1;
	unsigned int	vc_s_intensity	: 2;	/* saved rendition */
	unsigned int    vc_s_italic:1;
	unsigned int	vc_s_underline	: 1;
	unsigned int	vc_s_blink	: 1;
	unsigned int	vc_s_reverse	: 1;
	/* misc */
	unsigned int	vc_priv		: 3;
	unsigned int	vc_need_wrap	: 1;
	unsigned int	vc_can_do_color	: 1;
	unsigned int	vc_report_mouse : 2;
	unsigned char	vc_utf		: 1;	/* Unicode UTF-8 encoding */
	unsigned char	vc_utf_count;
		 int	vc_utf_char;
	unsigned int	vc_tab_stop[8];		/* Tab stops. 256 columns. */
	unsigned char   vc_palette[16*3];       /* Colour palette for VGA+ */
	unsigned short * vc_translate;
	unsigned char 	vc_G0_charset;
	unsigned char 	vc_G1_charset;
	unsigned char 	vc_saved_G0;
	unsigned char 	vc_saved_G1;
	unsigned int    vc_resize_user;         /* resize request from user */
	unsigned int	vc_bell_pitch;		/* Console bell pitch */
	unsigned int	vc_bell_duration;	/* Console bell duration */
	unsigned short	vc_cur_blink_ms;	/* Cursor blink duration */
	struct vc_data **vc_display_fg;		/* [!] Ptr to var holding fg console for this display */
	struct uni_pagedir *vc_uni_pagedir;
	struct uni_pagedir **vc_uni_pagedir_loc; /* [!] Location of uni_pagedir variable for this console */
	struct uni_screen *vc_uni_screen;	/* unicode screen content */
	/* additional information is in vt_kern.h */
};

struct vc {
	struct vc_data *d;
	struct work_struct SAK_work;

	/* might add  scrmem, kbd  at some time,
	   to have everything in one place - the disadvantage
	   would be that vc_cons etc can no longer be static */
};

extern struct vc vc_cons [MAX_NR_CONSOLES];
extern void vc_SAK(struct work_struct *work);

#define CUR_DEF		0
#define CUR_NONE	1
#define CUR_UNDERLINE	2
#define CUR_LOWER_THIRD	3
#define CUR_LOWER_HALF	4
#define CUR_TWO_THIRDS	5
#define CUR_BLOCK	6
#define CUR_HWMASK	0x0f
#define CUR_SWMASK	0xfff0

#define CUR_DEFAULT CUR_UNDERLINE

bool con_is_visible(const struct vc_data *vc);

#endif /* _LINUX_CONSOLE_STRUCT_H */
