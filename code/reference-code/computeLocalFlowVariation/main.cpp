#include <stdio.h>
#include <ldof.h>
#include <COpticFlowPart.h>
#include <CMatrix.h>
#include <CTensor.h>
#include <CDiffusion.h>

void computeLocalFlowVariation(CTensor<float>& aFlow, CMatrix<float>& aFlowVar) 
{
  int aXSize = aFlow.xSize();
  int aYSize = aFlow.ySize();
  int aXSize0 = aXSize;
  int aYSize0 = aYSize;
  int aSize = aFlow.size();
  CTensor<float> aStats(aXSize,aYSize,4);
  aStats.paste(aFlow,0,0,0);
  for (int i = 0; i < aSize; i++)
    aStats.data()[i+aSize] = aFlow.data()[i]*aFlow.data()[i];
  aXSize = 1+(aXSize >> 1);
  aYSize = 1+(aYSize >> 1);
  aStats.downsample(aXSize,aYSize);
  CDiffusion aDiffusion;
  CTensor<float> aFlowCoarse(aFlow);
  aFlowCoarse.downsample(aXSize,aYSize);
  CMatrix<float> GX(aXSize,aYSize,1);
  CMatrix<float> GY(aXSize,aYSize,1);
  aDiffusion.isotropicDiffMatrixMultiChannel(aFlowCoarse,GX,GY);
  aDiffusion.mTau2D = 0.5f;
  int aIterations = (int)ceil(10.0f/aDiffusion.mTau2D);
  CMatrix<float> aTemp(aXSize,aYSize);
  for (int k = 0; k < aIterations; k++)
    for (int z = 0; z < aStats.zSize(); z++) {
      aStats.getMatrix(aTemp,z);
      aDiffusion.isotropicDiffuseSemiImplicit(aTemp,GX,GY);
      aStats.putMatrix(aTemp,z);
    }
  aSize = aXSize*aYSize;
  int a2Size = 2*aSize;
  int a3Size = 3*aSize;
  aFlowVar.setSize(aXSize,aYSize);
  for (int i = 0; i < aSize; i++) {
    float help = aStats.data()[i+a2Size]-aStats.data()[i]*aStats.data()[i]+aStats.data()[i+a3Size]-aStats.data()[i+aSize]*aStats.data()[i+aSize];
    if (help < 0.0f) help = 0.0f;
    aFlowVar.data()[i] = sqrt(help);
  }
  aFlowVar.upsample(aXSize0,aYSize0);
}


int main(int argc, char** args) {
  
  // input arguments
  std::string flowFile = args[1];
 
  // initialization
  CTensor<float> aFlow;
  COpticFlow::readMiddlebury(flowFile.c_str(), aFlow);
  int aXSize = aFlow.xSize();
  int aYSize = aFlow.ySize();
  CTensor<float> aFlowVis(aXSize, aYSize, 3);
  COpticFlow::flowToImage(aFlow, aFlowVis, 2.0f);
  aFlowVis.writeToPPM("flowField.ppm");
  CMatrix<float> aFlowVar;

  // compute flow variation
  computeLocalFlowVariation(aFlow, aFlowVar);

  // visualize result
  std::cout << "local flow variation:" << std::endl;
  std::cout << "min: " << aFlowVar.min() << " max: " << aFlowVar.max() << std::endl;
  aFlowVar.normalize(0,255);
  aFlowVar.writeToPGM("localFlowVariation.pgm");

  return 0;
}

