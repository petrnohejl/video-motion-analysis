#ifndef __ERDIL_H__
#define __ERDIL_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

void dilatation(IplImage *out, int diff, int method);
void erosion(IplImage *out, int diff, int method);

#endif
