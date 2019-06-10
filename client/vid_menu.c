/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../client/client.h"
#include "../client/qmenu.h"
#include "../ref_gl/r_local.h"

extern cvar_t *vid_ref;

extern void M_ForceMenuOff (void);

int refresh = 0;

int menuSize = 0;

/*
====================================================================

MENU INTERACTION

====================================================================
*/
static menuframework_s	s_opengl_menu;
static menuframework_s	s_opengl2_menu;
static menuframework_s *s_current_menu;

static menulist_s		s_mode_list;
static menuslider_s		s_aniso_slider;

static menuslider_s		s_brightness_slider;
static menuslider_s		s_contrast_slider;
static menuslider_s		s_saturation_slider;
static menuslider_s		s_gamma_slider;
static menuslider_s		s_vibrance_slider;
static menuslider_s		s_fixfov_slider;
static menulist_s		s_lut_list;

static menuslider_s		s_bloomIntens_slider;
static menuslider_s		s_bloomThreshold_slider;
static menuslider_s		s_bloomWidth_slider;

static menulist_s  		s_fs_box;

static menuslider_s	    s_reliefScale_slider;
static menulist_s	    s_flare_box;
static menulist_s	    s_tc_box;
static menulist_s	    s_refresh_box;
static menulist_s	    s_parallax_box;
static menulist_s	    s_parallax_shadow;

static menulist_s	    s_samples_list;

static menulist_s	    s_bloom_box;
static menulist_s	    s_dof_box;

static menulist_s  		s_finish_box;
static menuaction_s		s_apply_action;
static menuaction_s		s_defaults_action;

static	menulist_s		s_autoBump_list;
static	menulist_s		s_radBlur_box;
static	menulist_s		s_ssao;
static	menulist_s		s_fxaa_box;
static	menulist_s		s_film_grain;
static	menulist_s		s_mb_box;
static	menuslider_s	s_ambientLevel_slider;

static	menuaction_s	s_menuAction_color;

static	menufield_s		s_menuColorTemp;

/////////////////////////////////////////////////////////
//
// MENU GENERIC FUNCTIONS
//
/////////////////////////////////////////////////////////

float ClampCvar (float min, float max, float value) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

static void ambientLevelCallback (void *s) {
	float ambient = s_ambientLevel_slider.curvalue / 20;
	Cvar_SetValue ("r_lightmapScale", ambient);
}

static void filmCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_filmFilter", box->curvalue * 1);
}

static void ParallaxCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_reliefMapping", box->curvalue * 1);
}

static void reliefScaleCallback(void *s) {
	menuslider_s *slider = (menuslider_s *)s;
	Cvar_SetValue("r_reliefScale", slider->curvalue * 1);
}

static void reliefShadowCallback(void *s) {
	menulist_s *box = (menulist_s *)s;
	Cvar_SetValue("r_reliefMappingSelfShadow", box->curvalue * 1);
}

static void FlareCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_drawFlares", box->curvalue * 1);
}

static void AnisoCallback (void *s) {
	menuslider_s *slider = (menuslider_s *)s;

	Cvar_SetValue ("r_anisotropic", slider->curvalue * 1);
}

static void BrightnessCallback (void *s) {
	float brt;
	brt = s_brightness_slider.curvalue / 10;

	Cvar_SetValue ("r_brightness", brt);
}

static void ContrastCallback(void *s) {
	float contr;
	contr = s_contrast_slider.curvalue / 10;

	Cvar_SetValue("r_contrast", contr);
}

static void SaturationCallback(void *s) {
	float sat;
	sat = s_saturation_slider.curvalue / 10;

	Cvar_SetValue("r_saturation", sat);
}

static void GammaCallback(void *s) {
	float gm;
	gm = s_gamma_slider.curvalue / 10;

	Cvar_SetValue("r_gamma", gm);
}

static void VibranceCallback(void *s) {
	float vb;
	vb = s_vibrance_slider.curvalue / 10;

	Cvar_SetValue("r_colorVibrance", vb);
}

static void FixFovCallback(void *s) {
	float vb;
	vb = s_fixfov_slider.curvalue / 10;

	Cvar_SetValue("r_fixFovStrength", vb);
}


static void BloomCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_bloom", box->curvalue * 1);
}

static void DofCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_dof", box->curvalue * 1);
}

static void RBCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_radialBlur", box->curvalue * 1);
}

static void ssaoCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_ssao", box->curvalue * 1);
}

static void fxaaCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_fxaa", box->curvalue * 1);
}

static void mbCallback (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_motionBlur", box->curvalue * 1);
}

static void bloomLevelCallback(void *s) {
	float intens = s_bloomIntens_slider.curvalue / 10;
	Cvar_SetValue("r_bloomIntens", intens);
}

