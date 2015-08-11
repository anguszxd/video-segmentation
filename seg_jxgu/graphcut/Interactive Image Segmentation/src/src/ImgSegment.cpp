// Fahim Mannan
// School of Computer Science, McGill University.
//
#include <cv.h>
#include <highgui.h>
#include <assert.h>
#include "ImgSegment.h"
#define CV_RELEASE_IMG(x) { if(*x) cvReleaseImage(x); *x = NULL; }

int N8[8][2] = {-1, -1,
                -1,  0,
                -1,  1,
                 0, -1,
                 0,  1,
                 1, -1,
                 1,  0,
                 1,  1,};
int N24[24][3];
ImgSegment::ImgSegment() : 
    img(0), /*grayImg(0),*/ obj(0), bkg(0), width(0), height(0), 
    pGraph(0), bUpdated(false), sigma(2), lambda(7),
    NType(8), Dim(2), penwidth(4) {}

ImgSegment::~ImgSegment()
{
    reset();
}
void ImgSegment::incPenWidth()
{
    penwidth++;
}

void ImgSegment::decPenWidth()
{
    if(penwidth > 1)
        penwidth--;
}
void ImgSegment::reset()
{
    CV_RELEASE_IMG(&img);
    //CV_RELEASE_IMG(&grayImg);
    CV_RELEASE_IMG(&obj);
    CV_RELEASE_IMG(&bkg);
    if(pGraph) delete pGraph;
    bUpdated = false;
    seed.clear();
    itSeed = seed.begin();
    imarkSeed = 0;
}
inline
double ImgSegment::SQR(double x) const
{
    return x * x;
}
double ImgSegment::distSqrPixelIntensity(int p, int q) const
{
    double a, b;
    int r1, c1, r2, c2;
    r1 = p/width;
    c1 = p%width;
    r2 = q/width;
    c2 = q%width;

    //may not be fast but generic api is required
    CvScalar val1 = cvGet2D(img, r1, c1);
    CvScalar val2 = cvGet2D(img, r2, c2);
    
    return SQR(val1.val[0] - val2.val[0])+SQR(val1.val[1] - val2.val[1])+SQR(val1.val[2] - val2.val[2]) ;
}

void ImgSegment::init()
{
    width = inputimg->width;
    height = inputimg->height;
    pGraph = new tGraph(width * height, 4 * width * height);
    pGraph->add_node(width * height);
    
    img = cvCreateImage(cvGetSize(inputimg), IPL_DEPTH_8U, inputimg->nChannels);
    //cvCvtColor(inputimg, img, CV_BGR2Lab);
    cvCopy(inputimg, img);
    //if(img->nChannels > 1) {
    //cvCvtColor(img, grayImg, CV_BGR2GRAY);
    /*}else {
        cvCopy(img, grayImg);
    }*/
    obj = cvCreateImage(cvGetSize(img), inputimg->depth, inputimg->nChannels);
    bkg = cvCreateImage(cvGetSize(img), inputimg->depth, inputimg->nChannels);
    //cvCopy(img, obj);
    //cvCopy(img, bkg);

    computeBoundaryPenalty();    
}
void ImgSegment::computeBoundaryPenalty()
{
    //Compute B_pq and K
    double B_pq, sum_pq;
    double two_sigma_sqr = 2*sigma*sigma;
    int i, j, k, n = width * height;
    K = 0; // K = 1 + max_p Sum(B_pq)
    for(i = 0; i < n; i++)
    {
        sum_pq = 0;
        for(k = 0; k < NType; k++)
        {
            j = getNeighbor(i, k); //get the nth neighbor of i
            if(j < 0) continue;
            //double d = distSqrPixelIntensity(i, j);
            //assert(d >= 0);
            assert(i != j);
            B_pq = exp(-distSqrPixelIntensity(i, j)/two_sigma_sqr) * (1.0/distPixel(i,j));
            assert(B_pq >= 0 && B_pq <= 1.0);
            pGraph->add_edge(i, j, B_pq, B_pq);
            sum_pq += B_pq;
        }
        if(sum_pq > K)
            K = sum_pq;
    }
    K++;
}
void ImgSegment::loadImage(const char* filename)
{
    reset();
    inputimg = cvLoadImage(filename, -1);
    init();
}

