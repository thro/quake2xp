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

#include "client.h"

image_t *i_conback;
image_t *i_inventory;
image_t *i_net;
image_t *i_pause;
image_t *i_loading;
image_t *i_backtile;
image_t *i_turtle;
image_t *i_quit;
extern image_t *r_notexture;


void CL_InitImages () {

	i_conback = Draw_FindPic ("conback");
	i_inventory = Draw_FindPic ("inventory");
	i_net = Draw_FindPic ("net");
	i_pause = Draw_FindPic ("pause");
	i_loading = Draw_FindPic ("loading");

	i_backtile = Draw_FindPic ("backtile");
	i_turtle = Draw_FindPic ("turtle");
}
