#include <NMath.h>
#include "CDiffusion.h"

// constructor
CDiffusion::CDiffusion() {
  mTau2D  = 1.0;
  mEpsilon = 0.001;
  mSmoothing = 0;
}

void CDiffusion::derivativeX(CMatrix<float>& aMatrix, CMatrix<float>& aResult)
{
  int aXSize = aMatrix.xSize();
  int aYSize = aMatrix.ySize();
  for (int y=0; y<aYSize; ++y)
  for (int x=1; x<aXSize-1; ++x) {
    aResult(x,y) = 0.5f*(aMatrix(x+1,y)-aMatrix(x-1,y));
  }
  for (int y=0; y<aYSize; ++y) {
    aResult(0,y) = 0.5f*(aMatrix(1,y)-aMatrix(0,y));
    aResult(aXSize-1,y) = 0.5f*(aMatrix(aXSize-1,y)-aMatrix(aXSize-2,y));
  }
}
void CDiffusion::derivativeY(CMatrix<float>& aMatrix, CMatrix<float>& aResult)
{
  int aXSize = aMatrix.xSize();
  int aYSize = aMatrix.ySize();
  for (int x=0; x<aXSize; ++x)
  for (int y=1; y<aYSize-1; ++y) {
    aResult(x,y) = 0.5f*(aMatrix(x,y+1)-aMatrix(x,y-1));
  }
  for (int x=0; x<aXSize; ++x) {
    aResult(x,0) = 0.5f*(aMatrix(x,1)-aMatrix(x,0));
    aResult(x,aYSize-1) = 0.5f*(aMatrix(x,aYSize-1)-aMatrix(x,aYSize-2));
  }
}

// isotropicDiffMatrixMultiChannel (one-sided differences)
void CDiffusion::isotropicDiffMatrixMultiChannel(const CTensor<float>& aTensor, CMatrix<float>& aResultX, CMatrix<float>& aResultY) {
  CMatrix<float> aGrad(aTensor.xSize(),aTensor.ySize());
  CMatrix<float> aTemp(aTensor.xSize(),aTensor.ySize());
  aResultX = 0;
  aResultY = 0;
  // Compute sum of all channel gradients
  for (int k = 0; k < aTensor.zSize(); k++) {
    aTensor.getMatrix(aTemp,k);
    // Gradients in x direction
    derivativeY(aTemp, aGrad);
    int i = 0;
    for (int y = 0; y < aTemp.ySize(); y++) {
      for (int x = 0; x < aTemp.xSize()-1; x++) {
        float aGradX = aTemp.data()[i+1]-aTemp.data()[i];
        float aGradY = 0.5*(aGrad.data()[i]+aGrad.data()[i+1]);
        aResultX.data()[i] += aGradX*aGradX+aGradY*aGradY;
        i++;
      }
      i++;
    }
    // Gradients in y direction
    derivativeX(aTemp, aGrad);
    i = 0;
    for (int y = 0; y < aTemp.ySize()-1; y++)
      for (int x = 0; x < aTemp.xSize(); x++) {
        float aGradX = 0.5*(aGrad.data()[i]+aGrad.data()[i+aTemp.xSize()]);
        float aGradY = aTemp.data()[i+aTemp.xSize()]-aTemp.data()[i];
        aResultY.data()[i] += aGradX*aGradX+aGradY*aGradY;
        i++;
      }
  }
  // Compute diffusivity
  int aSize = aTensor.xSize()*aTensor.ySize();
  for (int i = 0; i < aSize; i++) {
    aResultX.data()[i] = diffusivity(aResultX.data()[i]);
    aResultY.data()[i] = diffusivity(aResultY.data()[i]);
  }
}

// Semi-implicifloat AOS -----------------------------------------------------------

