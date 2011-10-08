#include <iostream>
#include "erdil.h"
#include "detector.h"

#define alpha 0.95f
#define LBPsize 3
#define histSize 256

Detector::Detector(bool lbp, bool diff, bool er) {
  this->lbp = lbp;
  this->diff = diff;
  this->showEr = er;
  thresholdLBP = 0.2;
  thresholdDiff = 75;
  winSize = 15;
  average = NULL;
  grayFrame = NULL; // Obrazek v odstinech sedi
  LBPframe = NULL;  // Obrazek pro ulozeni LBP priznaku
  frame = NULL;   // Ziskani obrazu z kamery
  resultImg = NULL; // Kopie obrazu pro zobrazeni vysledku
  averageImg = NULL;    // Obrazek pro pocitani prumeru predchozich snimku
  difference = NULL;  // Obrazek pro ulozeni rozdilu aktualniho snimku s predchozimi
  grayDiff = NULL;
  alarmFurCobra11 = 0;
  diff = 2;
  method = 0;
  outputFile = NULL;
  showWin = true;
  isAlarm = false;
}

void Detector::setDiff(int val) {
  differdil = val;
}

void Detector::setWindow(bool val) {
  showWin = val;
}

void Detector::setOutput(char *file) {
  outputFile = file;
}

void Detector::setMethod(int val) {
  method = val;
}

void Detector::setThresholdDiff(int val) {
  thresholdDiff = val;
}

void Detector::setAlarm(int val) {
  alarmValue = val;
}

void Detector::setThresholdLBP(double val) {
  thresholdLBP = val;
}

void Detector::setWinSize(int val) {
  winSize = val;
}

// Prevedeni obrazku na obrazek LBP priznaku
void Detector::convertToLBP(IplImage *frame, IplImage *LBPframe)
{
  int x, y, dc, dr, c, r, step, mult, pixel, pixelC, pixelLBP, radius = (LBPsize-1)/2;

  for (int row = 0; row < frame->height; row++){
    for (int col = 0; col < frame->width; col++){

      dc = 1;
      dr = 0;
      c = -1;
      r = 0;
      step = LBPsize;
      mult = 1;

      pixelLBP = 0;
      // Ziskani stredoveho pixelu
      pixelC = ((uchar *)(frame->imageData + row*frame->widthStep))[col];

      // Pruchod okoli pixelu (po smeru hodinovych rucicek)
      for (int j = 0; j < 4; j++){
        // Pruchod v jednom smeru
        for (int i = 0; i < step; i++){
          c += dc;
          r += dr;
          // Vypocet pozice pixelu
          x = col+c-radius;
          y = row+r-radius;
          // Osetreni pristupu mimo obrazek
          if (x < 0) x = 0;
          if (x > frame->width-1) x = frame->width-1;
          if (y < 0) y = 0;
          if (y > frame->height-1) y = frame->height-1;
          // Ziskani pixelu z okoli
          pixel = ((uchar *)(frame->imageData + y*frame->widthStep))[x];
          // Vypocet LBP
          if (pixelC <= pixel){
            pixelLBP += mult;
          }
          mult *= 2;
        }
        step -= abs(dc);

        // Zmena smeru
        int tmpDR = dr;
        dr = dc;
        dc = -tmpDR;
      }
      ((uchar *)(LBPframe->imageData + row*LBPframe->widthStep))[col] = pixelLBP;
    }
  }
}

