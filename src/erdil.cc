#include "erdil.h"

// Dilatace - frame, diference, metoda (0 je ctverec else kriz)
void dilatation(IplImage *out, int diff, int method)
{
  IplImage *in = cvCloneImage(out);

  // pruchod framebufferem
  for (int y = diff; y < (in->height)-diff; y++)
  {
    for (int x = diff; x < (in->width)-diff; x++)
    {
      // hodnota aktualniho pixelu
      uchar value = ((uchar *)(in->imageData + y*in->widthStep))[x];

      // pixel je zkoumanym objektem
      if(value==255)
      {
        // ctverec
        if(method==0)
        {
          for(int j=-diff;j<=diff;j++)
          for(int i=-diff;i<=diff;i++)
          ((uchar *)(out->imageData + (y+j)*out->widthStep))[x+i] = 255;
        }
        // kriz
        else
        {
          for(int i=-diff;i<=diff;i++)
          {
            ((uchar *)(out->imageData + (y)*out->widthStep))[x+i] = 255;
            ((uchar *)(out->imageData + (y+i)*out->widthStep))[x] = 255;
          }
        }
      }
    }
  }

  cvReleaseImage(&in);
}



// Eroze - frame, diference, metoda (0 je ctverec else kriz)
void erosion(IplImage *out, int diff, int method)
{
  IplImage *in = cvCloneImage(out);

  // pruchod framebufferem
  for (int y = diff; y < (in->height)-diff; y++)
  {
    for (int x = diff; x < (in->width)-diff; x++)
    {
      bool fill = true;

      // ctverec
      if(method==0)
      {
        for(int j=-diff;j<=diff;j++)
        for(int i=-diff;i<=diff;i++)
        {
          if(((uchar *)(in->imageData + (y+j)*in->widthStep))[x+i] == 0)
          {
            fill = false;
            break;
          }
        }
      }
      // kriz
      else
      {
        for(int i=-diff;i<=diff;i++)
        {
          if(((uchar *)(in->imageData + (y)*in->widthStep))[x+i] == 0 ||
             ((uchar *)(in->imageData + (y+i)*in->widthStep))[x] == 0)
          {
            fill = false;
            break;
          }
        }
      }

      if(fill) ((uchar *)(out->imageData + y*out->widthStep))[x] = 255;
      else ((uchar *)(out->imageData + y*out->widthStep))[x] = 0;
    }
  }

  cvReleaseImage(&in);
}