// isotropicDiffuseSemiImplicifloat (2-D)
void CDiffusion::isotropicDiffuseSemiImplicit(CMatrix<float>& aMatrix, const CMatrix<float>& GX, const CMatrix<float>& GY) {
  CMatrix<float> aResult(aMatrix.xSize(),aMatrix.ySize());
  CMatrix<float> M(aMatrix.xSize(),aMatrix.ySize());
  CMatrix<float> Y(aMatrix.xSize(),aMatrix.ySize());
  CMatrix<float> I(aMatrix.xSize(),aMatrix.ySize());
  CMatrix<float> J(aMatrix.xSize(),aMatrix.ySize());
  CMatrix<float> K(aMatrix.xSize(),aMatrix.ySize());
  int aSize = aMatrix.xSize()*aMatrix.ySize();
  float aTau = mTau2D;
  if (mSmoothing == 0) aTau *= 2.0;
  // Compute diffusion in x direction
  // alpha -> I   beta -> J    gamma -> K
  int i = 0;
  if (mSmoothing == 0) {
    for (int y = 0; y < aMatrix.ySize(); y++) {
      float aTemp1 = aTau*GX.data()[i];
      float aTemp2;
      I.data()[i] = 1+aTemp1;
      J.data()[i] = -aTemp1;
      if (i > 0) K.data()[i-1] = 0;
      i++;
      for (int x = 1; x+1 < aMatrix.xSize(); x++) {
        aTemp1 = aTau*GX.data()[i];
        aTemp2 = aTau*GX.data()[i-1];
        I.data()[i] = 1+aTemp1+aTemp2;
        J.data()[i] = -aTemp1;
        K.data()[i-1] = -aTemp2;
        i++;
      }
      aTemp2 = aTau*GX.data()[i-1];
      I.data()[i] = 1+aTemp2;
      J.data()[i] = 0;
      K.data()[i-1] = -aTemp2;
      i++;
    }
  }
  else {
    for (int y = 0; y < aMatrix.ySize(); y++) {
      float aTemp1 = aTau*(GX.data()[i]+GX.data()[i+1]);
      float aTemp2;
      I.data()[i] = 1+aTemp1;
      J.data()[i] = -aTemp1;
      if (i > 0) K.data()[i-1] = 0;
      i++;
      for (int x = 1; x+1 < aMatrix.xSize(); x++) {
        aTemp1 = aTau*(GX.data()[i]+GX.data()[i+1]);
        aTemp2 = aTau*(GX.data()[i]+GX.data()[i-1]);
        I.data()[i] = 1+aTemp1+aTemp2;
        J.data()[i] = -aTemp1;
        K.data()[i-1] = -aTemp2;
        i++;
      }
      aTemp2 = aTau*(GX.data()[i]+GX.data()[i-1]);
      I.data()[i] = 1+aTemp2;
      J.data()[i] = 0;
      K.data()[i-1] = -aTemp2;
      i++;
    }
  }
  M.data()[0] = 1.0/I.data()[0];
  Y.data()[0] = aMatrix.data()[0];
  i = 0;
  do {
    float l = K.data()[i]*M.data()[i];
    i++;
    M.data()[i] = 1.0/(I.data()[i]-l*J.data()[i-1]);
    Y.data()[i] = aMatrix.data()[i]-l*Y.data()[i-1];
  }
  while (i < aSize-1);
  float v = Y.data()[aSize-1]*M.data()[aSize-1];
  aResult.data()[aSize-1] = v;
  for (int j = aSize-2; j >= 0; j--) {
    v = (Y.data()[j]-J.data()[j]*v)*M.data()[j];
    aResult.data()[j] = v;
  }
  // Compute diffusion in y direction
  // alpha -> I   beta -> J    gamma -> K
  i = 0;
  if (mSmoothing == 0) {
    for (int x = 0; x < aMatrix.xSize(); x++) {
      float aTemp1 = aTau*GY(x,0);
      float aTemp2;
      I.data()[i] = 1+aTemp1;
      J.data()[i] = -aTemp1;
      if (i > 0) K.data()[i-1] = 0;
      i++;
      for (int y = 1; y+1 < aMatrix.ySize(); y++) {
        aTemp1 = aTau*GY(x,y);
        aTemp2 = aTau*GY(x,y-1);
        I.data()[i] = 1+aTemp1+aTemp2;
        J.data()[i] = -aTemp1;
        K.data()[i-1] = -aTemp2;
        i++;
      }
      aTemp2 = aTau*GY(x,aMatrix.ySize()-2);
      I.data()[i] = 1+aTemp2;
      J.data()[i] = 0;
      K.data()[i-1] = -aTemp2;
      i++;
    }
  }
  else {
    for (int x = 0; x < aMatrix.xSize(); x++) {
      float aTemp1 = aTau*(GY(x,0)+GY(x,1));
      float aTemp2;
      I.data()[i] = 1+aTemp1;
      J.data()[i] = -aTemp1;
      if (i > 0) K.data()[i-1] = 0;
      i++;
      for (int y = 1; y+1 < aMatrix.ySize(); y++) {
        aTemp1 = aTau*(GY(x,y)+GY(x,y+1));
        aTemp2 = aTau*(GY(x,y)+GY(x,y-1));
        I.data()[i] = 1+aTemp1+aTemp2;
        J.data()[i] = -aTemp1;
        K.data()[i-1] = -aTemp2;
       i++;
      }
      aTemp2 = aTau*(GY(x,aMatrix.ySize()-1)+GY(x,aMatrix.ySize()-2));
      I.data()[i] = 1+aTemp2;
      J.data()[i] = 0;
      K.data()[i-1] = -aTemp2;
      i++;
    }
  }
  M.data()[0] = 1.0/I.data()[0];
  Y.data()[0] = aMatrix.data()[0];
  i = 0;
  int x = 0;
  int y = 0;
  do {
    float l = K.data()[i]*M.data()[i];
    i++;
    y++;
    if (y >= aMatrix.ySize()) {
      y = 0;
      x++;
    }
    M.data()[i] = 1.0/(I.data()[i]-l*J.data()[i-1]);
    Y.data()[i] = aMatrix(x,y)-l*Y.data()[i-1];
  }
  while (i < aSize-1);
  v = Y.data()[aSize-1]*M.data()[aSize-1];
  aResult.data()[aSize-1] += v;
  x = aMatrix.xSize()-1; y = aMatrix.ySize()-1;
  for (int j = aSize-2; j >= 0; j--) {
    y--;
    if (y < 0) {
      y = aMatrix.ySize()-1;
      x--;
    }
    v = (Y.data()[j]-J.data()[j]*v)*M.data()[j];
    aResult(x,y) += v;
  }
  // Average between x- and y-direction
  for (int j = 0; j < aSize; j++)
    aMatrix.data()[j] = 0.5*aResult.data()[j];
}