// Ziskani histogramu pro jedno okno
CvHistogram* Detector::getOneHist(int winX, int winY, IplImage *frame)
{
  int offsetX = winX*winSize;
  int offsetY = winY*winSize;
  int winSizeX, winSizeY;

  // Pokud by okno zasahovalo mimo obrazek, musime prepocitat jeho velikost
  if (frame->width-offsetX < winSize) winSizeX = frame->width-offsetX;
  else winSizeX = winSize;
  if (frame->height-offsetY < winSize) winSizeY = frame->height-offsetY;
  else winSizeY = winSize;

  // Vytvoreni obrazku okna
  IplImage *windowImg = cvCreateImage(cvSize(winSizeX,winSizeY), frame->depth, 1);
  for (int y = 0; y < winSizeY; y++){
    for (int x = 0; x < winSizeX; x++){
      ((uchar *)(windowImg->imageData + y*windowImg->widthStep))[x] =
        ((uchar *)(frame->imageData + (y+offsetY)*frame->widthStep))[x+offsetX];
    }
  }

  IplImage* images[] = { windowImg };

  // Velikost histogramu
  int bins = histSize;
  int hsize[] = {bins};

  // Rozsahy
  float xranges[] = { 0, histSize };
  float* ranges[] = { xranges };

  CvHistogram* hist = cvCreateHist(1, hsize, CV_HIST_ARRAY, ranges, 1);
  cvCalcHist(images, hist, 0, NULL);

  cvReleaseImage(&windowImg);
  //delete[] images;
  return hist;
}

// Ziskani pole histogramu z celeho obrazku
void Detector::getHistograms(IplImage *frame, CvHistogram** histograms, int numWinX, int numWinY)
{
  // Pruchod vsemi okny
  for (int y = 0; y < numWinY; y++){
    for (int x = 0; x < numWinX; x++){
      histograms[y*numWinX+x] = getOneHist(x, y, frame);
    }
  }
}

// Kopirovani pole histogramu
void Detector::copyHistograms(CvHistogram** histSrc, CvHistogram** histDst, int numWinX, int numWinY)
{
  for (int y = 0; y < numWinY; y++){
    for (int x = 0; x < numWinX; x++){
      cvCopyHist(histSrc[y*numWinX+x], &histDst[y*numWinX+x]);
    }
  }
}

// Zmena prumeru predchozich snimku
void Detector::changeAverage(CvHistogram** average, CvHistogram** histograms, int numWinX, int numWinY)
{
  // Pruchod polem histogramu
  for (int y = 0; y < numWinY; y++){
    for (int x = 0; x < numWinX; x++){
      // Pruchod histogramem
      for (int i = 0; i < histSize; i++){
        int pos = y*numWinX+x;
        *cvGetHistValue_1D(average[pos], i) =
          alpha * (*cvGetHistValue_1D(average[pos], i)) + (1 - alpha) * (*cvGetHistValue_1D(histograms[pos], i));
      }
    }
  }
}

// Zmeni jedno okno vysledneho obrazku (zacerni dane okno)
void Detector::changeOneWindow(IplImage* resultImg, int winX, int winY)
{
  int offsetX = winX*winSize;
  int offsetY = winY*winSize;
  int winSizeX, winSizeY;

  // Pokud by okno zasahovalo mimo obrazek, musime prepocitat jeho velikost
  if (resultImg->width-offsetX < winSize) winSizeX = resultImg->width-offsetX;
  else winSizeX = winSize;
  if (resultImg->height-offsetY < winSize) winSizeY = resultImg->height-offsetY;
  else winSizeY = winSize;

  for (int y = 0; y < winSizeY; y++){
    for (int x = 0; x < winSizeX; x++){
//       ((uchar *)(resultImg->imageData + (y+offsetY)*resultImg->widthStep))[(x+offsetX)*resultImg->nChannels + 0] = 255; // B
       ((uchar *)(resultImg->imageData + (y+offsetY)*resultImg->widthStep))[(x+offsetX)*resultImg->nChannels + 1] = 255; // G
      //((uchar *)(resultImg->imageData + (y+offsetY)*resultImg->widthStep))[(x+offsetX)*resultImg->nChannels + 2] = 255; // R
    }
  }
}

