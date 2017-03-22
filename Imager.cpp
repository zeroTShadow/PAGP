// #include "opencv2/opencv.hpp"
// #include <stdio.h>
// #include <iostream>
#include "Imager.h"
// #include "setOffsets.cpp"
// #include "StageMapper.cpp"

using namespace cv;

// void PullLateralImage(int, lateralImageProduct*);
// double FindRealHeightRatio(int rowLow);
// void setOffsets (lateralImageProduct* lateralProduct, MappingVector2d* xy, MappingVector2d* xz, int cam);
// void stageMapper (StageMap*, MappingVector2d* xy, MappingVector2d* xz);

// int main(int, char**)
// {
//   lateralImageProduct sideCameraMapping;
//   PullLateralImage(1, &sideCameraMapping);
//
//   MappingVector2d xyMap, xzMap;
//   setOffsets(&sideCameraMapping, &xyMap, &xzMap, 2);
//
//   StageMap env;
//   stageMapper(&env, &xyMap, &xzMap);
//   /*****************************************************************************
//    * THE BELOW IS DEBUG CODE--ONLY USE WHEN ABSOLUTELY NECESSARY AS IT IS HIGHLY
//    * INTENSIVE. 3D VECTORS ARE NOT MEANT TO BE ITERATED THROUGH, ESPECIALLY NOT
//    * WHEN THAT ITERATION INCLUDES PRINT CALLS. SERIOUSLY, ONLY USE TO DEBUG--
//    * THIS BIT IS REALLY, REALLY GROSS.
//    *****************************************************************************
//    */
//   // int height = env.stagemap.size();
//   // int width = env.stagemap.at(0).size();
//   // int depth = env.stagemap.at(0).at(0).size();
//   // std::cout<<height<<"x"<<width<<"x"<<depth<<std::endl;
//   // for(int i = 0; i <= height - 1; i++) {
//   //   for(int j = 0; j <= width - 1; j++) {
//   //     for(int k = 0; k <= depth - 1; k++) {
//   //       if(env.stagemap.at(i).at(j).at(k).xOffset > 0 || env.stagemap.at(i).at(j).at(k).yOffset > 0 || env.stagemap.at(i).at(j).at(k).zOffset > 0) {
//   //         printf("%d,%d,%d\n",i,j,k);
//   //       }
//   //     }
//   //   }
//   // }
//
//   return 0;
// }

// magnification ratio = 0.0044972114x + 1.0054352602 | x <- pixel offset from bottom of image
// derived by hand and estimated
/*
* Returns magnification for true size at this pixel "distance"
*/
double FindRealHeightRatio(int rowLow){
  double magRatio = 0.0045*(double)rowLow + 1.0054;
  return magRatio;
}

