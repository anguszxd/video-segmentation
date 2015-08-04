/************
   代码阅读顺序
BackGroundModel::classificationMThread，是代码的入口

*************/

#include "StdAfx.h"
#include "BackGroundModel.h"
#include <iostream>
#include <time.h> 
#include <fstream>

#define MAX_THREAD 10

using namespace std;

DWORD WINAPI ThreadClassification(LPVOID pParam)
{
	ParameterClassification *param = (ParameterClassification *)pParam;
	
	int nWidth = param->m_nWidth; 
	int nHeight = param->m_nHeight;
	int nRadius = param->m_nRadius;
	int nThreshold = param->m_nThreshold;
	int	nModelSize = param->m_nModelSize;
	int nRowIndex = param->m_nRowIndex;
	int nRows = param->m_nRows;
	
	byte * pForeground	= param->m_pForeground; 
	byte * pIntensity	= param->m_pIntensity; 
	byte * pModel		= param->m_pModel; 
	 
	int nImageSize = nWidth * nHeight;  
	
	int nDifference = 0; 
	int nOffset = 0;
	int nIntensityModel, nIntensityFrame; 
	for(int n = 0; n < nWidth * nRows ; n++)
	{
		int nCount = 0; 
		int bUpdate = false;  
		// set to be foreground
		pForeground[n] = pIntensity[n];
		for(int j = 0; j < nModelSize; j++)
		{
			// offset for the same pixel of the model 
			nOffset =  j * nImageSize + nRowIndex * nWidth + n;
			nIntensityModel = pModel[nOffset];
			nIntensityFrame = pIntensity[n];
			nDifference =  nIntensityModel - nIntensityFrame;
			if(nDifference <= nRadius && nDifference >= -nRadius)
			{
				nCount++; 
			}
			// classfiy as background
			if(nCount >= nThreshold)
			{
				pForeground[n] = 0;
				bUpdate = true;
				break; 
			}	
		}

		// 分别在时间域和空间域 update a backround pixel; 
		if(bUpdate)
		{	
			//initialize random seed: 
			srand ((unsigned int) time(NULL) );
			 //generate randaom number: 
			int nIndex  = rand() % nModelSize;

			// update randomly in time sequence
			nOffset =  nIndex * nImageSize + nRowIndex * nWidth + n;
			pModel[nOffset] = pIntensity[n];
			srand ((unsigned int) time(NULL) );
			nIndex  = rand() % 8;

			// update randomly in spatially 
			nOffset = getOffset(nOffset, nWidth, nHeight, nIndex); 
			if(nOffset > 0)
			{
				pModel[nOffset] = pIntensity[n];
			}
		}
	}
	return 0;
}


BackGroundModel::BackGroundModel(
	int nWidth, 
	int nHeight, 
	int nModelSize, 
	int nRadius, 
	int nThreshold 
)
{
	try{
		m_nWidth = nWidth; 
		m_nHeight = nHeight;
		m_nModelSize = nModelSize; 
		m_nRadius = nRadius;
		m_nForeBackThreshold = nThreshold;
		m_nInitCount = 0;
		m_bModelInitialized = false;
		initialModelMemory();
	}catch(exception &e)
	{
		throw e;
	};
}

BackGroundModel::~BackGroundModel(void)
{
	if(m_pModel)
	{
		delete [] m_pModel;
		m_pModel = NULL;
	}
}

//void BackGroundModel::preprocess(byte * pIntensity, int nWidth, int nHeight)
//{
//	IplImage * pGrayImage = cvCreateImage(cvSize(nWidth,nHeight),8,1);
//	IplImage * pTmpImage = cvCreateImage(cvSize(nWidth,nHeight),8,1);
//	char * pTmp = pGrayImage->imageData;
//	pGrayImage->imageData = (char *) pIntensity; 
//	cvSmooth(pGrayImage, pTmpImage, CV_MEDIAN, 3, 3);
//	//cvEqualizeHist(pTmpImage, pGrayImage);
//	pIntensity = (byte *)pTmpImage->imageData;
//	pTmpImage->imageData = pTmp;
//	cvReleaseImage(&pTmpImage);
//	cvReleaseImage(&pGrayImage);
//}