// Funkce pro porovnani histogramu
void Detector::getDifference(CvHistogram** average, CvHistogram** histograms, int numWinX, int numWinY, IplImage* resultImg)
{
  alarmFurCobra11 = 0;
  // Pruchod polem histogramu
  for (int y = 0; y < numWinY; y++){
    for (int x = 0; x < numWinX; x++){
      int pos = y*numWinX+x;
      // Vytvoreni pomocnych histogramu
      CvHistogram* normalizedA = NULL;
      CvHistogram* normalizedH = NULL;
      cvCopyHist(average[pos], &normalizedA);
      cvCopyHist(histograms[pos], &normalizedH);
      // Normalizace histogramu
      cvNormalizeHist(normalizedA, 1);
      cvNormalizeHist(normalizedH, 1);
      // Porovnani dvou histogramu
      double cmp = cvCompareHist(normalizedA, normalizedH, CV_COMP_CHISQR);
      // Dalsi zpusoby porovnani z openCV - CV_COMP_CORREL, CV_COMP_CHISQR, CV_COMP_INTERSECT, CV_COMP_BHATTACHARYYA
      if (cmp >= thresholdLBP) {
        changeOneWindow(resultImg, x, y);
        alarmFurCobra11++;
      }

      cvReleaseHist(&normalizedA);
      cvReleaseHist(&normalizedH);
    }
  }
}

void Detector::threshold(IplImage *frame)
{
  CvScalar pixel;
  for(int x = 0; x < frame->height; x++){
    for(int y = 0; y < frame->width; y++){
      pixel = cvGet2D(frame, x, y);
      if (pixel.val[0] >= thresholdDiff) pixel.val[0] = 255;
      else pixel.val[0] = 0;
      cvSet2D(frame, x, y, pixel);
    }
  }
}

//Zmena prumeru predchozich snimku
void Detector::changeAverageImg(IplImage *average, IplImage *frame)
{
  CvScalar pixelA;
  CvScalar pixelF;
  for(int x = 0; x < frame->height; x++){
    for(int y = 0; y < frame->width; y++){
      pixelA = cvGet2D(average, x, y);
      pixelF = cvGet2D(frame, x, y);
      for(int i = 0; i < frame->nChannels; i++){
        pixelA.val[i] = alpha*pixelA.val[i] + (1-alpha)*pixelF.val[i];
      }
      cvSet2D(average, x, y, pixelA);
    }
  }
}

//Vypocita rozdil mezi soucasnym snimkem a prumeram predchozich snimku
IplImage *Detector::getDifferenceImg(IplImage *average, IplImage *frame)
{
  IplImage *difference = cvCloneImage(frame);
  CvScalar pixelD;
  CvScalar pixelA;

  for (int x = 0; x < frame->height; x++){
    for (int y = 0; y < frame->width; y++){
      pixelD = cvGet2D(difference, x, y);
      pixelA = cvGet2D(average, x, y);
      for (int i = 0; i < frame->nChannels; i++){
        pixelD.val[i] = abs(pixelD.val[i] - pixelA.val[i]);
      }
      cvSet2D(difference, x, y, pixelD);
    }
  }
  return difference;
}

void Detector::getLBP(bool firstTime) {
  if (resultImg != NULL) {
    cvReleaseImage(&resultImg);
    resultImg = NULL;
  }

  if (grayFrame != NULL) {
    cvReleaseImage(&grayFrame);
    grayFrame = NULL;
  }

  if (LBPframe != NULL) {
    cvReleaseImage(&LBPframe);
    LBPframe = NULL;
  }
  resultImg = cvCloneImage(frame);  // Kopie obrazu pro zobrazeni vysledku
  if (firstTime){ // Prvni pruchod cyklem
    // Pocet oken v ose x
    numWinX = (int) ceil((float)frame->width/(float)winSize);
    // Pocet oken v ose y
    numWinY = (int) ceil((float)frame->height/(float)winSize);
    average = new CvHistogram*[numWinX*numWinY];
    for (int i = 0; i < numWinX*numWinY; i++) {
      average[i] = NULL;
    }
  }

  // Prevedeni obrazku do stupne sedi
  grayFrame = cvCreateImage(cvGetSize(frame), frame->depth, 1);
  cvCvtColor(frame,grayFrame,CV_BGR2GRAY);

  // Ve windowsu se obraz po prevodu obratil, proto ho tady obracim zpet
  #if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__)
  cvFlip(grayFrame,grayFrame,0);
  #endif

  // Prevedeni obrazku (v odstinech sedi) na LBP priznaky
  LBPframe = cvCreateImage(cvGetSize(grayFrame), grayFrame->depth, 1);
  convertToLBP(grayFrame, LBPframe);

  // Prevedeni obrazku (LBP priznaku) na histogramy
  CvHistogram** histograms = new CvHistogram*[numWinX*numWinY];
  getHistograms(LBPframe, histograms, numWinX, numWinY);

  if (firstTime){ // V prnim pruchodu bude avreage pouze kopii histogramu
    copyHistograms(histograms, average, numWinX, numWinY);
  }

  getDifference(average, histograms, numWinX, numWinY, resultImg);
  changeAverage(average, histograms, numWinX, numWinY);

  // Uvolneni pameti
  for(int i=0;i<numWinX*numWinY;i++) {
    cvReleaseHist(&histograms[i]);
    histograms[i] = NULL;
  }
  delete[] histograms;
}

