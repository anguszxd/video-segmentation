#ifndef CDiffusionH
#define CDiffusionH

#include <math.h>
#include <CVector.h>
#include <CMatrix.h>
#include <CTensor.h>
#include <CTensor4D.h>

class CDiffusion {
public:
  // constructor
  CDiffusion();
  // copy operator
  inline void operator=(CDiffusion aDiffusion);

  // Properties ----------------------------------------------------------------
  // Time step size afloat one iteration
  // For explicifloat schemes this value musfloat be <= 0.25 for 2-D 
  float mTau2D;
  // Needed for some diffusivity functions to obtain stability
  float mEpsilon;
  // Pre-smoothing for derivation computation, width of binomial filter stencil = 2*mSmoothing+1
  int mSmoothing;

  void isotropicDiffMatrixMultiChannel(const CTensor<float>& aTensor, CMatrix<float>& aResultX, CMatrix<float>& aResultY);
  // AOS schemes ---------------------------------------------------------------
  void isotropicDiffuseSemiImplicit(CMatrix<float>& aMatrix, const CMatrix<float>& GX, const CMatrix<float>& GY);

private:
  void derivativeX(CMatrix<float>& aMatrix, CMatrix<float>& aResult);
  void derivativeY(CMatrix<float>& aMatrix, CMatrix<float>& aResult);
  
  float diffusivity(float aValue);
};

// I M P L E M E N T A T I O N -------------------------------------------------

inline void CDiffusion::operator=(CDiffusion aDiffusion) {
  mTau2D = aDiffusion.mTau2D;
  mEpsilon = aDiffusion.mEpsilon;
  mSmoothing = aDiffusion.mSmoothing;
}

// P R I V A T E ---------------------------------------------------------------

// diffusivity
inline float CDiffusion::diffusivity(float aValue) 
{
    return 1.0/sqrt(aValue+mEpsilon*mEpsilon);
}

#endif