static void bloomThresholdCallback(void *s) {
	float mins = s_bloomThreshold_slider.curvalue / 10;
	Cvar_SetValue("r_bloomThreshold", mins);
}

static void bloomWhidthCallback(void *s) {
	float star = s_bloomWidth_slider.curvalue / 10;
	Cvar_SetValue("r_bloomWidth", star);
}

static void ResetDefaults (void *unused) {
	VID_MenuInit ();
}

static void ApplyChanges (void *unused) {

	Cvar_SetValue ("r_anisotropic", s_aniso_slider.curvalue);
	Cvar_SetValue ("r_fullScreen", s_fs_box.curvalue);
	Cvar_SetValue ("r_drawFlares", s_flare_box.curvalue);
	Cvar_SetValue ("r_textureCompression", s_tc_box.curvalue);
	Cvar_SetValue ("r_mode", s_mode_list.curvalue);
	Cvar_SetValue ("r_reliefScale", s_reliefScale_slider.curvalue);
	Cvar_SetValue ("r_reliefMapping", s_parallax_box.curvalue);

	Cvar_SetValue("r_reliefMappingSelfShadow", s_parallax_shadow.curvalue);

	Cvar_SetValue ("r_bloom", s_bloom_box.curvalue);
	Cvar_SetValue ("r_dof", s_dof_box.curvalue);
	Cvar_SetValue ("r_radialBlur", s_radBlur_box.curvalue);
	Cvar_SetValue ("r_ssao", s_ssao.curvalue);
	Cvar_SetValue ("r_fxaa", s_fxaa_box.curvalue);
	Cvar_SetValue ("r_vsync", s_finish_box.curvalue);
	Cvar_SetValue ("r_filmFilter", s_film_grain.curvalue);
	Cvar_SetValue ("r_motionBlur", s_mb_box.curvalue);
	Cvar_SetValue("r_fixFovStrength", s_fixfov_slider.curvalue);

	/*
	** update appropriate stuff if we're running OpenGL and gamma
	** has been modified
	*/
	
	if (r_reliefMappingSelfShadow->modified)
		vid_ref->modified = qtrue;

	if (r_brightness->modified)
		vid_ref->modified = qtrue;
	
	if (r_contrast->modified)
		vid_ref->modified = qtrue;
	
	if (r_saturation->modified)
		vid_ref->modified = qtrue;

	if (r_gamma->modified)
		vid_ref->modified = qtrue;

	if (r_anisotropic->modified)
		vid_ref->modified = qtrue;

	if (r_reliefScale->modified)
		vid_ref->modified = qtrue;

	if (r_bloom->modified)
		vid_ref->modified = qtrue;

	if (r_dof->modified)
		vid_ref->modified = qtrue;

	if (r_displayRefresh->modified)
		vid_ref->modified = qtrue;

	if (r_drawFlares->modified)
		vid_ref->modified = qtrue;

	if (r_reliefMapping->modified)
		vid_ref->modified = qtrue;

	if (r_textureCompression->modified)
		vid_ref->modified = qtrue;

	if (r_vsync->modified)
		vid_ref->modified = qtrue;

	if (r_dof->modified)
		vid_ref->modified = qtrue;

	if (r_radialBlur->modified)
		vid_ref->modified = qtrue;

	if (r_ssao->modified)
		vid_ref->modified = qtrue;

	if (r_fxaa->modified)
		vid_ref->modified = qtrue;

	if (r_lightmapScale->modified)
		vid_ref->modified = qtrue;

	if (r_motionBlur->modified)
		vid_ref->modified = qtrue;

	if (r_bloomIntens->modified)
		vid_ref->modified = qtrue;

	if (r_bloomThreshold->modified)
		vid_ref->modified = qtrue;

	if (r_bloomWidth->modified)
		vid_ref->modified = qtrue;

	if (r_fixFovStrength->modified)
		vid_ref->modified = qtrue;
	
	M_ForceMenuOff ();

}

static void CancelChanges (void *unused) {
	extern void M_PopMenu (void);

	M_PopMenu ();
}


static void autoBumpCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_imageAutoBump", box->curvalue * 1);
}

static void vSyncCallBack (void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue ("r_vsync", box->curvalue * 1);
}

static void lutCallBack(void *s) {
	menulist_s *box = (menulist_s *)s;

	Cvar_SetValue("r_lutId", box->curvalue * 1);
}

void ColorTempFunc(void *unused)
{
	Cvar_Set("r_colorTempK", s_menuColorTemp.buffer);
	r_colorTempK->integer = ClampCvarInteger(999, 40000, r_colorTempK->integer);
}