//void BackGroundModel::postprocess(byte * pForeground, int nWidth, int nHeight)
//{
//	IplImage * pGrayImage = cvCreateImage(cvSize(nWidth,nHeight),8,1);
//	IplImage * pTmpImage = cvCreateImage(cvSize(nWidth,nHeight),8,1);
//	char * pTmp = pGrayImage->imageData;
//	pGrayImage->imageData = (char *) pForeground;
//	cvDilate(pGrayImage, pTmpImage, NULL, 1); 
//	cvErode( pTmpImage, pGrayImage, NULL, 2); 
//	cvDilate(pGrayImage, pTmpImage, NULL, 3); 
//	pForeground = (byte *) pTmpImage->imageData;
//	pTmpImage->imageData = pTmp;
//	cvReleaseImage(&pTmpImage);
//	cvReleaseImage(&pGrayImage);
//}


//代码入口
//classificationMThread 输入一次为
//输入图像数据，图像宽，图像高，输出判别结果
int BackGroundModel::classificationMThread(byte * pIntensity,  int nWidth, int nHeight, byte * pForeground)
{
	try
	{
		//preprocess(pIntensity, nWidth, nHeight);
		
		//初始化模型，由m_nModelSize参数控制，要预先读入多少幅图像
		//读入的数据放入m_pModel中
		if(!m_bModelInitialized)
		{
			//参数：图像素，宽，高，初始化计数
			initialModelData(pIntensity, nWidth, nHeight, m_nInitCount); 
			m_nInitCount++;  //初始为0，记录初始图像的帧数
			return 0; 
		} 
		if(!pIntensity || nWidth != m_nWidth || nHeight!= m_nHeight)
		{
			throw exception("Input intensity image is not correct");
		}
		if(!pForeground)
		{
			throw exception("Foregrund is not initialized");
		}
		
		//开始算法
		HANDLE hThread[MAX_THREAD];
		int nDivision = nHeight / MAX_THREAD; //将图像分为10块，供10个线程同时处理
		int nIndex = 0; 

		ParameterClassification param[MAX_THREAD];
		for(int i = 0; i < MAX_THREAD; i++)
		{
			
			param[i].m_nHeight = nHeight;
			param[i].m_nWidth = nWidth;
			param[i].m_nRadius = m_nRadius;
			param[i].m_nRowIndex = i * nDivision;
			int nIndex = i * nDivision * nWidth;  //各个块的起始点
			//防止图像高不能被MAX_THREAD整除
			if( i == MAX_THREAD -1)
			{
				int nVal = nHeight %  MAX_THREAD;
				if(nVal !=0)
				{
					nDivision = nVal;
				} 
			}
			param[i].m_nRows = nDivision;  
			param[i].m_nThreshold = m_nForeBackThreshold; 
			param[i].m_nModelSize = m_nModelSize; //采样点的数量
			param[i].m_pModel = m_pModel;  //图像数据指针
			param[i].m_pIntensity = pIntensity + nIndex; 
			param[i].m_pForeground = pForeground + nIndex;
			
			//CreateThread创建线程函数，windows API中的函数
			//ThreadClassification是自己写的函数
			/*CreateThread函数说明
			lpThreadAttributes：指向SECURITY_ATTRIBUTES型态的结构的指针。
			在Windows 98中忽略该参数。在Windows NT中，它被设为NULL，表示使用缺省值。 　
			　
			dwStackSize，线程堆栈大小，一般=0，在任何情况下，Windows根据需要动态延长堆栈的大小。
			
			lpStartAddress，指向线程函数的指针，形式：@函数名，函数名称没有限制，
			但是必须以下列形式声明：
			DWORD WINAPI ThreadProc (LPVOID lpParam) ，格式不正确将无法调用成功。 　　
			
			lpParameter：向线程函数传递的参数，是一个指向结构的指针，不需传递参数时，为NULL。 　
			
			dwCreationFlags ：线程标志,可取值如下
			（1）CREATE_SUSPENDED-----创建一个挂起的线程， 　　
			（2）0--------------------表示创建后立即激活。 　　
			
			lpThreadId:保存新线程的id。 　　
			返回值: 函数成功，返回线程句柄；函数失败返回false。
			*/

			hThread[i] = CreateThread(NULL,0,ThreadClassification,param + i,0,0);
			if (hThread[i] == NULL) 
			{
				throw exception("Failed to create thread for classification");
			}
		}
		
		//windowsAPI函数，可以等待几乎所有内核对象
		DWORD bRet = WaitForMultipleObjects(MAX_THREAD, hThread , TRUE, INFINITE);

		for(int i=0; i< MAX_THREAD; i++)
		{
			//关闭线程
			CloseHandle(hThread[i]);
		}

		//原作者希望有一个滤波函数
		//postprocess(pForeground, nWidth, nHeight);

		return 0; 
	}catch(exception &e)
	{
		throw e;
	}
}

