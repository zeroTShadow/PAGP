#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <iostream>
#include <queue>

/******************************************************************************/
struct axialOffsets3d {
  int xOffset;
  int yOffset;
  int zOffset;
};

struct axialOffsets2d {
  int yzOffset;
  int xOffset;
};

struct lateralImageProduct {
  std::vector<std::vector<int> > rescaledXY;
  std::vector<double> magMap;
};

struct MappingVector2d {
  std::vector<std::vector<axialOffsets2d> > dimOffset;
};

struct StageMap {
  std::vector<std::vector<std::vector<axialOffsets3d> > > stagemap;
};

/******************************************************************************/
void PullLateralImage(int, lateralImageProduct*);
double FindRealHeightRatio(int rowLow);
void setOffsets (lateralImageProduct*, MappingVector2d*, MappingVector2d*, int);
void renderOffset(lateralImageProduct*, MappingVector2d*);
void stageMapper (StageMap*, MappingVector2d*, MappingVector2d*);

/******************************************************************************/
#include "setOffsets.cpp"
#include "StageMapper.cpp"