void M_ColorInit() {

	if (!r_gamma)
		r_gamma = Cvar_Get("r_gamma", "1.0", CVAR_ARCHIVE);
	
	if (!r_colorTempK)
		r_colorTempK = Cvar_Get("r_colorTempK", "6500", CVAR_ARCHIVE);

	if (!r_brightness)
		r_brightness = Cvar_Get("r_brightness", "1", CVAR_ARCHIVE);

	if (!r_contrast)
		r_contrast = Cvar_Get("r_contrast", "1", CVAR_ARCHIVE);

	if (!r_saturation)
		r_saturation = Cvar_Get("r_saturation", "1", CVAR_ARCHIVE);

	if (!r_bloomThreshold)
		r_bloomThreshold = Cvar_Get("r_bloomThreshold", "0.75", CVAR_ARCHIVE);

	if (!r_bloomIntens)
		r_bloomIntens = Cvar_Get("r_bloomIntens", "0.5", CVAR_ARCHIVE);

	if (!r_bloomWidth)
		r_bloomWidth = Cvar_Get("r_bloomWidth", "3.0", CVAR_ARCHIVE);

	if (!r_fixFovStrength)
		r_fixFovStrength = Cvar_Get("r_fixFovStrength", "0.0", CVAR_ARCHIVE);

	r_gamma->value = ClampCvar(0.1, 2.5, r_gamma->value);
	r_brightness->value = ClampCvar(0.1, 2.0, r_brightness->value);
	r_contrast->value = ClampCvar(0.1, 2.0, r_contrast->value);
	r_saturation->value = ClampCvar(0.1, 2.0, r_saturation->value);
	r_colorVibrance->value = ClampCvar(-1.0, 1.0, r_colorVibrance->value);

	r_bloomIntens->value = ClampCvar(0.1, 2.0, r_bloomIntens->value);
	r_bloomThreshold->value = ClampCvar(0.1, 1.0, r_bloomThreshold->value);
	r_bloomWidth->value = ClampCvar(0.1, 3.0, r_bloomWidth->value);
	
	r_fixFovStrength->value = ClampCvar(0.0, 1.0, r_fixFovStrength->value);

	static char	*lut_table[] = { "Neutral", "Technicolor", "Sepia", "Black&White", 0 };

	s_opengl2_menu.x = viddef.width * 0.50;
	s_opengl2_menu.nitems = 0;

	s_gamma_slider.generic.type = MTYPE_SLIDER;
	s_gamma_slider.generic.x = 0;
	s_gamma_slider.generic.y = 10 * cl_fontScale->value;
	s_gamma_slider.generic.name = "Gamma";
	s_gamma_slider.generic.callback = GammaCallback;
	s_gamma_slider.minvalue = 1;
	s_gamma_slider.maxvalue = 20;
	s_gamma_slider.curvalue = r_gamma->value * 10;
	s_gamma_slider.generic.statusbar = "Screen Gamma";

	s_brightness_slider.generic.type = MTYPE_SLIDER;
	s_brightness_slider.generic.x = 0;
	s_brightness_slider.generic.y = 20 * cl_fontScale->value;
	s_brightness_slider.generic.name = "Brightness";
	s_brightness_slider.generic.callback = BrightnessCallback;
	s_brightness_slider.minvalue = 1;
	s_brightness_slider.maxvalue = 20;
	s_brightness_slider.curvalue = r_brightness->value * 10;
	s_brightness_slider.generic.statusbar = "Screen Brightness";

	s_contrast_slider.generic.type = MTYPE_SLIDER;
	s_contrast_slider.generic.x = 0;
	s_contrast_slider.generic.y = 30 * cl_fontScale->value;
	s_contrast_slider.generic.name = "Contrast";
	s_contrast_slider.generic.callback = ContrastCallback;
	s_contrast_slider.minvalue = 1;
	s_contrast_slider.maxvalue = 20;
	s_contrast_slider.curvalue = r_contrast->value * 10;
	s_contrast_slider.generic.statusbar = "Screen Contrast";

	s_saturation_slider.generic.type = MTYPE_SLIDER;
	s_saturation_slider.generic.x = 0;
	s_saturation_slider.generic.y = 40 * cl_fontScale->value;
	s_saturation_slider.generic.name = "Saturation";
	s_saturation_slider.generic.callback = SaturationCallback;
	s_saturation_slider.minvalue = 1;
	s_saturation_slider.maxvalue = 20;
	s_saturation_slider.curvalue = r_saturation->value * 10;
	s_saturation_slider.generic.statusbar = "Screen Saturation";

	s_vibrance_slider.generic.type = MTYPE_SLIDER;
	s_vibrance_slider.generic.x = 0;
	s_vibrance_slider.generic.y = 50 * cl_fontScale->value;
	s_vibrance_slider.generic.name = "Vibrance";
	s_vibrance_slider.generic.callback = VibranceCallback;
	s_vibrance_slider.minvalue = -10;
	s_vibrance_slider.maxvalue = 10;
	s_vibrance_slider.curvalue = r_colorVibrance->value * 10;
	s_vibrance_slider.generic.statusbar = "Color Vibrance";


	s_bloomIntens_slider.generic.type = MTYPE_SLIDER;
	s_bloomIntens_slider.generic.x = 0;
	s_bloomIntens_slider.generic.y = 70 * cl_fontScale->value;
	s_bloomIntens_slider.generic.name = "Bloom Intensity";
	s_bloomIntens_slider.generic.callback = bloomLevelCallback;
	s_bloomIntens_slider.minvalue = 1;
	s_bloomIntens_slider.maxvalue = 20;
	s_bloomIntens_slider.curvalue = r_bloomIntens->value * 10;
	s_bloomIntens_slider.generic.statusbar = "Bloom Intensity";

	s_bloomThreshold_slider.generic.type = MTYPE_SLIDER;
	s_bloomThreshold_slider.generic.x = 0;
	s_bloomThreshold_slider.generic.y = 80 * cl_fontScale->value;
	s_bloomThreshold_slider.generic.name = "Bloom Threshold";
	s_bloomThreshold_slider.generic.callback = bloomThresholdCallback;
	s_bloomThreshold_slider.minvalue = 1;
	s_bloomThreshold_slider.maxvalue = 10;
	s_bloomThreshold_slider.curvalue = r_bloomThreshold->value * 10;
	s_bloomThreshold_slider.generic.statusbar = "Bloom Threshold";

	s_bloomWidth_slider.generic.type = MTYPE_SLIDER;
	s_bloomWidth_slider.generic.x = 0;
	s_bloomWidth_slider.generic.y = 90 * cl_fontScale->value;
	s_bloomWidth_slider.generic.name = "Bloom Shape Size";
	s_bloomWidth_slider.generic.callback = bloomWhidthCallback;
	s_bloomWidth_slider.minvalue = 1;
	s_bloomWidth_slider.maxvalue = 30;
	s_bloomWidth_slider.curvalue = r_bloomWidth->value * 10;
	s_bloomWidth_slider.generic.statusbar = "Bloom Shape Size";

	s_fixfov_slider.generic.type = MTYPE_SLIDER;
	s_fixfov_slider.generic.x = 0;
	s_fixfov_slider.generic.y = 110 * cl_fontScale->value;
	s_fixfov_slider.generic.name = "Hi-FOV Corection";
	s_fixfov_slider.generic.callback = FixFovCallback;
	s_fixfov_slider.minvalue = 0;
	s_fixfov_slider.maxvalue = 10;
	s_fixfov_slider.curvalue = r_fixFovStrength->value * 10;
	s_fixfov_slider.generic.statusbar = "Reducing Field Of View Distortion";

	s_lut_list.generic.type = MTYPE_SPINCONTROL;
	s_lut_list.generic.name = "Color Grading";
	s_lut_list.generic.x = 0;
	s_lut_list.generic.y = 130 * cl_fontScale->value;
	s_lut_list.itemnames = lut_table;
	s_lut_list.curvalue = r_lutId->integer;
	s_lut_list.generic.callback = lutCallBack;
	s_lut_list.generic.statusbar = "Neutral, Technicolor, Sepia, Black&White";

	s_menuColorTemp.generic.type = MTYPE_FIELD;
	s_menuColorTemp.generic.name = "Color Temperature";
	s_menuColorTemp.generic.flags = QMF_NUMBERSONLY;
	s_menuColorTemp.generic.x = 0;
	s_menuColorTemp.generic.y = 140 * cl_fontScale->value;
	s_menuColorTemp.generic.statusbar = "Color Temperature in Kelvins 1000 - 40000";
	s_menuColorTemp.length = 5;
	s_menuColorTemp.visible_length = 5;
	s_menuColorTemp.generic.callback = ColorTempFunc;
	strcpy(s_menuColorTemp.buffer, Cvar_VariableString("r_colorTempK"));

	menuSize = 130;

	Menu_AddItem(&s_opengl2_menu, (void *)&s_gamma_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_brightness_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_contrast_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_saturation_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_vibrance_slider);

	Menu_AddItem(&s_opengl2_menu, (void *)&s_bloomIntens_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_bloomThreshold_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_bloomWidth_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_fixfov_slider);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_lut_list);
	Menu_AddItem(&s_opengl2_menu, (void *)&s_menuColorTemp);


	Menu_Center(&s_opengl2_menu);
	s_opengl2_menu.x -= 8;
}

