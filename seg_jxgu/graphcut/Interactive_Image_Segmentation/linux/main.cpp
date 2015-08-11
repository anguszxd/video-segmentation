#include <stdio.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cvaux.h"
#include "ImgSegment.h"
//#include <comctl32.h>

#if 0
#pragma comment(lib, "cvd.lib")
#pragma comment(lib, "cvauxd.lib")
#pragma comment(lib, "cxcored.lib")
#pragma comment(lib, "highguid.lib")
#pragma comment(lib, "comctl32.lib")  
#endif // 1


#define ESC '\033'
//#define MAX_SEEDS 1024

IplImage* img = 0;
IplImage* obj_img = 0;
IplImage* bkg_img = 0;
ImgSegment iseg;

char szImgWnd[] = "Image";
char szObjWnd[] = "Object";
char szBkgWnd[] = "Background";
char szGSWnd[] = "Gray scale image";
char szTBLambda[] = "lambda";
char szTBSigma[] = "sigma";

HWND hMainWnd;
int imgWidth, imgHeight;
bool bUpdate;
//int obj_seeds[2*MAX_SEEDS], bkg_seeds[2*MAX_SEEDS];
//int obj_seed_cnt, bkg_seed_cnt;

void cvMouseHandler(int mouseevent, int c, int r, int flags, void* vparam)
{
    //printf("%d %d\n", r, c);
    if(!img) return;
    if(r < 0 || r >= img->height || c < 0 || c >= img->width) return;
    
    static bool LBdown, RBdown;
    HWND hWnd = *((HWND*)vparam);
    Pixel p = {r, c};
    switch(mouseevent)
    {
        case CV_EVENT_MOUSEMOVE:
            if(LBdown)
            {
               /* obj_seeds[obj_seed_cnt*2] = x;
                obj_seeds[obj_seed_cnt*2 + 1] = y;
                obj_seed_cnt++;*/
                iseg.addSeed(p, OBJ);
            }
            else if(RBdown)
            {
                /*bkg_seeds[bkg_seed_cnt*2] = x;
                bkg_seeds[bkg_seed_cnt*2 + 1] = y;
                bkg_seed_cnt++;*/
                iseg.addSeed(p, BKG);
            }
            bUpdate = true;
            break;
        case CV_EVENT_LBUTTONUP: //object seeds
            LBdown = false;
            break;
        case CV_EVENT_RBUTTONUP: //background seeds
            RBdown = false;
            break;
        case CV_EVENT_LBUTTONDOWN: //object seeds
            LBdown = true;
            break;
        case CV_EVENT_RBUTTONDOWN: //background seeds
            RBdown = true;
            break;
    }
}

void MarkPoints()
{
    iseg.markSeeds(img);

    cvShowImage(szImgWnd, img);
}

void Usage()
{
    printf("o/O: Opening Image File\ns/S: Perform Image Segmentation\nESC : Exit\n");
}
//
void UpdateLambda(int pos)
{
    iseg.setLambda((float)pos);
}
void UpdateSigma(int pos)
{
    iseg.setSigma((float)pos);
}
char * OpenImageFile(char *szFullFileName)
{
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(ofn));
    TCHAR szFileName[MAX_PATH] = TEXT("");
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = HWND_DESKTOP;
    ofn.lpstrFilter = TEXT("Jpeg Files (*.jpg)\0*.jpg\0BMP Files (*.bmp)\0*.bmp\0PPM Files (*.ppm)\0*.ppm\0All Files (*.*)\0*.*\0");
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("jpg");

    if(GetOpenFileName(&ofn))
    {
       GetFullPathName(szFileName, MAX_PATH, szFullFileName, NULL);
       return szFullFileName;
    }
    return 0;
}
//
int main(int argc, char ** argv)
{
    int input;
    char szFileName[MAX_PATH], szObjFileName[MAX_PATH], szBkgFileName[MAX_PATH];
    cvNamedWindow(szBkgWnd, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(szObjWnd, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(szGSWnd, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(szImgWnd, CV_WINDOW_AUTOSIZE);
    
    int val = iseg.getLambda();
    cvCreateTrackbar(szTBLambda, szImgWnd, &val, MAX_LAMBDA, UpdateLambda);
    val = iseg.getSigma();
    cvCreateTrackbar(szTBSigma, szImgWnd, &val, MAX_SIGMA, UpdateSigma);
    hMainWnd = (HWND)1;
    //hSegWnd = (HWND)2;
    
    cvSetMouseCallback(szImgWnd, cvMouseHandler, &hMainWnd);
    
    //strcpy(szFileName, "img3.jpg");
    
    //iseg.loadImage(szFileName);
    /*img = cvLoadImage(szFileName,-1);   
    
    cvShowImage(szImgWnd, img);
    IplImage *cielab = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
    cvCvtColor(img, cielab, CV_BGR2Lab);
    cvShowImage(szGSWnd, cielab);*/

    bool bTerminate = false;
    bUpdate = false;
    Usage();
    while(!bTerminate) //main loop
    {
        input = cvWaitKey(2);

        switch(input)
        {
            case ESC:
                bTerminate = true;
                break;
            case 'r':
            case 'R':
                iseg.resetSeed(OBJ);
                iseg.resetSeed(BKG);
                break;
            case '+':
                iseg.incPenWidth();
                break;
            case '-':
                iseg.decPenWidth();
                break;
            case 's':
            case 'S':
                iseg.segment(&obj_img, &bkg_img);
                if(obj_img)
                   cvShowImage(szObjWnd, obj_img);
                if(bkg_img)
                   cvShowImage(szBkgWnd, bkg_img);
                break;
            case 'o':
            case 'O':
                OpenImageFile(szFileName);
                printf("Opening %s\n", szFileName);
                //printf("Open filename: ");
                //scanf("%s", szImgFilename);
                img = cvLoadImage(szFileName, -1);
                iseg.loadImage(szFileName);
                cvShowImage(szImgWnd, img);
                //cvShowImage(szGSWnd, iseg.getGrayImage());
                break;
            case 'w':
            case 'W':
                sprintf(szObjFileName, "%s_obj.jpg", szFileName);
                sprintf(szBkgFileName, "%s_bkg.jpg", szFileName);
                cvSaveImage(szObjFileName, iseg.getObj());
                cvSaveImage(szBkgFileName, iseg.getBkg());
                break;
        }
        if(bUpdate)
        {
            MarkPoints();
            bUpdate = false;
        }
    }
    cvDestroyAllWindows();
    return 0;
}
