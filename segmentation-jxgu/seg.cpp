 #include <opencv2/core/core.hpp>
 #include <opencv2/highgui/highgui.hpp>
 #include <iostream>

using namespace cv;

int main(int argc, char** argv)
{
  string filename = "..//..//..//video//test-0712-001.avi";
  VideoCapture capture(filename);
  Mat frame;

  if( !capture.isOpened() )
    throw "Error when reading steam_avi";

  namedWindow( "w", 1);
  for( ; ; )
  {
    capture >> frame;
    //  if(!frame)
    //     break;
    imshow("w", frame);
    waitKey(20); // waits to display frame
  }
  waitKey(0); 
}
