#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

Parser::Parser() {
  thresholdLBP = 0.3;
  winSize = 20;
  thresholdDiff = 80;
  alarmValue = 60;
}

double Parser::getThresholdLBP() {
  return thresholdLBP;
}

int Parser::getThresholdDiff() {
  return thresholdDiff;
}

int Parser::getWinSize() {
  return winSize;
}

int Parser::getAlarmValue() {
  return alarmValue;
}

int Parser::getDiff() {
  return diff;
}

int Parser::getMethod() {
  return method;
}

int Parser::parseLine(std::string line, int lineNum) {
  int start = 0,
      length = -1;
  std::string word;

  while(isspace(line[start])) start++;
  length = start;
  while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;

  length -= start;

  if (length == 0) {
    return 0;
  }

  word = line.substr(start, length);

  if(word[0]=='#') {
    return 0;
  }

  if(word.compare("thresholdLBP")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->thresholdLBP = atof(word.c_str());
  } else if (word.compare("winSize")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->winSize = atoi(word.c_str());
  } else if (word.compare("diff")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->diff = atoi(word.c_str());
  } else if (word.compare("method")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->method = atoi(word.c_str());
  } else if (word.compare("thresholdDiff")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->thresholdDiff = atoi(word.c_str());
  } else if (word.compare("alarmValue")==0) {
    start += length;
    while(isspace(line[start])) start++;
    length = start;
    while(!isspace(line[length]) && length < static_cast<int>(line.size())) length++;
    word = line.substr(start, length);
    this->alarmValue = atoi(word.c_str());
  }else {
    std::cerr << "Unknown word " << word << " on line " << lineNum << std::endl;
  }
  return 0;
}

int Parser::parse(const char *filename) {
  std::ifstream file;
  file.open(filename, std::ifstream::in);                    //otevre konfiguracni soubor
  char line[MAXLINELENGTH];
  std::string strLine;
  int lineNum = 1;

  if (file.fail()) {
    std::cerr << "Nejde kurva otevrit soubor\n";
    return 1;
  }

  while (file.getline(line,MAXLINELENGTH)) {              //nacetl dalsi radek
    strLine.assign(line);                              //priradi do stringu
    if (parseLine(strLine, lineNum)!=0) {
      std::cerr << "Jakasi chyba na radku " << lineNum << std::endl;
    }
    lineNum++;
  }

  file.close();     //zavre konfiguracni soubor
  return 0;
}