int Detector::detect(char *video) {
  IplImage *res;
  CvCapture* capture;
  bool firstTime = true;
  // Ukládač videa.
  CvVideoWriter *AVIWriter = NULL;
  double fps = 25.0;
  if (outputFile == NULL) {
    showWin = true;
  }

  
//   std::cout << "ThresholdLBP: " << thresholdLBP;
//   std::cout << "\nWindow size: " << winSize;
//   std::cout << "\nthresholdDiff: " << thresholdDiff;
//   std::cout << "\nAlarmValue: " << alarmValue;
//   std::cout << "\nDiff: " << differdil;
//   std::cout << "\nMethod: " << method << std::endl;

  
  // Otevreni videa
  if (video == NULL) {
    capture = cvCaptureFromCAM(-1);
//     std::cout << "Video je z webkamery\n";
  }
  else {
    capture = cvCaptureFromAVI(video);
//     std::cout << "Video je ze souboru " << video << std::endl;
  }

  fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
//   std::cout << "FPSka: " << fps << std::endl;

  if (showWin) {
    cvNamedWindow("Original", 1); // Vytvoreni pojmenovaneho okna
  }

  if (lbp && showWin) {
    cvNamedWindow("LBP motion detection", 1); // Vytvoreni pojmenovaneho okna
    cvNamedWindow("LBP", 1); // Vytvoreni pojmenovaneho okna
  }

  if (diff && showWin) {
    cvNamedWindow("Diff mask", 1); // Vytvoreni pojmenovaneho okna
    cvNamedWindow("Difference method motion detection", 1); // Vytvoreni pojmenovaneho okna
    if (showEr) {
      cvNamedWindow("Difference method motion detection without erosion", 1); // Vytvoreni pojmenovaneho okna
    }
  }

  if (diff && lbp && showWin) {
    cvNamedWindow("The result", 1); // Vytvoreni pojmenovaneho okna
  }

  while (capture != NULL)
  {
    if (!cvGrabFrame(capture)) {
      break;    // Rychle zachyceni obrazu z kamery
    }

    frame = cvRetrieveFrame(capture); // Ziskani obrazu z kamery
    if (showWin) {
      cvShowImage("Original", frame);
    }

    if (outputFile != NULL) {
      if( AVIWriter == NULL){
        AVIWriter = cvCreateVideoWriter( outputFile, CV_FOURCC('M','J','P','G'),
        fps, cvSize( frame->width , frame->height), 1);
      }
    }

    if (lbp) {
      getLBP(firstTime);
      if (showWin) {
        cvShowImage("LBP", LBPframe);
        cvShowImage("LBP motion detection", resultImg);
      }
      if (alarmFurCobra11*100 > ceil(numWinX*numWinY*alarmValue)) {
        if (!isAlarm) {
        #if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__)
          PlaySound(,NULL, SND_ASYNC);
        #else
          std::cout << "Alarm für Cobra 11\n";
        #endif
        }
        isAlarm = true;
      }
      else {
        isAlarm = false;
      }

      if (!diff && outputFile != NULL && AVIWriter != NULL) {
        cvWriteFrame( AVIWriter, resultImg);
      }
    }
    if (diff) {
      getDiff(firstTime);
      if (showWin) {
        cvShowImage("Diff mask", grayDiff);
        cvShowImage("Difference method motion detection", diffImg);
      }
      if (!lbp && outputFile != NULL && AVIWriter != NULL) {
        cvWriteFrame( AVIWriter, diffImg);
      }
      cvReleaseImage(&diffImg);
    }

    if (diff && lbp) {
      res = cvCloneImage(frame);
      for(int x=0; x<res->width; x++) {
        for(int y=0; y<res->height; y++) {
          if (((uchar *)(grayDiff->imageData + y*grayDiff->widthStep))[x*grayDiff->nChannels] == 255 ||
              ((uchar *)(resultImg->imageData + y*resultImg->widthStep))[x*resultImg->nChannels + 1] == 255) {
            ((uchar *)(res->imageData + y*res->widthStep))[x*res->nChannels + 1] = 255;
          }
        }
      }
      if (showWin) {
        cvShowImage("The result", res);
      }
      if (outputFile != NULL && AVIWriter != NULL) {
        cvWriteFrame( AVIWriter, res);
      }
      cvReleaseImage(&res);
    }
    
    // Pocka 10 ms na libovolnou klavesu
    if (cvWaitKey(10) >= 0) {
      break;
    }

    if (firstTime) {
      firstTime = false;
    }
  } // END OF WHILE

  if (lbp) {
    for(int i=0;i<numWinX*numWinY;i++) {
      cvReleaseHist(&average[i]);
    }

    delete[] average;
    if (showWin) {
      cvDestroyWindow("LBP motion detection");
      cvDestroyWindow("LBP");
    }
    if (resultImg != NULL) {
      cvReleaseImage(&resultImg);
      resultImg = NULL;
    }

    if (grayFrame != NULL) {
      cvReleaseImage(&grayFrame);
      grayFrame = NULL;
    }

    if (LBPframe != NULL) {
      cvReleaseImage(&LBPframe);
      LBPframe = NULL;
    }
  }

  if (showWin) {
    cvDestroyWindow("Original");
  }

  if (diff) {
    if (showWin) {
      cvDestroyWindow("Diff mask");
      cvDestroyWindow("Difference method motion detection");
      cvReleaseImage(&averageImg);
      if (showEr) {
        cvDestroyWindow("Difference method motion detection without erosion");
      }
    }
    if (grayDiff != NULL) {
      cvReleaseImage(&grayDiff);
      grayDiff = NULL;
    }
  }

  if (diff && lbp && showWin) {
    cvDestroyWindow("The result"); // Vytvoreni pojmenovaneho okna
  }

  if (AVIWriter != NULL) {
    cvReleaseVideoWriter(&AVIWriter);
  }
  cvReleaseCapture(&capture);

  return 0;
}