//
//int BackGroundModel::classificationMThreadIPP(byte * pIntensity, int nWidth, int nHeight, byte * pForeground)
//{
//	try
//	{
//		if(!m_bModelInitialized)
//		{
//			this->initialModelData(pIntensity, nWidth, nHeight, m_nInitCount); 
//			m_nInitCount++;
//			return 0; 
//		} 
//		if(!pIntensity || nWidth != m_nWidth || nHeight!= m_nHeight)
//		{
//			throw exception("Input intensity image is not correct");
//		}
//		if(!pForeground)
//		{
//			throw exception("Foregrund is not initialized");
//		}
//		byte * pDifferenceModel = NULL;
//		if(!(pDifferenceModel = new byte[m_nArraySize])) 
//		{
//			throw exception("Memory allocation failed");
//		}
//		// calculate the abolute difference with IPP
//		IppStatus status; 
//		IppiSize roiSize;
//		roiSize.width = nWidth;
//		roiSize.height = nHeight;
//		
//		for(int i = 0; i < m_nModelSize; i++)
//		{
//			int nIndex = i * m_nImageSize; 
//			status = ippiAbsDiff_8u_C1R(m_pModel + nIndex, nWidth, pIntensity, nWidth, pDifferenceModel + nIndex, nWidth, roiSize);
//		}
//		HANDLE hThread[MAX_THREAD];
//		int nDivision = nHeight / MAX_THREAD;
//		int nIndex = 0; 
//
//		ParameterClassification param[MAX_THREAD];
//		for(int i = 0; i < MAX_THREAD; i++)
//		{
//			
//			param[i].m_nHeight = nHeight;
//			param[i].m_nWidth = nWidth;
//			param[i].m_nRadius = m_nRadius;
//			param[i].m_nRowIndex = i * nDivision;
//			int nIndex = i * nDivision * nWidth;
//			if( i == MAX_THREAD -1)
//			{
//				int nVal = nHeight %  MAX_THREAD;
//				if(nVal !=0)
//				{
//					nDivision = nVal;
//				} 
//			}
//			param[i].m_nRows = nDivision; 
//			param[i].m_nThreshold = m_nForeBackThreshold;
//			param[i].m_nModelSize = m_nModelSize;
//			param[i].m_pModel = m_pModel; 
//			param[i].m_pDifferenceModel = pDifferenceModel ;
//			param[i].m_pIntensity = pIntensity + nIndex; 
//			param[i].m_pForeground = pForeground + nIndex;
//			
//			hThread[i] = CreateThread(NULL,0,ThreadClassificationIPP,param + i,0,0);
//			if (hThread[i] == NULL) 
//			{
//				throw exception("Failed to create thread for classification");
//			}
//		}
//		
//		DWORD bRet = WaitForMultipleObjects(MAX_THREAD, hThread , TRUE, INFINITE);
//
//		for(int i=0; i< MAX_THREAD; i++)
//		{
//			CloseHandle(hThread[i]);
//		}
//		if(pDifferenceModel)
//		{
//			delete [] pDifferenceModel; 
//			pDifferenceModel = NULL; 
//		}
//		return 2;
//	}catch(exception &e)
//	{
//		throw e;
//	}
//} 