void PullLateralImage (int cam, lateralImageProduct* newProduct) {
  VideoCapture cap(cam); // open the camera
  // if(!cap.isOpened())  // check if we succeeded
  //     return null;

  Mat frame, edges0, edges1, edges2, output, tmpA, tmpB;
  // map1, map2;

  cap >> frame; // get a new frame from camera
  cvtColor(frame, edges0, CV_BGR2GRAY);
  /*************************************************
  * These are hardcoded values to remove distortion effect
  * of the camera lens.
  *
  * These values could be generated automatically upon
  * initialization using known markers on the stage space.
  * See opencv's calibration tutorial source code for the
  * xml/yml matrix document generation scheme.
  *************************************************/
  // Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
  // cameraMatrix.at<double>(0,0) = 488.815627795;
  // cameraMatrix.at<double>(0,1) = 0.;
  // cameraMatrix.at<double>(0,2) = 319.5;
  // cameraMatrix.at<double>(1,0) = 0.;
  // cameraMatrix.at<double>(1,1) = 488.81562779465474;
  // cameraMatrix.at<double>(1,2) = 239.5;
  // cameraMatrix.at<double>(2,0) = 0.;
  // cameraMatrix.at<double>(2,1) = 0.;
  // cameraMatrix.at<double>(2,2) = 1.;
  //
  // Mat distCoeffs = Mat::zeros(5, 1, CV_64F);
  // distCoeffs.at<double>(0,0) = -0.69299687973847002;
  // distCoeffs.at<double>(0,1) = 5.3149622417099929;
  // distCoeffs.at<double>(0,2) = 0.;
  // distCoeffs.at<double>(0,3) = 0.;
  // distCoeffs.at<double>(0,4) = -19.2852819604873;
  //
  // initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
  //   getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, edges0.size(), 1, edges0.size(), 0),
  //   edges0.size(), CV_16SC2, map1, map2);

  cap >> frame; // get a new frame from camera
  cvtColor(frame, edges0, CV_BGR2GRAY);
  cap >> frame;
  cvtColor(frame, edges1, CV_BGR2GRAY);
  cap >> frame;
  cvtColor(frame, edges2, CV_BGR2GRAY);

  //after capturing 3 samples, we run
  Canny(edges0, edges0, 110, 200, 3);
  Canny(edges1, edges1, 110, 200, 3);
  Canny(edges2, edges2, 110, 200, 3);

  int rows = edges0.size().width;
  int cols = edges0.size().height;

  output = Mat(Size(rows, cols), CV_8U);

  for(size_t i = 0; i < cols; i++) {
    for (size_t j = 0; j < rows; j++) {
      output.at<uchar>(i,j) = edges0.at<uchar>(i,j) + edges1.at<uchar>(i,j) + edges2.at<uchar>(i,j);
    }
  }

  Mat element = getStructuringElement(MORPH_DILATE,
    Size(11, 11),
    Point(5, 5));
    dilate(output, output, element);

    /*
    * Begin filling in silhouettes
    */
    tmpA = output.clone();
    tmpB = output.clone();

    for(size_t i = 1; i < cols; i++) {
      for(size_t j = 1; j < rows; j++){
        if(tmpA.at<uchar>(i-1,j-1) && tmpA.at<uchar>(i-1, j) && tmpA.at<uchar>(i,j-1)) {
          tmpA.at<uchar>(i,j) = 255;
        }
      }
    }


    for(size_t i = cols-2; i > 0; i--) {
      for(size_t j = rows-2; j > 0; j--){
        if(tmpB.at<uchar>(i+1,j+1) && tmpB.at<uchar>(i+1, j) && tmpB.at<uchar>(i,j+1)) {
          tmpB.at<uchar>(i,j) = 255;
        }
      }
    }

    for(size_t i = 1; i < cols-1; i++) {
      for(size_t j=1;j<rows-1;j++)
      {
        output.at<uchar>(i,j) = tmpA.at<uchar>(i,j) && tmpB.at<uchar>(i,j) ? 255 : 0;
      }
    }
    /***************Silhouettes filled in*************/

    cols = output.size().width;
    rows = output.size().height;

    // link interim output 2dvector to create proper heighted silhouettes
    newProduct->rescaledXY.resize(rows, std::vector<int>(cols, 0));

    // create magnification map array for width magnification
    newProduct->magMap.resize(cols, 0.0);

    int lowVal = 0;
    int highVal = 0;

    // populate magMap by iteratively running FindRealHeightRatio on output
    // printf("rows%d,cols%d\n", rows, cols);
    for(int i = 0; i <= cols - 1; i++) {
      lowVal = 0;
      highVal = 0;
      // printf("%d\n", i);
      for(int j = 0; j <=  rows - 1; j++) {
        if(output.at<uchar>(j,i) == 255) {
          // interimMagnify.at(j).at(i) = 1;
          highVal = j;
          // printf("lowVal: %d\n",lowVal);
          while(output.at<uchar>(j,i) == 255 && j <= rows - 1){
            // interimMagnify.at(j).at(i) = 1;
            j++;
          }
          if(j <= rows - 1) {
            lowVal = j;
          }
        }
      }

      if(highVal > 0 && lowVal > 0) {
        // printf("\nPIXEL BOUNDS @ %d: %d\n", i, lowVal-highVal);
        newProduct->magMap.at(i) = FindRealHeightRatio(rows - lowVal);
        // printf("\nRESCALED HEIGHT: %f\n", magMap.at(i)*(highVal-lowVal));
        for(int q = 0; q < (int) (newProduct->magMap.at(i)*(lowVal-highVal)); q++) {
          newProduct->rescaledXY.at(rows - q - 1).at(i) = 1;
        }
      }
    }

    // width rescale
    int left = 0;
    int right = 0;
    int newRight = 0;
    int newLeft = 0;
    int midpoint = 0;

    for(int i = 0; i <= rows - 1; i++) {
      for(int j = 0; j <= cols - 1; j++) {
        if(newProduct->rescaledXY.at(i).at(j) == 1) {
          left = j++;
          while(j <= (cols - 1) && newProduct->rescaledXY.at(i).at(j) == 1) {
            j++;
          }
          right = (j - 1);
          midpoint = round(0.5*(right-left));
          newRight = right + round(0.5*(right - left)*newProduct->magMap.at(midpoint));
          newLeft = left - round(0.5*(right - left)*newProduct->magMap.at(midpoint));
          if (newLeft < 0) newLeft = 0;
          if(newLeft < left) {
            // printf("newLeft %d\n", newLeft);
            while(newLeft < left) {
              newProduct->rescaledXY.at(i).at(newLeft) = 1;
              newLeft++;
            }
          }
          if(newRight < right) {
            // printf("newRight %d--", newRight);
            while(right <= newRight && right <= (cols - 1)) {
              newProduct->rescaledXY.at(i).at(right) = 1;
              right++;
            }
          }
        }
        left = 0;
        right = 0;
        newRight = 0;
        newLeft = 0;
        midpoint = 0;
      }
    }

    // Reset magMap for other camera, using the height of a row as the pixel distance offset
    for(int i = 0; i <= cols - 1; i++) {
      int j = 0;
      while(newProduct->rescaledXY.at(j).at(i) == 1 && j <= rows - 1) {
        j++;
      }
      if(--j < rows - 1) {
        newProduct->magMap.at(i) = FindRealHeightRatio(j);
      } else {
        newProduct->magMap.at(i) = 0.0;
      }
    }
  }