void Detector::getDiff(bool firstTime) {
  diffImg = cvCloneImage(frame);

  if (grayDiff != NULL) {
    cvReleaseImage(&grayDiff);
    grayDiff = NULL;
  }
  
  // V prvnim pruchodu cyklem bude average stejne jako prvni frame
  if (firstTime){
    averageImg = cvCloneImage(frame);
  }

  difference = getDifferenceImg(averageImg, frame);
  changeAverageImg(averageImg, frame);

    // Prevedeni obrazku do stupne sedi
  grayDiff = cvCreateImage(cvGetSize(difference), difference->depth, 1);
  cvCvtColor(difference,grayDiff,CV_BGR2GRAY);
  cvReleaseImage(&difference);
    // Ve windowsu se obraz po prevodu obratil, proto ho tady obracim zpet
  #if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__)
    cvFlip(grayDiff,grayDiff,0);
  #endif
  threshold(grayDiff);
  if (showEr && showWin) {
    cvShowImage("Difference method motion detection without erosion", grayDiff);
  }
  // odstrani male bile objekty
  erosion(grayDiff, diff, method);
  dilatation(grayDiff, diff, method);
  // odstrani dirky a zalivy v bilem objektu
  dilatation(grayDiff, diff, method);
  erosion(grayDiff, diff, method);
  for(int x=0; x<grayDiff->width; x++) {
    for(int y=0; y<grayDiff->height; y++) {
      if (((uchar *)(grayDiff->imageData + y*grayDiff->widthStep))[x*grayDiff->nChannels] == 255) {
        ((uchar *)(diffImg->imageData + y*diffImg->widthStep))[x*diffImg->nChannels + 1]  = 255;
      }
    }
  }
}