int Default_MenuKey(menuframework_s * m, int key);
void M_PushMenu(void(*draw) (void), int(*key) (int k));

void Color_MenuDraw(void)
{
	int w, h;

	menuSize = 170 * cl_fontScale->value;

	// draw the banner
	Draw_GetPicSize(&w, &h, "m_banner_video");

	if (cl_fontScale->value == 2) {
		Draw_PicScaled((int)(viddef.width *0.5 - (w *0.5)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video");
		Draw_PicBumpScaled((int)(viddef.width *0.5 - (w *0.5)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video", "m_banner_video_bump");
	}
	else if (cl_fontScale->value == 3) {
		Draw_PicScaled((int)(viddef.width *0.5 - (w *0.75)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video");
		Draw_PicBumpScaled((int)(viddef.width *0.5 - (w *0.75)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video", "m_banner_video_bump");
	}
	
	Menu_AdjustCursor(&s_opengl2_menu, 1);
	Menu_Draw(&s_opengl2_menu);
}

int Color_MenuKey(int key)
{
	return Default_MenuKey(&s_opengl2_menu, key);
}

void M_Menu_Color_f(void) {
	M_ColorInit();
	M_PushMenu(Color_MenuDraw, Color_MenuKey);
}


static void ColorSettingsFunc(void *unused) {
	M_Menu_Color_f();
}


/*
** VID_MenuInit
*/
void VID_MenuInit (void) {

	static char *resolutions[] = {
		"[Native][Desktop Resolution]",
		"[1024 768][4:3]",
		"[1152 864][4:3]",
		"[1280 1024][5:4]",
		"[1600 1200][4:3]",
		"[2048 1536][4:3]",

		"[1280 720][720p HDTV]",
		"[1280 800][16:10]",
		"[1366 768][16:9 Plasma and LCD TV]",
		"[1440 900][16:10]",
		"[1600 900][16:9 LCD]",
		"[1680 1050][16:10]",
		"[1920 1080][1080p FULL HDTV]",
		"[1920 1200][16:10]",
		"[2560 1440][16:9 WQHD]",
		"[2560 1600][16:10]",
		"[3440 1440][21:9 QHD]",
		"[3840 1600][12:5 UW TV]",
		"[3840 2160][16:9 UHD TV 4K]",
		"[4096 1716][2.39:1 DCI 4K WIDE]",
		"[4096 2160][1.89:1 DCI 4K]",
		"[Custom]", 0 };

	static char	*yesno_names[] = { "off", "yes", 0 };
	static char	*adaptive_vc[] = { "off", "standart", "adaptive", 0 };

	if(!r_reliefMappingSelfShadow)
		r_reliefMappingSelfShadow = Cvar_Get("r_reliefMappingSelfShadow", "0", CVAR_ARCHIVE);

	if (!r_mode)
		r_mode = Cvar_Get ("r_mode", "0", CVAR_ARCHIVE);

	if (!r_anisotropic)
		r_anisotropic = Cvar_Get ("r_anisotropic", "1", CVAR_ARCHIVE);

	if (!r_textureCompression)
		r_textureCompression = Cvar_Get ("r_textureCompression", "0", CVAR_ARCHIVE);

	if (!r_drawFlares)
		r_drawFlares = Cvar_Get ("r_drawFlares", "0", CVAR_ARCHIVE);


	if (!r_bloom)
		r_bloom = Cvar_Get ("r_bloom", "0", CVAR_ARCHIVE);

	if (!r_reliefMapping)
		r_reliefMapping = Cvar_Get ("r_reliefMapping", "0", CVAR_ARCHIVE);

	if (r_reliefMapping->value > 1)
		r_reliefMapping = Cvar_Get("r_reliefMapping", "1", CVAR_ARCHIVE);

	r_reliefScale->value = ClampCvar(1.0, 10.0, r_reliefScale->value);

	if (!r_dof)
		r_dof = Cvar_Get ("r_dof", "0", CVAR_ARCHIVE);

	if (!r_radialBlur->integer)
		r_radialBlur = Cvar_Get ("r_radialBlur", "0", CVAR_ARCHIVE);

	if (!r_imageAutoBump)
		r_imageAutoBump = Cvar_Get ("r_imageAutoBump", "0", CVAR_ARCHIVE);

	if (!r_vsync)
		r_vsync = Cvar_Get ("r_vsync", "0", CVAR_ARCHIVE);

	if (!cl_fontScale)
		cl_fontScale = Cvar_Get ("cl_fontScale", "2", CVAR_ARCHIVE);

	if (!r_ssao->integer)
		r_ssao = Cvar_Get ("r_ssao", 0, CVAR_ARCHIVE);

	if (!r_fxaa->integer)
		r_fxaa = Cvar_Get ("r_fxaa", 0, CVAR_ARCHIVE);

	if (!r_lightmapScale->value)
		r_lightmapScale = Cvar_Get ("r_lightmapScale", "0", CVAR_ARCHIVE);

	if (!r_motionBlur->integer)
		r_motionBlur = Cvar_Get ("r_motionBlur", "0", CVAR_ARCHIVE);

	s_opengl_menu.x = viddef.width * 0.50;
	s_opengl_menu.nitems = 0;

	s_mode_list.generic.type = MTYPE_SPINCONTROL;
	s_mode_list.generic.name = "Screen Resolution";
	s_mode_list.generic.x = 0;
	s_mode_list.generic.y = 10 * cl_fontScale->value;
	s_mode_list.itemnames = resolutions;
	s_mode_list.curvalue = r_mode->value;
	s_mode_list.generic.statusbar = "Screen Resolution <Requires Restart Video Sub-System>";

	s_fs_box.generic.type = MTYPE_SPINCONTROL;
	s_fs_box.generic.x = 0;
	s_fs_box.generic.y = 20 * cl_fontScale->value;
	s_fs_box.generic.name = "Fullscreen";
	s_fs_box.itemnames = yesno_names;
	s_fs_box.curvalue = r_fullScreen->value;
	s_fs_box.generic.statusbar = " Use Full Screen <Requires Restart Video Sub-System>";

	// -----------------------------------------------------------------------

	s_aniso_slider.generic.type = MTYPE_SLIDER;
	s_aniso_slider.generic.x = 0;
	s_aniso_slider.generic.y = 40 * cl_fontScale->value;
	s_aniso_slider.generic.name = "Anisotropy Filtering";
	s_aniso_slider.minvalue = 1;
	s_aniso_slider.maxvalue = 16;
	s_aniso_slider.curvalue = r_anisotropic->value;
	s_aniso_slider.generic.callback = AnisoCallback;
	s_aniso_slider.generic.statusbar = "Texture Anisotropy Level";

	s_tc_box.generic.type = MTYPE_SPINCONTROL;
	s_tc_box.generic.x = 0;
	s_tc_box.generic.y = 50 * cl_fontScale->value;
	s_tc_box.generic.name = "Texture Compression";
	s_tc_box.itemnames = yesno_names;
	s_tc_box.curvalue = r_textureCompression->value;
	s_tc_box.generic.statusbar = "Use Compressed Textures <Requires Restart Video Sub-System>";

	// -----------------------------------------------------------------------

	s_autoBump_list.generic.type = MTYPE_SPINCONTROL;
	s_autoBump_list.generic.name = "Generate Normal Maps";
	s_autoBump_list.generic.x = 0;
	s_autoBump_list.generic.y = 70 * cl_fontScale->value;
	s_autoBump_list.itemnames = yesno_names;
	s_autoBump_list.curvalue = r_imageAutoBump->value;
	s_autoBump_list.generic.callback = autoBumpCallBack;
	s_autoBump_list.generic.statusbar = "Realtime Normal Maps Generation For Old Textures";

	s_parallax_box.generic.type = MTYPE_SPINCONTROL;
	s_parallax_box.generic.x = 0;
	s_parallax_box.generic.y = 80 * cl_fontScale->value;
	s_parallax_box.generic.name = "Relief Mapping";
	s_parallax_box.itemnames = yesno_names;
	s_parallax_box.curvalue = r_reliefMapping->value;
	s_parallax_box.generic.callback = ParallaxCallback;
	s_parallax_box.generic.statusbar = "Use High Quality Virtual Displacement Mapping";

	s_parallax_shadow.generic.type = MTYPE_SPINCONTROL;
	s_parallax_shadow.generic.x = 0;
	s_parallax_shadow.generic.y = 90 * cl_fontScale->value;
	s_parallax_shadow.generic.name = "Self Shadowing Parallax";
	s_parallax_shadow.itemnames = yesno_names;
	s_parallax_shadow.curvalue = r_reliefMappingSelfShadow->integer;
	s_parallax_shadow.generic.callback = reliefShadowCallback;
	s_parallax_shadow.generic.statusbar = "Virtual Displacement Mapping Self Shadowing";

	s_reliefScale_slider.generic.type = MTYPE_SLIDER;
	s_reliefScale_slider.generic.x = 0;
	s_reliefScale_slider.generic.y = 100 * cl_fontScale->value;
	s_reliefScale_slider.generic.name = "Relief Scale";
	s_reliefScale_slider.minvalue = 1;
	s_reliefScale_slider.maxvalue = 6;
	s_reliefScale_slider.curvalue = r_reliefScale->value;
	s_reliefScale_slider.generic.callback = reliefScaleCallback;
	s_reliefScale_slider.generic.statusbar = "Virtual Displacement Depth";


	s_ambientLevel_slider.generic.type = MTYPE_SLIDER;
	s_ambientLevel_slider.generic.x = 0;
	s_ambientLevel_slider.generic.y = 110 * cl_fontScale->value;
	s_ambientLevel_slider.generic.name = "Lightmap Brightness";
	s_ambientLevel_slider.generic.callback = ambientLevelCallback;
	s_ambientLevel_slider.minvalue = 0;
	s_ambientLevel_slider.maxvalue = 20;
	s_ambientLevel_slider.curvalue = r_lightmapScale->value * 20;
	s_ambientLevel_slider.generic.statusbar = "Precomputed Lighting Level";

	s_flare_box.generic.type = MTYPE_SPINCONTROL;
	s_flare_box.generic.x = 0;
	s_flare_box.generic.y = 130 * cl_fontScale->value;
	s_flare_box.generic.name = "Light Flares";
	s_flare_box.itemnames = yesno_names;
	s_flare_box.curvalue = r_drawFlares->value;
	s_flare_box.generic.callback = FlareCallback;
	s_flare_box.generic.statusbar = "Draw Lights Corona Effect";

	s_bloom_box.generic.type = MTYPE_SPINCONTROL;
	s_bloom_box.generic.x = 0;
	s_bloom_box.generic.y = 140 * cl_fontScale->value;
	s_bloom_box.generic.name = "Bloom";
	s_bloom_box.itemnames = yesno_names;
	s_bloom_box.curvalue = r_bloom->value;
	s_bloom_box.generic.callback = BloomCallback;
	s_bloom_box.generic.statusbar = "Draw Bloom Effect";

	s_dof_box.generic.type = MTYPE_SPINCONTROL;
	s_dof_box.generic.x = 0;
	s_dof_box.generic.y = 150 * cl_fontScale->value;
	s_dof_box.generic.name = "Depth of Field";
	s_dof_box.itemnames = yesno_names;
	s_dof_box.curvalue = r_dof->value;
	s_dof_box.generic.callback = DofCallback;
	s_dof_box.generic.statusbar = "Draw Depth of Field Effect";

	s_radBlur_box.generic.type = MTYPE_SPINCONTROL;
	s_radBlur_box.generic.x = 0;
	s_radBlur_box.generic.y = 160 * cl_fontScale->value;
	s_radBlur_box.generic.name = "Radial Blur";
	s_radBlur_box.itemnames = yesno_names;
	s_radBlur_box.curvalue = r_radialBlur->value;
	s_radBlur_box.generic.callback = RBCallback;
	s_radBlur_box.generic.statusbar = "Draw Radial Blur Effect";

	s_mb_box.generic.type = MTYPE_SPINCONTROL;
	s_mb_box.generic.x = 0;
	s_mb_box.generic.y = 170 * cl_fontScale->value;
	s_mb_box.generic.name = "Motion Blur";
	s_mb_box.itemnames = yesno_names;
	s_mb_box.curvalue = r_motionBlur->value;
	s_mb_box.generic.callback = mbCallback;
	s_mb_box.generic.statusbar = "Draw Motion Blur Effect";

	s_ssao.generic.type = MTYPE_SPINCONTROL;
	s_ssao.generic.x = 0;
	s_ssao.generic.y = 180 * cl_fontScale->value;
	s_ssao.generic.name = "SSAO";
	s_ssao.itemnames = yesno_names;
	s_ssao.curvalue = r_ssao->value;
	s_ssao.generic.callback = ssaoCallback;
	s_ssao.generic.statusbar = "Draw Screen Space Ambient Occlusion Effect";

	s_film_grain.generic.type = MTYPE_SPINCONTROL;
	s_film_grain.generic.x = 0;
	s_film_grain.generic.y = 190 * cl_fontScale->value;
	s_film_grain.generic.name = "Cinematic filter";
	s_film_grain.itemnames = yesno_names;
	s_film_grain.curvalue = r_filmFilter->integer;
	s_film_grain.generic.callback = filmCallback;
	s_film_grain.generic.statusbar = "Use Cinematic Film Effect noise, vignet and scratches";

	s_fxaa_box.generic.type = MTYPE_SPINCONTROL;
	s_fxaa_box.generic.x = 0;
	s_fxaa_box.generic.y = 200 * cl_fontScale->value;
	s_fxaa_box.generic.name = "FXAA";
	s_fxaa_box.itemnames = yesno_names;
	s_fxaa_box.curvalue = r_fxaa->value;
	s_fxaa_box.generic.callback = fxaaCallback;
	s_fxaa_box.generic.statusbar = "Use Post-Process Anti-Aliasing";

	s_finish_box.generic.type = MTYPE_SPINCONTROL;
	s_finish_box.generic.x = 0;
	s_finish_box.generic.y = 210 * cl_fontScale->value;
	s_finish_box.generic.name = "Vertical Sync";
	s_finish_box.generic.callback = vSyncCallBack;
	s_finish_box.curvalue = r_vsync->value;
if (r_vsync->value >= 3)
	Cvar_SetValue ("r_vsync", 2);
	s_finish_box.itemnames = adaptive_vc;
	s_finish_box.generic.statusbar = "Standart Or Adaptive";


	s_menuAction_color.generic.type = MTYPE_ACTION;
	s_menuAction_color.generic.x = 0;
	s_menuAction_color.generic.y = 230 * cl_fontScale->value;
	s_menuAction_color.generic.name = "Post-Process Settings...";
	s_menuAction_color.generic.callback = ColorSettingsFunc;
	s_menuAction_color.generic.statusbar = "Color Balance and Bloom Settings";

	s_defaults_action.generic.type = MTYPE_ACTION;
	s_defaults_action.generic.name = "reset to defaults";
	s_defaults_action.generic.x = 0;
	s_defaults_action.generic.y = 250 * cl_fontScale->value;
	s_defaults_action.generic.callback = ResetDefaults;

	s_apply_action.generic.type = MTYPE_ACTION;
	s_apply_action.generic.name = "Apply Changes";
	s_apply_action.generic.x = 0;
	s_apply_action.generic.y = 260 * cl_fontScale->value;
	s_apply_action.generic.callback = ApplyChanges;

	menuSize = 270;

	Menu_AddItem (&s_opengl_menu, (void *)&s_mode_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fs_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_aniso_slider);

	Menu_AddItem (&s_opengl_menu, (void *)&s_tc_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_autoBump_list);
	Menu_AddItem (&s_opengl_menu, (void *)&s_parallax_box);
	Menu_AddItem(&s_opengl_menu, (void *)&s_parallax_shadow);

	Menu_AddItem (&s_opengl_menu, (void *)&s_reliefScale_slider);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ambientLevel_slider);
	Menu_AddItem (&s_opengl_menu, (void *)&s_flare_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_bloom_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_dof_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_radBlur_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_mb_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_ssao);
	Menu_AddItem (&s_opengl_menu, (void *)&s_film_grain);
	Menu_AddItem (&s_opengl_menu, (void *)&s_fxaa_box);
	Menu_AddItem (&s_opengl_menu, (void *)&s_finish_box);

	Menu_AddItem (&s_opengl_menu, (void *)&s_menuAction_color);
	Menu_AddItem (&s_opengl_menu, (void *)&s_defaults_action);
	Menu_AddItem (&s_opengl_menu, (void *)&s_apply_action);

	Menu_Center (&s_opengl_menu);
	s_opengl_menu.x -= 8;

}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void) {
	int w, h;

	s_current_menu = &s_opengl_menu;

	menuSize = 170 * cl_fontScale->value;

	// draw the banner
	Draw_GetPicSize (&w, &h, "m_banner_video");

	if (cl_fontScale->value == 2) {
		Draw_PicScaled((int)(viddef.width *0.5 - (w *0.5)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video");
		Draw_PicBumpScaled((int)(viddef.width *0.5 - (w *0.5)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video", "m_banner_video_bump");
	}
	else if (cl_fontScale->value == 3) {
		Draw_PicScaled((int)(viddef.width *0.5 - (w *0.75)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video");
		Draw_PicBumpScaled((int)(viddef.width *0.5 - (w *0.75)), (int)(viddef.height *0.5 - menuSize), cl_fontScale->value, cl_fontScale->value, "m_banner_video", "m_banner_video_bump");
	}

	// move cursor to a reasonable starting position
	Menu_AdjustCursor (s_current_menu, 1);

	// draw menu
	Menu_Draw (s_current_menu);
}

/*
================
VID_MenuKey
================
*/
int VID_MenuKey (int key) {
	menuframework_s *m = s_current_menu;

	switch (key) {
		case K_XPAD_B:
		case K_XPAD_BACK:
		case K_ESCAPE:
			CancelChanges (NULL);
			return 0;

		case K_MWHEELUP:
		case K_KP_UPARROW:
		case K_XPAD_DPAD_UP:
		case K_UPARROW:
			m->cursor--;
			Menu_AdjustCursor (m, -1);
			break;

		case K_MWHEELDOWN:
		case K_KP_DOWNARROW:
		case K_XPAD_DPAD_DOWN:
		case K_DOWNARROW:
			m->cursor++;
			Menu_AdjustCursor (m, 1);
			break;
		case K_KP_LEFTARROW:
		case K_XPAD_DPAD_LEFT:
		case K_LEFTARROW:
			Menu_SlideItem (m, -1);
			break;
		case K_KP_RIGHTARROW:
		case K_XPAD_DPAD_RIGHT:
		case K_RIGHTARROW:
			Menu_SlideItem (m, 1);
			break;
		case K_KP_ENTER:
		case K_XPAD_A:
		case K_ENTER:
			if (!Menu_SelectItem (m))
				ApplyChanges (NULL);
			break;
	}

	return menu_in_sound;
}