void ImgSegment::markSeeds(IplImage *img)
{
    if(img == 0) return;
    int i = 0;
    CvScalar color;
    tSeed::iterator it;
    //for(it = seed.begin(); it != seed.end() && i < imarkSeed; it++, i++)
    //    ;
    //if(!seed.empty())
    
    //if(imarkSeed < seed.size())
    //for(; it != seed.end(); it++, imarkSeed++)
    for(it = seed.begin(); it != seed.end(); it++)
    {
        //tSeed::iterator it = seed.end();
        //it--;
        //seed.at
        if(it->second == OBJ)
            color = CV_RGB(255, 0, 0);
        else
            color = CV_RGB(0, 0, 255);
        cvCircle(img, cvPoint(it->first.c, it->first.r), 1, color, -1);
    }
}
IplImage* ImgSegment::getObj() const
{
    return obj;
}

IplImage* ImgSegment::getBkg() const
{
    return bkg;
}

//IplImage* ImgSegment::getGrayImage() const
//{
//    return grayImg;
//}

void ImgSegment::resetSeed(tRegion region)
{
    seed.clear();
    bUpdated = false;
}
void ImgSegment::addSeed(Pixel p, tRegion region)
{
    Pixel tmp;
    int dr, dc;
    for(dr = -penwidth + 1; dr < penwidth; dr++)
        for(dc = -penwidth + 1; dc < penwidth; dc++)
        {
            tmp.r = p.r + dr;
            tmp.c = p.c + dc;
            if(tmp.r >= 0 && tmp.r < height && tmp.c >= 0 && tmp.c < width)
                seed[tmp] = region;
        }
    bUpdated = false;
}

void ImgSegment::setLambda(double lambda)
{
    this->lambda = lambda;
}

void ImgSegment::setSigma(double sigma)
{
    this->sigma = sigma;
    computeBoundaryPenalty();
}
void ImgSegment::setDim(int Dim)
{
    this->Dim = Dim;
}
double ImgSegment::getSigma() const
{
    return sigma;
}
double ImgSegment::getLambda() const 
{
    return lambda;
}
inline
void  ImgSegment::getPixelValue(IplImage *_img, Pixel &p, CvScalar *val) const
{
    //*val = cvGet2D(_img, p.r, p.c);
    val->val[0] = CV_IMAGE_ELEM(_img, uchar, p.r, p.c);
    if(_img->nChannels == 3) {
        val->val[1] = CV_IMAGE_ELEM(_img, uchar, p.r, p.c * _img->nChannels + 1);
        val->val[2] = CV_IMAGE_ELEM(_img, uchar, p.r, p.c * _img->nChannels + 2);
    }else if(img->nChannels == 1){
        val->val[1] = 0;
        val->val[2] = 0;
    }else abort();

}
inline
void  ImgSegment::setPixelValue(IplImage *_img, Pixel &p, CvScalar &val)
{
    cvSet2D(_img, p.r, p.c, val);
}

inline double ImgSegment::distPixel(int p, int q) const 
{   
    double r1,c1,r2,c2;
    r1 = p/width;
    c1 = p%width;
    r2 = q/width;
    c2 = q%width;
    return sqrt((r1-r2)*(r1-r2) + (c1-c2)*(c1-c2));
}

int ImgSegment::getNeighbor(int p, int n) const
{
    int r, c;
    r = p/width;
    c = p%width;
   
    if(NType == 8)
    { //Hard code it for now
       r += N8[n][0];
       c += N8[n][1];
       if(r >= 0 && r < height && c >= 0 && c < width)
        return r * width + c;
    }
    return -1;
}

