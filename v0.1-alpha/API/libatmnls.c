#include <stdint.h>
#include <hb.h>
#include <hb-ft.h>
#include <stdlib.h>

void AutumnAPI_ShapeCompx(hb_font_t *hb_font, const char *txt); //description for existing symbol

//FIXME: added hb_font_t parameter for redirection to shaping function.
uint32_t AutumnAPI_UnicodeTF8(const char** s, hb_font_t *hb_font) {
	const unsigned char* p = (const unsigned char*)*s;
	if (!*p) return 0;
	
	//FIXME: added redirection to shaping function for 4 byte or special cases.
	if (p[0] >= 0xF0) { 
	    AutumnAPI_ShapeCompx(hb_font, *s);
	}

	if (p[0] < 0x80) {
		*s += 1;
		return p[0];
	}
	else if ((p[0] & 0xE0) == 0xC0) {
		if (p[1] == 0) return 0xFFFD;
		*s += 2;
		return ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
	}
	else if ((p[0] & 0xF0) == 0xE0) {
		if (p[1] == 0 || p[2] == 0) return 0xFFFD;
        	*s += 3;
		return ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    	}
	else if ((p[0] & 0xF8) == 0xF0) {
        	if (p[1] == 0 || p[2] == 0 || p[3] == 0) return 0xFFFD;
        	*s += 4;
        	return ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    	}
	*s += 1;
	return 0xFFFD;
}

//FIXME: added Harfbuzz based complex text shaping function for 4 byte chars.
void AutumnAPI_ShapeCompx(hb_font_t *hb_font, const char *txt) {
	hb_buffer_t *buf = hb_buffer_create(); //adding buffer for adding text data.
	if (!buf) return;

	hb_buffer_add_utf8(buf, utf8_text, -1, 0, -1); //adding text to buffer (*buf)
	hb_buffer_guess_segment_properties(buf); //detecting auto language and script data on UTF8 format
	
	hb_shape(hb_font, buf, NULL, 0); //shaping the font

	unsigned int glyph_count; //buffer for glyph data
    	hb_glyph_info_t *glyph_infos = hb_buffer_get_glyph_infos(buf, &glyph_count); //getting glyph info
    	hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count); //get glyph positions

	hb_buffer_destroy(buf); //MEMORY ALLOC!!!
}
