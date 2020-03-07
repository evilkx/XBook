/*
 * file:		include/lib/math.h
 * auther:		Jason Hu
 * time:		2019/6/27
 * copyright:	(C) 2018-2020 by Book OS developers. All rights reserved.
 */

#ifndef _LIB_MATH_H
#define _LIB_MATH_H

/* max() & min() */
#define	MAX(a,b)	((a) > (b) ? (a) : (b))
#define	MIN(a,b)	((a) < (b) ? (a) : (b))

/* 除后上入 */
#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

/* 除后下舍 */
#define DIV_ROUND_DOWN(X, STEP) ((X) / (STEP))

int min(int a, int b);
int max(int a, int b);
int abs(int n);
double pow(double x, double y);
int ipow (double x,int y);
double exp (double x);
double ln (double x);
double sin (double x);
double cos(double x);
double tan(double x);
double fabs (double x);
double sqrt (double x);
#endif /* _LIB_MATH_H */
