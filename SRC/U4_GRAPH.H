/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */
#ifndef __U4_GRAPH_H__
#define __U4_GRAPH_H__

extern /*C_20C1*/LoadGraphicsDriver(char *);
extern /*C_213B*/DrawGameFrame();
extern cdecl Gra_Line(int16, int16, int16, int16, int16);
extern cdecl /*C_2201*/Gra_ActiveBox(int16, int16);
extern cdecl /*C_2206*/Gra_ZoomTileRow(/**/);
extern cdecl /*C_220B*/Gra_FillTiles(/**/);
extern cdecl /*C_2210*/Gra_ShiftDown();
extern cdecl /*C_221B*/Gra_ShiftUp();
extern cdecl /*C_2226*/Gra_DrawTiles(/*int h, int w, void *src, int dst_ofs*/);
extern cdecl /*C_222B*/Gra_Dot(/*int, int, int*/);
extern /*C_2230*/Gra_CR();
extern /*C_2241*/Gra_InvertView();
extern /*C_2246*/Gra_ClearView();
extern /*C_224B*/Gra_InvertStatusLine(int16);
extern cdecl /*C_2250*/Gra_InvertChar(int16, int16);/*(uncalled)*/
extern /*C_226C*/Gra_ClearStatus();
extern cdecl /*C_2271*/Gra_DotOr(/*int, int, int*/);
extern cdecl /*C_2276*/Gra_DotXor(/*int, int, int*/);
extern cdecl /*C_227B*/Gra_LoadPicture(/*char *, int, int*/);
extern cdecl /*C_2280*/Gra_RevealCodex();
extern cdecl /*C_2297*/Gra_DrawQuad(/**/);
extern cdecl /*C_229C*/Gra_VirtueGlyph();
extern cdecl /*C_22A7*/Gra_AnimCampfire();
extern cdecl /*C_22AC*/Gra_Init(/*char *, char *, callback*/);
extern cdecl /*C_22B1*/Gra_ClearScreen();
extern /*C_22B6*/Gra_AnimFlow(uint16);/*bug?*/
extern /*C_22BB*/Gra_AnimFlags();
extern /*C_22C0*/Gra_PutChar(char);
extern /*C_22CF*/InterleaveShapes();
extern /*C_2341*/InterleaveCharset();

#endif
