/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
#include "r_local.h"

index_t cube_idx[] = {
	// front
	0, 1, 2,
	2, 3, 0,
	// right
	1, 5, 6,
	6, 2, 1,
	// back
	7, 6, 5,
	5, 4, 7,
	// left
	4, 0, 3,
	3, 7, 4,
	// bottom
	4, 5, 1,
	1, 0, 4,
	// top
	3, 2, 6,
	6, 7, 3
};
uint iShadowCache[MD3_MAX_VERTS * MD3_MAX_MESHES * 6];

void R_InitVertexBuffers() {

	vec2_t		tmpVerts[4];
	index_t		iCache[6 * MAX_DRAW_STRING_LENGTH];
	int			idx = 0, i;
	void		*vmap, *imap;
	vmap = NULL;
	imap = NULL;

	glDeleteVertexArrays(1, &vao.fullscreenQuad);
	glDeleteVertexArrays(1, &vao.halfScreenQuad);
	glDeleteVertexArrays(1, &vao.quaterScreenQuad);

	Com_Printf("Initializing Vertex Buffers: ");

	for (i = 0; i < MD3_MAX_VERTS * MD3_MAX_MESHES * 6; i++)
		iShadowCache[i] = i;
	
	qglGenBuffers(1, &vbo.ibo_md3Shadow);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_md3Shadow);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint), iShadowCache, GL_STATIC_DRAW);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	for (i = 0, idx = 0; i < MAX_DRAW_STRING_LENGTH * 4; i += 4)
	{
		iCache[idx++] = i + 0;
		iCache[idx++] = i + 1;
		iCache[idx++] = i + 2;
		iCache[idx++] = i + 0;
		iCache[idx++] = i + 2;
		iCache[idx++] = i + 3;
	}

	qglGenBuffers(1, &vbo.ibo_quadTris);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, idx * sizeof(ushort), iCache, GL_STATIC_DRAW);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// precalc screen quads for postprocessing
	// full quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width, 0);
	VA_SetElem2(tmpVerts[3], 0, 0);
	qglGenBuffers(1, &vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao.fullscreenQuad);
		
	glBindVertexArray(vao.fullscreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_fullScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// half quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width * 0.5, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width * 0.5, vid.height * 0.5);
	VA_SetElem2(tmpVerts[3], 0, vid.height * 0.5);
	qglGenBuffers(1, &vbo.vbo_halfScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_halfScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenVertexArrays(1, &vao.halfScreenQuad);
	
	glBindVertexArray(vao.halfScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_halfScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// quater quad
	VA_SetElem2(tmpVerts[0], 0, vid.height);
	VA_SetElem2(tmpVerts[1], vid.width * 0.25, vid.height);
	VA_SetElem2(tmpVerts[2], vid.width * 0.25, vid.height * 0.25);
	VA_SetElem2(tmpVerts[3], 0, vid.height * 0.25);
	qglGenBuffers(1, &vbo.vbo_quarterScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_quarterScreenQuad);
	qglBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * 4, tmpVerts, GL_STATIC_DRAW);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &vao.quaterScreenQuad);

	glBindVertexArray(vao.quaterScreenQuad);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_quarterScreenQuad);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_quadTris);

	qglEnableVertexAttribArray(ATT_POSITION);
	qglVertexAttribPointer(ATT_POSITION, 2, GL_FLOAT, qfalse, 0, 0);

	glBindVertexArray(0);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//------------------------------------------------
	qglGenBuffers(1, &vbo.vbo_Dynamic);
	qglBindBuffer(GL_ARRAY_BUFFER, vbo.vbo_Dynamic);
	qglBufferData(GL_ARRAY_BUFFER, MAX_STREAM_VBO_VERTS * sizeof(vec4_t), 0, GL_STREAM_DRAW);
	qglBindBuffer(GL_ARRAY_BUFFER, 0);

	qglGenBuffers(1, &vbo.ibo_Dynamic);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_Dynamic);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_STREAM_IBO_IDX * sizeof(uint), 0, GL_STREAM_DRAW);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	qglGenBuffers(1, &vbo.ibo_cube);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.ibo_cube);
	qglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);
	qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	Com_Printf(S_COLOR_GREEN"ok\n\n");
}

void R_ShutDownVertexBuffers() {

	qglDeleteBuffers(1, &vbo.vbo_fullScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_halfScreenQuad);
	qglDeleteBuffers(1, &vbo.vbo_quarterScreenQuad);
	qglDeleteBuffers(1, &vbo.ibo_quadTris);
	qglDeleteBuffers(1, &vbo.vbo_Dynamic);
	qglDeleteBuffers(1, &vbo.ibo_Dynamic);
	qglDeleteBuffers(1, &vbo.vbo_BSP);
	qglDeleteBuffers(1, &vbo.ibo_md3Shadow);

	glDeleteVertexArrays(1, &vao.bsp_a);
	glDeleteVertexArrays(1, &vao.bsp_l);
	glDeleteVertexArrays(1, &vao.fullscreenQuad);
	glDeleteVertexArrays(1, &vao.halfScreenQuad);
	glDeleteVertexArrays(1, &vao.quaterScreenQuad);

}