void ImgSegment::buildGraph()
{
    //if(bUpdated)
    //    return;
    int i,j,k,n;
    Pixel p;
    CvScalar img_val;
    int obj_cnt = 0, bkg_cnt = 0;
  
    bUpdated = true;    
    
    //memset(Ip_O, 0, MAX_INTENSITY * MAX_INTENSITY * MAX_INTENSITY * sizeof(double));
    //memset(Ip_B, 0, MAX_INTENSITY * MAX_INTENSITY * MAX_INTENSITY *sizeof(double));
    Ip_O.clear();
    Ip_B.clear();
   
    //
    for(tSeed::iterator it = seed.begin(); it != seed.end(); it++)
    {   
        p = it->first;
        //img_val = cvGet2D(img, p.r, p.c);
        getPixelValue(img, p, &img_val);
        //i = img_val.val[0];
        //j = img_val.val[1];
        //k = img_val.val[2];
        //I = /*img_val.val[0];*/(unsigned char) *(grayImg->imageData + p.r * grayImg->widthStep + p.c);
        if(it->second == OBJ) {
            
            if(Ip_O.find(img_val) == Ip_O.end())
                Ip_O[img_val] = 1;
            else
                Ip_O[img_val] = Ip_O[img_val] + 1;
            obj_cnt++;
        }else {
            if(Ip_B.find(img_val) == Ip_B.end())
                Ip_B[img_val] = 1;
            else
                Ip_B[img_val] = Ip_B[img_val]+1;
            bkg_cnt++;
        }
    }
    //Normalize
    
    for(tIp::iterator it = Ip_O.begin(); it != Ip_O.end(); it++)
    {
        logIp_O[it->first] = -log((double)it->second/obj_cnt);
    }
    for(tIp::iterator it = Ip_B.begin(); it != Ip_B.end(); it++)
    {
        logIp_B[it->first] = -log((double)it->second/bkg_cnt);
    }
    n = width * height;
    double RpBkg, RpObj;
    for(i = 0; i < n; i++)
    {
        p.r = i/width;
        p.c = i%width;
        if(seed.find(p) == seed.end()) {
            //img_val = cvGet2D(img, p.r, p.c);
            getPixelValue(img, p, &img_val);
            //assert(img_val.val[0] == (unsigned char) *(grayImg->imageData + p.r * grayImg->widthStep + p.c));
            //I = /*img_val.val[0];*/(unsigned char) *(grayImg->imageData + p.r * grayImg->widthStep + p.c);
            if(logIp_B.find(img_val) == logIp_B.end())
                RpBkg = K;
            else RpBkg = -logIp_B[img_val];
            if(logIp_O.find(img_val) == logIp_O.end())
                RpObj = K; 
            else RpObj = -logIp_O[img_val];
            pGraph->add_tweights(i, lambda * RpBkg, lambda * RpObj);
        }else if(seed[p] == BKG) {
            pGraph->add_tweights(i, 0, K);
        }else if(seed[p] == OBJ){
            pGraph->add_tweights(i, K, 0);
        }else assert(0);
    }
}

void ImgSegment::setNType(int NType)
{
    this->NType = NType;
}

void ImgSegment::segment(IplImage **pObj, IplImage **pBkg)
{
    printf("Building Graph ...\n");
    buildGraph();
    printf("Segmenting...\n");
    double flow = pGraph->maxflow();
    printf("flow = %lf\n", flow);
    
    int i;
    int n = width * height;
    Pixel p;
    CvScalar val;
    val.val[0] = val.val[1] = val.val[2] = 0;
    cvCopy(inputimg, bkg);
    cvCopy(inputimg, obj);
    for(i = 0; i < n; i++)
    {
        p.r = i/width;
        p.c = i%width;

        if(pGraph->what_segment(i) == tGraph::SOURCE) {
            //cvSet2D(bkg, p.r, p.c, val);            
            setPixelValue(bkg, p, val);
            /*((uchar*)(bkg->imageData + p.r * bkg->widthStep))[p.c * bkg->nChannels] =
                ((uchar*)(bkg->imageData + p.r * bkg->widthStep))[p.c * bkg->nChannels+1] =
                ((uchar*)(bkg->imageData + p.r * bkg->widthStep))[p.c * bkg->nChannels+2] = 0;*/
        } else {
            /*assert(pGraph->what_segment(i) == tGraph::SINK);
            ((uchar*)(obj->imageData + p.r * obj->widthStep))[p.c * obj->nChannels] =
                ((uchar*)(obj->imageData + p.r * obj->widthStep))[p.c * obj->nChannels+1] =
                ((uchar*)(obj->imageData + p.r * obj->widthStep))[p.c * obj->nChannels+2] = 0;*/
            //cvSet2D(obj, p.r, p.c, val);
            setPixelValue(obj, p, val);
        }
    }
    *pBkg = bkg;
    *pObj = obj;
}