int BackGroundModel::classification( byte * pIntensity, int nWidth, int nHeight, byte * pForeground)
{
	try
	{
		if(!m_bModelInitialized)
		{
			this->initialModelData(pIntensity, nWidth, nHeight, m_nInitCount); 
			m_nInitCount++;
			return 0; 
		} 
		if(!pIntensity || nWidth != m_nWidth || nHeight!= m_nHeight)
		{
			throw exception("Input intensity image is not correct");
		}
		if(!pForeground)
		{
			throw exception("Foregrund is not initialized");
		}
		// perform the classifiction 
		int  nDifference = 0; 
		int nOffset = 0;
		for(int i = 0; i < m_nImageSize; i++)
		{
			int nCount = 0; 
			int bUpdate = false;
			// set to be foreground
			pForeground[i] = 255;
			for(int j = 0; j < m_nModelSize; j++)
			{
				nOffset =  j * m_nImageSize + i;
				nDifference = m_pModel[nOffset] - pIntensity[i];
				if(nDifference <= m_nRadius && nDifference >= -m_nRadius)
				{
					nCount++; 
				}
				// classfiy as background
				if(nCount >= m_nForeBackThreshold)
				{
					pForeground[i] = 0;
					bUpdate = true;
					break; 
				}
			}
			// update a backround pixel;
			if(bUpdate)
			{	
				// update randomly in time sequence
				//initialize random seed: 
				srand ((unsigned int) time(NULL) );
				 //generate randaom number: 
				int nIndex  = rand() % m_nModelSize;
				nOffset =  nIndex * m_nImageSize + i;
				m_pModel[nOffset] = pIntensity[i];
				srand ((unsigned int) time(NULL) );
				nIndex  = rand() % 8;
				// update randomly in spatially 
				nOffset = getOffset(nOffset, nWidth, nHeight, nIndex); 
				if(nOffset > 0)
				{
					m_pModel[nOffset] = pIntensity[i];
				}
			}
		}
		return 1;
	}catch(exception &e)
	{
		throw e; 
	}
}

//参数：图像素，宽，高，初始化计数
void BackGroundModel::initialModelData(byte * pIntensity, int nWidth, int nHeight, int nIndex)
{
	try
	{
		// input validation 
		if(!pIntensity)
		{
			throw exception("Image data is not correct"); 
		}
		if(nWidth != m_nWidth || nHeight != m_nHeight)
		{
			throw exception("Input image width or height does not match model");
		}
		if(nIndex > m_nModelSize || nIndex <0) 
		{
			throw exception("Model index out of model size"); 
		}
		//nOffset是一个偏移量，将连续几张图片的数据读入m_pModel
		int nOffset = m_nImageSize * nIndex; 

		//由src指向地址为起始地址的连续n个字节的数据复制到以dest指向地址为起始地址的空间内
		//void *memcpy(void *dest, const void *src, size_t n);
		memcpy(m_pModel + nOffset, pIntensity, m_nImageSize);

		// check if the model intializaed or not 
		if(nIndex == m_nModelSize - 1)
		{
			m_bModelInitialized = true;
		}
	}catch(exception &e)
	{
		throw e;
	}
}
void BackGroundModel::initialModelMemory()
{
	try
	{
		// Validation of model image width and height
		if(m_nWidth <= 0 || m_nHeight <= 0)
		{
			throw exception("Model image height and width not setting properly");
		}
		// Validation of model size 
		if(m_nModelSize <= 0)
		{
			throw exception("Model size not initialled properly"); 
		}
		m_nImageSize = m_nWidth * m_nHeight; 
		m_nArraySize = m_nImageSize * m_nModelSize; 
		if(!(m_pModel = new byte[m_nArraySize]))
		{
			throw exception("Memory allocation failed");
		}
		memset(m_pModel, 0, m_nArraySize);
	}catch(exception &e)
	{
		throw e;
	}
}

//针对八连通域
int getOffset(int nIndex, int nWidth, int nHeight, int nRandom)
{
	int s_size = nWidth * nHeight;
	int nOffset = -1;
	switch(nRandom)
	{
	case 0:
		if(nIndex - nWidth > 0) 
		{
			nOffset =  nIndex - nWidth;
		}
		break;
	case 1:
		if(nIndex - nWidth + 1 > 0) 
		{
			nOffset = nIndex - nWidth + 1;
		}
		break;
	case 2:
		if(nIndex + 1 <  s_size) 
		{
			nOffset = nIndex + 1;
		}
		break; 
	case 3:
		if(nIndex + nWidth + 1 < s_size) 
		{
			nOffset = nIndex + nWidth + 1;
		}
		break; 
	case 4:
		if(nIndex + nWidth <  s_size)
		{
			nOffset = nIndex + nWidth;
		}
		break; 
	case 5:
		if(nIndex + nWidth -1 < s_size)
		{
			nOffset = nIndex + nWidth - 1;
		}
		break; 
	case 6:
		if(nIndex -1 > 0)
		{
			nOffset = nIndex - 1;
		}
		break; 
	case 7:
		if(nIndex - nWidth - 1 > 0)
		{
			nOffset = nIndex - nWidth - 1;
		}
		break; 
	}
	return nOffset;
}