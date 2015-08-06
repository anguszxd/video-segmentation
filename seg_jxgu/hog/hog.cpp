#include <iostream>
#include <opencv2/opencv.hpp>
 
using namespace std;
using namespace cv;
 
int main (int argc, const char * argv[])
{
    bool bSuccess;
    VideoCapture cap(argv[1]); // open the video camera no. 0
    if (!cap.isOpened())  // if not success, exit program
    {
        cout << "Cannot open the video cam" << endl;
        return -1;
    }
 
    Mat img;
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
 
    namedWindow("video capture", CV_WINDOW_AUTOSIZE);
    while (true)
    {
        if (!(bSuccess = cap.read(img))) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
        if (!img.data)
            continue;
        resize(img,img,Size(img.cols/3,img.rows/3));
        cvtColor(img,img,CV_BGR2GRAY);
        vector<Rect> found, found_filtered;
        hog.detectMultiScale(img, found, 0, Size(8,8), Size(32,32), 1.05, 2);
 
        size_t i, j;
        for (i=0; i<found.size(); i++)
        {
            Rect r = found[i];
            for (j=0; j<found.size(); j++)
                if (j!=i && (r & found[j])==r)
                    break;
            if (j==found.size())
                found_filtered.push_back(r);
        }
        for (i=0; i<found_filtered.size(); i++)
        {
        Rect r = found_filtered[i];
            r.x += cvRound(r.width*0.1);
        r.width = 1.5*cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.06);
        r.height = 1.5*cvRound(r.height*0.9);
        rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 2);
    }
        imshow("video capture", img);
        if (waitKey(20) >= 0)
            break;
    }
    return 0;
}