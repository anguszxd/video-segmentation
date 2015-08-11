//background subtraction with 
// (1)background RGB Gaussian model training
// (2)shadow modelling
// (3)Graphcut cleaning
// (4)non-recursive largest blob finding
//
//written by Li Guan
//lguan@cs.unc.edu
//Oct 2nd, 2008


#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include "graph.h"
#include "blob.h"
#include "common.h"

IplImage *frame = NULL, *frame1 = NULL, *temp = NULL, *mean_image = NULL, 
		*test1_image = NULL, *test2_image = NULL, *test3_image = NULL, *test4_image = NULL;

//for debug purpose only
void on_mouse( int event, int x, int y, int flags )
{
	double Mr, Mg, Mb, R, G, B, nR, nG, nB;
	double CD, ratio;

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:		
		printf("Location: horizontal - %d, vertical - %d\n", x, y);
		Mr = ((unsigned char *)(mean_image->imageData + mean_image->widthStep*y))[x*3];
		Mg = ((unsigned char *)(mean_image->imageData + mean_image->widthStep*y))[x*3+1];
		Mb = ((unsigned char *)(mean_image->imageData + mean_image->widthStep*y))[x*3+2];
		printf("Background value: %lf, %lf, %lf\n", Mr, Mg, Mb);
		R = ((unsigned char *)(frame1->imageData + frame1->widthStep*y))[x*3];
		G = ((unsigned char *)(frame1->imageData + frame1->widthStep*y))[x*3+1];
		B = ((unsigned char *)(frame1->imageData + frame1->widthStep*y))[x*3+2];
		nR = R/Mr;
		nG = G/Mg;
		nB = B/Mb;
		ratio = sqrt(nR*nR + nG*nG + nB*nB)/sqrt(3.0);		
		if(ratio>0.3 && ratio<1.0)
			CD = sqrt((nR-ratio)*(nR-ratio)+(nG-ratio)*(nG-ratio)+(nB-ratio)*(nB-ratio))*ratio;
		else if(ratio>0.3)
			CD = -1.0;
		else CD = 0.0;
		printf("Foreground value: %lf, %lf, %lf\n", R, G, B);
		printf("Ratio value: %lf, %lf, %lf\n", R/Mr, G/Mg, B/Mb);
		printf("CD = %lf\tShadow weight: %d\n", CD, ((unsigned char *)(test2_image->imageData + test2_image->widthStep*y))[x]);
		//printf("C")
        break;
	default:
		;
    }
}

