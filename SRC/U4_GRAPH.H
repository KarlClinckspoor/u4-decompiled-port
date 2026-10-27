/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */
#ifndef __U4_GRAPH_H__
#define __U4_GRAPH_H__

extern int16 /*C_20C1*/LoadGraphicsDriver(char *);
extern int16 /*C_213B*/DrawGameFrame();
extern int16 cdecl Gra_Line(int16, int16, int16, int16, int16);
extern int16 cdecl /*C_2201*/Gra_ActiveBox(int16, int16);
extern int16 cdecl /*C_2206*/Gra_ZoomTileRow(/**/);
extern int16 cdecl /*C_220B*/Gra_FillTiles(/**/);
extern int16 cdecl /*C_2210*/Gra_ShiftDown();
extern int16 cdecl /*C_221B*/Gra_ShiftUp();
extern int16 cdecl /*C_2226*/Gra_DrawTiles(/*int h, int w, void *src, int dst_ofs*/);
extern int16 cdecl /*C_222B*/Gra_Dot(/*int, int, int*/);
extern int16 /*C_2230*/Gra_CR();
extern int16 /*C_2241*/Gra_InvertView();
extern int16 /*C_2246*/Gra_ClearView();
extern int16 /*C_224B*/Gra_InvertStatusLine(int16);
extern int16 cdecl /*C_2250*/Gra_InvertChar(int16, int16);/*(uncalled)*/
extern int16 /*C_226C*/Gra_ClearStatus();
extern int16 cdecl /*C_2271*/Gra_DotOr(/*int, int, int*/);
extern int16 cdecl /*C_2276*/Gra_DotXor(/*int, int, int*/);
extern int16 cdecl /*C_227B*/Gra_LoadPicture(/*char *, int, int*/);
extern int16 cdecl /*C_2280*/Gra_RevealCodex();
extern int16 cdecl /*C_2297*/Gra_DrawQuad(/**/);
extern int16 cdecl /*C_229C*/Gra_VirtueGlyph();
extern int16 cdecl /*C_22A7*/Gra_AnimCampfire();
extern int16 cdecl /*C_22AC*/Gra_Init(/*char *, char *, callback*/);
extern int16 cdecl /*C_22B1*/Gra_ClearScreen();
extern int16 /*C_22B6*/Gra_AnimFlow(uint16);/*bug?*/
extern int16 /*C_22BB*/Gra_AnimFlags();
extern int16 /*C_22C0*/Gra_PutChar(char);
extern int16 /*C_22CF*/InterleaveShapes();
extern int16 /*C_2341*/InterleaveCharset();

#endif
