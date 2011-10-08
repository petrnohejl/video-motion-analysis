#ifndef __PARSER_H__
#define __PARSER_H__

#define MAXLINELENGTH 512

class Parser {
    double thresholdLBP;
    int thresholdDiff;
    int winSize;
    int alarmValue;
    char *filename;
    int diff;
    int method;

    int parseLine(std::string line, int lineNum);

  public:
    Parser();
    int parse(const char *filename);
    double getThresholdLBP();
    int getThresholdDiff();
    int getWinSize();
    int getAlarmValue();
    int getDiff();
    int getMethod();
    
};

#endif