int main(void)
{
	unsigned int i,j;
	double R,G,B;
	
	char fileAbsoluteName[500];
	char dir[300] = "../sampleData/PillarSequence/bg/";
	char inputext[] = ".bmp";	
	char bgFilename[] = "bg";
	int digit = 1;
	int backgroundStart = 0;
	int backgroundEnd =	0;

	char dirI[] = "../sampleData/PillarSequence/fg/";
	char fgFilename[] = "fg";
	int foregroundStart = 0;
	int foregroundEnd =	9;

	char outputdir[] = "../sampleData/result";
	char outputext[] = ".bmp";

	double defaultBackgroundVariance = 49.0/255.0/255.0; //deviation^2
	double expan = 0.6; //expan for the deviation for background modes
	double PFprior = 0.3; //the prior for a pixel to be foreground pixel
	double PF = 1.0/100000.0;//the possibility given a color to be foreground (ideally should be 1/(255)^3)
	double shadow_calculation_threshold_soft = 0.9;	// 0 - 1
	double shadow_calculation_threshold_hard = 0.3;	// 0 - 1
	double shadow_sigma_hard = 0.025; //shadow length [0 - sqrt(3)]
	double shadow_sigma_soft = 1.7; //shadow length [0 - sqrt(3)]
	double CUT_alpha = 0.2;
	double CUT_fWeight = 0.5;
	double CUT_bWeight;

	long current_frame = 0;
	static IplImage *frame = NULL, *frame1 = NULL, *mean_image = NULL, *test1_image = NULL, *test2_image = NULL;

	if((frame = cvLoadImage(generateFileName(backgroundStart, inputext, bgFilename, digit, fileAbsoluteName, dir))) == NULL){
		printf("can not locate input image: %s.\n", fileAbsoluteName);
		return -1;
	}

	CvSize frame_size;
	frame_size = cvGetSize(frame);
	cvReleaseImage(&frame);	

	frame1 = cvCreateImage( frame_size, IPL_DEPTH_8U, 3 );
	mean_image = cvCreateImage( frame_size, IPL_DEPTH_8U, 3 );
	test1_image = cvCreateImage( frame_size, IPL_DEPTH_8U, 1 );
	test2_image = cvCreateImage( frame_size, IPL_DEPTH_8U, 1 );
	test3_image = cvCreateImage( frame_size, IPL_DEPTH_8U, 1 );
	test4_image = cvCreateImage( frame_size, IPL_DEPTH_8U, 1 );
	if ( frame1 == NULL ||  mean_image == NULL || test1_image == NULL ||
		test2_image == NULL || test3_image == NULL || test4_image == NULL)
	{
		fprintf(stderr, "Error: Couldn't allocate image. Out of memory?\n");
		exit(-1);
	}

	double *Mr = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *Mg = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *Mb = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *Crr = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *Cgg = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *Cbb = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);

	current_frame = 1;
	int frameIndex;
	double oldMeanR, oldMeanG, oldMeanB;

	for (i=0; i< frame_size.height; i++) {
		for (j=0; j < frame_size.width; j++) {
			Mr[frame_size.width*i+j] = 0;
			Mg[frame_size.width*i+j] = 0;
			Mb[frame_size.width*i+j] = 0;
			Crr[frame_size.width*i+j] = 0;
			Cgg[frame_size.width*i+j] = 0;
			Cbb[frame_size.width*i+j] = 0;
		}
	}

	current_frame = 1;
	for(frameIndex = backgroundStart;frameIndex <= backgroundEnd;frameIndex++)
	{
		if((frame = cvLoadImage(generateFileName(frameIndex, inputext, bgFilename, digit, fileAbsoluteName, dir))) == NULL){
			printf("can not locate input image: %s.\n", fileAbsoluteName);
			return -1;
		}	

		cvConvertImage(frame, frame1, 0);	

		for (i=0; i< (int)frame_size.height; i++) {
			for (j=0; j < (int)frame_size.width; j++) {
				R = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3];
				G = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+1];
				B = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+2];
				R /= 255.0;
				G /= 255.0;
				B /= 255.0;

				oldMeanR = Mr[frame_size.width*i+j];
				oldMeanG = Mg[frame_size.width*i+j];
				oldMeanB = Mb[frame_size.width*i+j];

				Mr[frame_size.width*i+j] = (Mr[frame_size.width*i+j]*(double)(current_frame-1)+R)/(double)current_frame;
				Mg[frame_size.width*i+j] = (Mg[frame_size.width*i+j]*(double)(current_frame-1)+G)/(double)current_frame;
				Mb[frame_size.width*i+j] = (Mb[frame_size.width*i+j]*(double)(current_frame-1)+B)/(double)current_frame;

				//Variance = ((B-Mean).*(B-Mean) + Variance*(num-1) + (num-1)*(oldMean - Mean).*(oldMean - Mean))/num;
				Crr[frame_size.width*i+j] = ((R-Mr[frame_size.width*i+j])*(R-Mr[frame_size.width*i+j]) + Crr[frame_size.width*i+j]*(double)(current_frame-1) + (double)(current_frame-1)*(oldMeanR - Mr[frame_size.width*i+j])*(oldMeanR - Mr[frame_size.width*i+j]))/(double)(current_frame);
				Cgg[frame_size.width*i+j] = ((G-Mg[frame_size.width*i+j])*(G-Mg[frame_size.width*i+j]) + Cgg[frame_size.width*i+j]*(double)(current_frame-1) + (double)(current_frame-1)*(oldMeanG - Mg[frame_size.width*i+j])*(oldMeanG - Mg[frame_size.width*i+j]))/(double)(current_frame);
				Cbb[frame_size.width*i+j] = ((B-Mb[frame_size.width*i+j])*(B-Mb[frame_size.width*i+j]) + Cbb[frame_size.width*i+j]*(double)(current_frame-1) + (double)(current_frame-1)*(oldMeanB - Mb[frame_size.width*i+j])*(oldMeanB - Mb[frame_size.width*i+j]))/(double)(current_frame);
			}
		}
		printf("%d\n",current_frame);
		current_frame++;
		cvReleaseImage(&frame);
	}//end of for

	if(backgroundStart==backgroundEnd){
		for (i=0; i< frame_size.height; i++) {
			for (j=0; j < frame_size.width; j++) {
				Crr[frame_size.width*i+j] = defaultBackgroundVariance;
				Cgg[frame_size.width*i+j] = defaultBackgroundVariance;
				Cbb[frame_size.width*i+j] = defaultBackgroundVariance;
			}
		}
	}
	//store deviation in Cxx 
	for (i=0; i< frame_size.height; i++) {
		for (j=0; j < frame_size.width; j++) {
			Crr[frame_size.width*i+j] = sqrt(Crr[frame_size.width*i+j]);
			Cgg[frame_size.width*i+j] = sqrt(Cgg[frame_size.width*i+j]);
			Cbb[frame_size.width*i+j] = sqrt(Cbb[frame_size.width*i+j]);
			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3] = (char)(Mr[frame_size.width*i+j]*255.0);
			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3+1] = (char)(Mg[frame_size.width*i+j]*255.0);
			((unsigned char *)(mean_image->imageData + mean_image->widthStep*i))[j*3+2] = (char)(Mb[frame_size.width*i+j]*255.0);
		}
	}

	cvNamedWindow("Mean Image", CV_WINDOW_AUTOSIZE);
	cvShowImage("Mean Image", mean_image);


