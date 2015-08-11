#include <stdio.h>
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

#pragma comment(lib, "cvd.lib")
#pragma comment(lib, "cvauxd.lib")
#pragma comment(lib, "cxcored.lib")
#pragma comment(lib, "highguid.lib")

#define ESC '\033'
#define MAX_SEEDS 1024

IplImage* img = 0;
IplImage* seg_img = 0;
char szImgWnd[] = "Image";
char szSegWnd[]  = "Segmented Image";

char szImgFilename[MAX_PATH];

HWND hMainWnd;
int imgWidth, imgHeight;
int obj_seeds[2*MAX_SEEDS], bkg_seeds[2*MAX_SEEDS];
int obj_seed_cnt, bkg_seed_cnt;

void cvMouseHandler(int mouseevent, int x, int y, int flags, void* vparam)
{
    //printf("%s\n", param);
    static bool LBdown, RBdown;
    HWND hWnd = *((HWND*)vparam);
    switch(mouseevent)
    {
        case CV_EVENT_MOUSEMOVE:
            if(LBdown)
            {
                obj_seeds[obj_seed_cnt*2] = x;
                obj_seeds[obj_seed_cnt*2 + 1] = y;
                obj_seed_cnt++;
            }
            else if(RBdown)
            {
                bkg_seeds[bkg_seed_cnt*2] = x;
                bkg_seeds[bkg_seed_cnt*2 + 1] = y;
                bkg_seed_cnt++;
            }
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
    for(int i = 0; i < obj_seed_cnt; i++)
    {
        cvCircle(img, cvPoint(obj_seeds[i*2], obj_seeds[i*2+1]), 1, CV_RGB(255, 0, 0));
    }

    for(int i = 0; i < bkg_seed_cnt; i++)
    {
        cvCircle(img, cvPoint(bkg_seeds[i*2], bkg_seeds[i*2+1]), 1, CV_RGB(0, 0, 255));
    }
    cvShowImage(szImgWnd, img);
}

void Init()
{
    obj_seed_cnt = bkg_seed_cnt = 0;
    //points1 = cvCreateMat(1,point_count,CV_32FC2);
    //points2 = cvCreateMat(1,point_count,CV_32FC2);
    //status = cvCreateMat(1,point_count,CV_8UC1);
}
//void DumpPoints(const char *filename)
//{
//    int i;
//    FILE *fp = fopen(filename, "w+");
//    if(fp == NULL) return;
//
//    fprintf(fp, "%d %d\n", icLP, icRP);
//    for(i = 0; i < icLP; i++)
//        fprintf(fp, "%d %d\n", lPoints[i*2], lPoints[i*2 + 1]);
//    for(i = 0; i < icRP; i++)
//        fprintf(fp, "%d %d\n", rPoints[i*2], rPoints[i*2 + 1]);
//    fclose(fp);
//}
//
void SegmentImage(IplImage* seg_img, IplImage* img,int* obj_seeds,int obj_seed_cnt,int* bkg_seeds,int bkg_seed_cnt)
{

}


char* OpenImageFile()
{
    return NULL;
}
void Usage()
{
    printf("o/O: Opening Image File\ns/S: Perform Image Segmentation\nESC : Exit\n");
}
//
int main(int argc, char ** argv)
{
    int input;
    char szFilename[MAX_PATH];
    cvNamedWindow(szImgWnd, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(szSegWnd, CV_WINDOW_AUTOSIZE);
  
    hMainWnd = (HWND)1;
    //hSegWnd = (HWND)2;
    
    cvSetMouseCallback(szImgWnd, cvMouseHandler, &hMainWnd);
    
    strcpy(szImgFilename, "img1.jpg");

    img = cvLoadImage(szImgFilename, -1);
    
    
    cvShowImage(szImgWnd, img);
    

    bool bTerminate = false;
    Init();
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
                SegmentImage(seg_img, img, obj_seeds, obj_seed_cnt, bkg_seeds, bkg_seed_cnt);
                if(seg_img)
                   cvShowImage(szSegWnd, seg_img);
                break;
            case 'o':
            case 'O':
                printf("Open filename: ");
                scanf("%s", szImgFilename);
                img = cvLoadImage(szImgFilename, -1);
                cvShowImage(szImgWnd, img);
                break;
        }

        MarkPoints();
    }
    cvDestroyAllWindows();
    return 0;
}