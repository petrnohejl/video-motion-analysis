#include <iostream>
//#include <unistd.h>
#include "parser.h"
#include "detector.h"


#define configFile "zpo.cfg"

using namespace std;

const char *usage = "usage: ./metecion (-d|-l [-n]) [-i videoInput] [-o videoOutput] [-h|--help] \n\
\twhere\t-d is difference method detection\n\
\t\t-l is LBP method detecion\n\
\t\t-n shows window without erosion using\n\
\t\t-i videoInput specifies video that detection will be performed on\n\
\t\t-o videoOutput defines file which detection result will be saved in\n\
\t\t-w doesn't show windows, can be used only when -o is defined\n\
\t\t-h or --help prints this usage\n\
\nThe both parametres -d and -l can be used at the same time.";

int main(int argc, char** argv)
{
  char *video = NULL;
  char *outputFile = NULL;
  int opt;
  Parser parser;
  parser.parse(configFile);
  bool lbp = false;
  bool diff = false;
  bool er = false;
  bool show = true;
  
  while ((opt = getopt(argc, argv, "wo:ldnhi:-:")) != -1) {
    switch(opt) {
      case 'h' :
        std::cout << usage << std::endl;
        return EXIT_SUCCESS;
      case 'i' :
        video = optarg;
        break;
      case 'o' :
        outputFile = optarg;
        break;
      case 'l' :
        lbp = true;
        break;
      case 'd' :
        diff = true;
        break;
      case 'w' :
        show = false;
        break;
      case 'n' :
        er = true;
        break;
      case '-' :
        if (strcmp(optarg,"help")==0) {
          std::cout << usage << std::endl;
          return EXIT_SUCCESS;
        }
        else {
          std::cerr << "Unknown parameter " << optarg << std::endl;
          return EXIT_FAILURE;
        }
        break;
      default : return EXIT_FAILURE;
    }
  }

  if (!lbp && !diff) {
    std::cout << "You have to specify detection method! (-l or -d)\n" << usage << std::endl;
    return 0;
  }

  Detector *detector;
  int retval;
  detector = new Detector(lbp, diff, er);
  detector->setThresholdLBP(parser.getThresholdLBP());
  detector->setThresholdDiff(parser.getThresholdDiff());
  detector->setWinSize(parser.getWinSize());
  detector->setAlarm(parser.getAlarmValue());
  detector->setDiff(parser.getDiff());
  detector->setMethod(parser.getMethod());
  detector->setOutput(outputFile);
  detector->setWindow(show);

  retval = detector->detect(video);

  delete detector;

	return retval;
}
