#ifndef __DETECTOR_H__
#define __DETECTOR_H__

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>

class Detector {
    double thresholdLBP;
    int thresholdDiff;
    int winSize;
    bool lbp;
    bool diff;
    bool showEr;
    bool showWin;
    bool isAlarm;
    CvHistogram** average; // Prumerna hodnota histogramu z predchozich snimku
    int numWinX, numWinY;   // Pocet oken v ose x a y
    int alarmFurCobra11;
    int alarmValue;
    // dilatace a eroze
    int differdil;
    int method; // 0 ctverec jinak kriz
    char *outputFile;

    IplImage *grayFrame; // Obrazek v odstinech sedi
    IplImage *LBPframe;  // Obrazek pro ulozeni LBP priznaku
    IplImage *frame;   // Ziskani obrazu z kamery
    IplImage *resultImg; // Kopie obrazu pro zobrazeni vysledku
    IplImage *averageImg;    // Obrazek pro pocitani prumeru predchozich snimku
    IplImage *difference;  // Obrazek pro ulozeni rozdilu aktualniho snimku s predchozimi
    IplImage *grayDiff;
    IplImage *diffImg;
  
    // Prevedeni obrazku na obrazek LBP priznaku
    void convertToLBP(IplImage *frame, IplImage *LBPframe);
    // Ziskani histogramu pro jedno okno
    CvHistogram* getOneHist(int winX, int winY, IplImage *frame);
    // Ziskani pole histogramu z celeho obrazku
    void getHistograms(IplImage *frame, CvHistogram** histograms, int numWinX, int numWinY);
    // Kopirovani pole histogramu
    void copyHistograms(CvHistogram** histSrc, CvHistogram** histDst, int numWinX, int numWinY);
    // Zmena prumeru predchozich snimku
    void changeAverage(CvHistogram** average, CvHistogram** histograms, int numWinX, int numWinY);
    // Zmeni jedno okno vysledneho obrazku (zacerni dane okno)
    void changeOneWindow(IplImage* resultImg, int winX, int winY);
    // Funkce pro porovnani histogramu
    void getDifference(CvHistogram** average, CvHistogram** histograms, int numWinX, int numWinY, IplImage* resultImg);
    //
    void threshold(IplImage *frame);
    void changeAverageImg(IplImage *average, IplImage *frame);
    IplImage *getDifferenceImg(IplImage *average, IplImage *frame);

  public:
    Detector(bool lbp, bool diff, bool er);
    int detect(char *video);
    void getLBP(bool firsttime);
    void getDiff(bool firsttime);
    void setThresholdLBP(double val);
    void setThresholdDiff(int val);
    void setAlarm(int val);
    void setWinSize(int val);
    void setDiff(int val);
    void setMethod(int val);
    void setOutput(char *file);
    void setWindow(bool val);
};

#endif
