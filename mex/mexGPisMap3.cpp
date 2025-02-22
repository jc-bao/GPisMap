/*
Q: how to define a mex function?
A: 
 */

#include "mex.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <vector>
#include <memory>
#include <iostream>
#include <chrono>
#include <string.h>
#include "GPisMap3.h"

static GPisMap3 *gpm = 0;

static const double bigbirdCams_fx[5] = {570.9361, 572.3318, 568.9403, 567.9881, 572.7638};
static const double bigbirdCams_fy[5] = {570.9376, 572.3316, 568.9419, 567.9995, 572.7567};
static const double bigbirdCams_cx[5] = {306.8789, 309.9968, 308.4583, 310.5243, 310.4192};
static const double bigbirdCams_cy[5] = {238.8476, 230.6296, 225.8232, 223.9443, 214.8762};
static const int bigbirdCams_width = 640;
static const int bigbirdCams_height = 480;
static const double YCBCams_fx[5] = {570.2590, 571.8461, 568.4464, 566.9790, 574.0641};
static const double YCBCams_fy[5] = {570.2636, 571.8428, 568.4494, 566.9812, 574.0598};
static const double YCBCams_cx[5] = {313.7235, 314.9134, 310.3805, 314.3801, 314.6690};
static const double YCBCams_cy[5] = {236.0783, 229.4538, 224.6232, 223.9443, 220.7985};
static const int YCBCams_width = 640;
static const int YCBCams_height = 480;
static const double cubeCam_fx = 32;
static const double cubeCam_fy = 128;
static const double cubeCam_cx = 128;
static const double cubeCam_cy = 32;
static const int cubeCam_width = 256;
static const int cubeCam_height = 64;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  char command[128];
  mxGetString(prhs[0], command, 128);
  std::string commstr(command);

  if (commstr.compare("update") == 0)
  {

    if (gpm == 0)
    {
      gpm = new GPisMap3();
    }
    mwSize numDim = mxGetNumberOfDimensions(prhs[1]);
    const mwSize *dims = mxGetDimensions(prhs[1]);

    char dim_str[7];
    std::sprintf(dim_str, "%d", numDim);
    char nrhs_str[7];
    std::sprintf(nrhs_str, "%d", nrhs);
    strcat(dim_str,nrhs_str);

    if ((int(numDim) == 2) && (nrhs == 3))
    {
      if (mxSINGLE_CLASS != mxGetClassID(prhs[1]) ||
          mxSINGLE_CLASS != mxGetClassID(prhs[2]))
      {
        std::cout << "The input data must be float (single) type.  @ train()" << std::endl;
        return;
      }

      float *pz = (float *)mxGetData(prhs[1]);
      float *ppose = (float *)mxGetData(prhs[2]);

      std::vector<float> pose(ppose, ppose + mxGetNumberOfElements(prhs[2]));

      int numel = mxGetNumberOfElements(prhs[1]);
      gpm->update(pz, numel, pose);
    }
    else {
      std::cout << "Error: Check the input dimension." << std::endl;
    }
    return;
  }
  else if (commstr.compare("test") == 0)
  {
    std::ofstream out("test.txt");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
    std::cout << "out buf test" << std::endl;

    const mwSize *dims = mxGetDimensions(prhs[1]);
    int dim = dims[0];
    int N = dims[1];
    std::cout << dim << "," << N << std::endl;

    if (dim != 3)
    {
      return;
    }

    if (mxSINGLE_CLASS != mxGetClassID(prhs[1]))
    {
      std::cout << "The input data must be float (single) type. @ test()" << std::endl;
      return;
    }

    if ((gpm != 0) && (N > 0))
    {
      float *px = (float *)mxGetData(prhs[1]);

      if (sizeof(float) == sizeof(double))
      {
        plhs[0] = mxCreateDoubleMatrix(2 * (1 + dim), N, mxREAL); // f-vale[0],  grad-value[1,2], variances[3-5]
      }
      else if (sizeof(float) == sizeof(float))
      {
        plhs[0] = mxCreateNumericMatrix(2 * (1 + dim), N, mxSINGLE_CLASS, mxREAL);
      }

      float *pRes = (float *)mxGetPr(plhs[0]);
      gpm->test(px, dim, N, pRes);
    }
    else if (gpm == 0)
    {
      std::cout << "Error: the map is not initialized." << std::endl;
    }
  }
  else if (commstr.compare("setCamera") == 0)
  {

    double *camID = (double *)mxGetData(prhs[1]);
    int n = (int)(*camID) - 1;

    char cam_name[12];
    mxGetString(prhs[2], cam_name, 12);
    std::string camnamestr(cam_name);
    if (gpm != 0)
    {
      if (n >= 0 && n < 5)
      {
        if (camnamestr.compare("bigbird") == 0)
        {
          camParam c(bigbirdCams_fx[n], bigbirdCams_fy[n], bigbirdCams_cx[n], bigbirdCams_cy[n], bigbirdCams_width, bigbirdCams_height);
          gpm->resetCam(c);
        }
        else if (camnamestr.compare("ycb") == 0)
        {
          camParam c(YCBCams_fx[n], YCBCams_fy[n], YCBCams_cx[n], YCBCams_cy[n], YCBCams_width, YCBCams_height);
          gpm->resetCam(c);
        }
        else if (camnamestr.compare("cube") == 0)
        {
          camParam c(cubeCam_fx, cubeCam_fy, cubeCam_cx, cubeCam_cy, cubeCam_width, cubeCam_height);
          gpm->resetCam(c);
        }
      }
    }
    else
    {
      GPisMap3Param p;
      if (camnamestr.compare("bigbird") == 0)
      {
        camParam c(bigbirdCams_fx[n], bigbirdCams_fy[n], bigbirdCams_cx[n], bigbirdCams_cy[n], bigbirdCams_width, bigbirdCams_height);
        gpm = new GPisMap3(p, c);
      }
      else if (camnamestr.compare("ycb") == 0)
      {
        camParam c(YCBCams_fx[n], YCBCams_fy[n], YCBCams_cx[n], YCBCams_cy[n], YCBCams_width, YCBCams_height);
        gpm = new GPisMap3(p, c);
      }
      else if (camnamestr.compare("cube") == 0)
      {
        camParam c(cubeCam_fx, cubeCam_fy, cubeCam_cx, cubeCam_cy, cubeCam_width, cubeCam_height);
        gpm = new GPisMap3(p, c);
      }
    }
    return;
  }
  else if (commstr.compare("getAllPoints") == 0)
  {
    if (gpm != 0)
    {
      std::vector<float> pos;
      gpm->getAllPoints(pos);

      int N = pos.size() / 3; // 2D
      if (N > 0)
      {
        plhs[0] = mxCreateNumericMatrix(3, N, mxSINGLE_CLASS, mxREAL);
        float *pRes = (float *)mxGetPr(plhs[0]);
        memcpy(pRes, pos.data(), sizeof(float) * 3 * N);
      }
    }
  }
  else if (commstr.compare("reset") == 0)
  {
    if (gpm != 0)
    {
      gpm->reset();
      delete gpm;
      gpm = 0;
      std::cout << "Map cleared\n" << std::endl;
    }
    return;
  }

  return;
}
