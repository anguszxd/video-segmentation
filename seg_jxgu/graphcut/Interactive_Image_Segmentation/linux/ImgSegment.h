// Fahim Mannan
// School of Computer Science, McGill University.
//
#ifndef IMG_SEGMENTATION_H
#define IMG_SEGMENTATION_H

#include <opencv/cv.h>
#include <map>
#include "./maxflow/graph.h"
#include "Pixel.h"

const int MAX_INTENSITY = 256;
const double MAX_LAMBDA = 100;
const double MAX_SIGMA = 10;
const double EPS = 1e-16;
enum tRegion { BKG = 1, OBJ };
struct cmp_pixel 
{
    bool operator()(const Pixel &pa, const Pixel &pb) const
    {
        if(pa.c == pb.c)
            return pa.r < pb.r;
        return pa.c < pb.c;
    }
};
struct cmp_cvscalar
{
    bool operator()(const CvScalar &a, const CvScalar &b) const
    {
        if(a.val[0] < b.val[0]) return true;
        else if(a.val[0] > b.val[0]) return false;
        else if(a.val[1] < b.val[1]) return true;
        else if(a.val[1] > b.val[1]) return false;
        return a.val[2] < b.val[2];
    }
};
class ImgSegment 
{
    //std::vector<Pixel> vObjPixels;
    //std::vector<Pixel> vBkgPixels;
    //char *pixels;       // mark the pixels as unclassified/background/object
    typedef Graph<double, double, double> tGraph;
    typedef std::map<Pixel, tRegion, cmp_pixel> tSeed;
    typedef std::map<CvScalar, int, cmp_cvscalar> tIp;
    typedef std::map<CvScalar, double, cmp_cvscalar> tlogIp;
    tSeed seed;
    tSeed::iterator itSeed;
    int  imarkSeed;
    //double Ip_O[MAX_INTENSITY][MAX_INTENSITY][MAX_INTENSITY], Ip_B[MAX_INTENSITY][MAX_INTENSITY][MAX_INTENSITY];
    //double logIp_O[MAX_INTENSITY][MAX_INTENSITY][MAX_INTENSITY], logIp_B[MAX_INTENSITY][MAX_INTENSITY][MAX_INTENSITY];
    tIp Ip_O, Ip_B;
    tlogIp logIp_O, logIp_B;

    double lambda, sigma;
    tGraph   *pGraph;
    int   NType;
    int   Dim;
    int   width;
    int   height;
    double K;
    int   penwidth;

    IplImage *img;      // input image
    //IplImage *grayImg;  // gray-scale working image
    IplImage *inputimg;
    IplImage *obj;      // object
    IplImage *bkg;      // background

    bool     bUpdated;  // graph update
    void     init();
    void     reset();
    void     buildGraph();
    void     computeBoundaryPenalty();
    int      getNeighbor(int p, int n) const; //nth neighbor of p
    double   distPixel(int p, int q) const;
    double   distSqrPixelIntensity(int p, int q) const;
    double   SQR(double x) const;
    void     getPixelValue(IplImage* img, Pixel &p, CvScalar *val) const;
    void     setPixelValue(IplImage* img, Pixel &p, CvScalar &val);
    
public:
    ImgSegment();
    ~ImgSegment();

    double     getLambda() const;
    double     getSigma() const;
    IplImage* getObj() const;
    IplImage* getBkg() const;
    //IplImage* getGrayImage() const;

    void setLambda(double lambda);
    void setSigma(double sigma);
    void setDim(int Dim);
    void setNType(int NType);
    void incPenWidth();
    void decPenWidth();
    void loadImage(const char *filename);
    void addSeed(Pixel p, tRegion region);
    void resetSeed(tRegion region);
    void markSeeds(IplImage *img);
    void segment(IplImage **obj, IplImage **bkg);   
    
};

#endif