//-----------------------------------------------------------------------------------------------------------------------------------------------

	double *finalWeight = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *backgroundWeight = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);
	double *shadowWeight = (double*)malloc(sizeof(double)*frame_size.height*frame_size.width);

	cvNamedWindow("Backgound Weight", CV_WINDOW_AUTOSIZE);
	cvMoveWindow( "Backgound Weight", 320, 0 );
	cvNamedWindow("Shadow Weight", CV_WINDOW_AUTOSIZE);
	cvMoveWindow( "Shadow Weight", 640, 0);
	cvNamedWindow("Final Weight", CV_WINDOW_AUTOSIZE);
	cvMoveWindow( "Final Weight",  960, 0);
	cvNamedWindow("Final Cut Result", CV_WINDOW_AUTOSIZE);
	cvMoveWindow( "Final Cut Result", 960, 280 );

	printf("foregroundEnd = %d\n", foregroundEnd);
	for(int frameIndex = foregroundStart;frameIndex <= foregroundEnd;frameIndex++)
	{
		if((frame = cvLoadImage(generateFileName(frameIndex, inputext, fgFilename, digit, fileAbsoluteName, dirI))) == NULL){
			printf("can not locate input image: %s.\n", fileAbsoluteName);
			return -1;
		}
				
		printf("frame index - %d\n", frameIndex);

		// ### the reason that we convert is because of compatibility with avi file or real-time video, because of its inversion!!!!
		cvConvertImage(frame, frame1, 0);

		for (i=0; i< (unsigned int)frame_size.height; i++) {
			for (j=0; j < (unsigned int)frame_size.width; j++) {
				R = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3];
				G = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+1];
				B = ((unsigned char *)(frame1->imageData + frame1->widthStep*i))[j*3+2];
				R /= 255.0;
				G /= 255.0;
				B /= 255.0;
				double R1 = R - Mr[frame_size.width*i+j];
				double G1 = G - Mg[frame_size.width*i+j];
				double B1 = B - Mb[frame_size.width*i+j];
				double crr = Crr[frame_size.width*i+j]*expan;
				double cgg = Cgg[frame_size.width*i+j]*expan;
				double cbb = Cbb[frame_size.width*i+j]*expan;

				double forR, forG, forB;
				if(crr<1e-5)
					crr = 1e-5;
				forR = R1*R1/crr/crr;
				if(cgg<1e-5)
					cgg = 1e-5;
				forG = G1*G1/cgg/cgg;
				if(cbb<1e-5)
					cbb = 1e-5;
				forB = B1*B1/cbb/cbb;

				double index = forR + forG + forB;				
				backgroundWeight[i*frame_size.width+j] = exp(-0.5*index)/2.0/pi/sqrt(2.0*pi)/crr/cgg/cbb;
				backgroundWeight[i*frame_size.width+j] = backgroundWeight[i*frame_size.width+j]*(1.0-PFprior)/(backgroundWeight[i*frame_size.width+j]*(1.0-PFprior)+PF*PFprior);
				if(backgroundWeight[i*frame_size.width+j]<0.000000000000000000000000000000000000001)
					backgroundWeight[i*frame_size.width+j] = 0.000000000000000000000000000000000000001;

				((unsigned char *)(test1_image->imageData + test1_image->widthStep*i))[j] = (unsigned char)(backgroundWeight[frame_size.width*i+j]*255.0);

				// shadow probability ------------------------------------------------------------------------------------------
				// here normalized to (0,0,0)-(1,1,1) cube
				shadowWeight[frame_size.width*i+j]=0.0;
				//if(((unsigned char *)(test1_image->imageData + test1_image->widthStep*i))[j] == 0){//only test when shadow might be mistakenly set as foreground
					double nR = R/Mr[frame_size.width*i+j];
					double nG = G/Mg[frame_size.width*i+j];
					double nB = B/Mb[frame_size.width*i+j];
					double ratio = sqrt(nR*nR + nG*nG + nB*nB)/sqrt(3.0);
					if(ratio>shadow_calculation_threshold_hard && ratio<=shadow_calculation_threshold_soft){	// intensity not too small, and still smaller than mean pixel intensity, trigue shadow calculation
						double CD = sqrt((nR-ratio)*(nR-ratio)+(nG-ratio)*(nG-ratio)+(nB-ratio)*(nB-ratio))*ratio;
						shadowWeight[frame_size.width*i+j]=exp(-(CD*CD)/shadow_sigma_hard/shadow_sigma_hard*0.1);
						//shadowWeight[frame_size.width*i+j]=CD*3.0;
					}
					if(ratio>shadow_calculation_threshold_soft && ratio<1.0){	// intensity not too small, and still smaller than mean pixel intensity, trigue shadow calculation
						double CD = sqrt(nR*nR+nG*nG+nB*nB-(nR+nG+nB)*(nR+nG+nB)/9.0); //according to http://www.cs.unc.edu/~lguan/COMP255.files/FinalReport.htm
						shadowWeight[frame_size.width*i+j]=exp(-(CD*CD)/shadow_sigma_soft/shadow_sigma_soft*0.5);
						//shadowWeight[frame_size.width*i+j]=CD*3.0;
					}
				//}
				
				((unsigned char *)(test2_image->imageData + test2_image->widthStep*i))[j] = (unsigned char)(shadowWeight[frame_size.width*i+j]*255.0);
				
				finalWeight[frame_size.width*i+j] = shadowWeight[frame_size.width*i+j]>backgroundWeight[frame_size.width*i+j]?shadowWeight[frame_size.width*i+j]:backgroundWeight[frame_size.width*i+j];				
				//((unsigned char *)(test3_image->imageData + test3_image->widthStep*i))[j] = (unsigned char)(((finalWeight[frame_size.width*i+j]>0.1)?1:0)*255.0);
				((unsigned char *)(test3_image->imageData + test3_image->widthStep*i))[j] = 255 - (unsigned char)(finalWeight[frame_size.width*i+j]*255.0);

				//if (finalWeight[frame_size.width*i+j]<0.1)
				//	((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 255;
				//else
				//	((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 0;
			}
		}
		
		// begin graphcut ---------------------------------------------------------------------------------------------		
		Graph *g = new Graph();			// create a new graph.
		Graph::node_id *nodes;			// initialize the graph nodes.
		nodes = (Graph::node_id *)malloc(sizeof(Graph::node_id)*frame_size.height*frame_size.width);
		for (i=0; i< (unsigned int)frame_size.height; i++) {
			for (j=0; j < (unsigned int)frame_size.width; j++) {
				CUT_bWeight = 1 - finalWeight[frame_size.width*i+j];
				nodes[frame_size.width*i+j] = g->add_node();
				g -> set_tweights(nodes[frame_size.width*i+j], CUT_fWeight, CUT_bWeight);
				
				// using four connection
				if(i!=0)					//not on the first row of the image
					g -> add_edge(nodes[frame_size.width*i+j], nodes[frame_size.width*(i-1)+j], CUT_alpha, CUT_alpha);
				
				if(j!=0)					//not on the first row of the image
					g -> add_edge(nodes[frame_size.width*i+j], nodes[frame_size.width*i+(j-1)], CUT_alpha, CUT_alpha);
			}
		}
		double flow = g -> maxflow();
		for (i=0; i< (unsigned int)frame_size.height; i++) {
			for (j=0; j < (unsigned int)frame_size.width; j++) {
				if (g->what_segment(nodes[frame_size.width*i+j]) == Graph::SOURCE)
					((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 0;
				else
					((unsigned char *)(test4_image->imageData + test4_image->widthStep*i))[j] = 128;
			}
		}
		delete nodes;
		delete g;

		//preserve the largest blob of the binary image
		findLaregeBlob((unsigned char *)(test4_image->imageData), frame_size.height, frame_size.width);

		//cvShowImage("Mean Image", frame1);
		cvShowImage("Backgound Weight", test1_image);
		cvShowImage("Shadow Weight", test2_image);
		cvShowImage("Final Weight", test3_image);
		cvShowImage("Final Cut Result", test4_image);
		cvReleaseImage(&frame);		
				
		
		//save the image
		cvSaveImage(generateFileName(frameIndex, outputext, fgFilename, digit, fileAbsoluteName, outputdir), test4_image);	    
		char c = cvWaitKey(0);
		if(c=='x')
			break;
	}

	cvDestroyWindow("Mean Image");
	cvDestroyWindow("Backgound Weight");
	cvDestroyWindow("Shadow Weight");
	cvDestroyWindow("Final Weight");
	cvDestroyWindow("Final Cut Result");

	printf("X!\n");
    return 0;
}

