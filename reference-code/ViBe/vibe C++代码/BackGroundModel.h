#pragma once

class BackGroundModel
{
public:
	// class constructor
	BackGroundModel(
		int nWidth,					///< Width of the images
		int nHeight,				///< Height of the images
		int nModelSize = 20,		///< Number of samples keeped in the model
		int nRadius = 30,			///< Radius - Intensity radius reduce to 10 for low contrast images 
		int nThreshold = 2			///< Threshold is set to:  nModelSize / 10
		);
		
	~BackGroundModel(void);
	
	// Main caller function 
	int classificationMThread( 
		byte *pIntensity,	    	///< Input of gray scale pixel array 
		int nWidth,		        	///< Width of gray scale image
		int nHeight,			    ///< Height of gray scale image 
		byte * pForeground			///< Output of foreground images plz allocate your own memory
	);
	
	// classification foreground
	int classification(byte *pIntensity, int nWidth, int nHeight, byte * pForeground);

protected:
	int m_nWidth;				///< Width of the back ground model
	int m_nHeight;				///< Height of the backgroud model 
	byte * m_pModel;			///< Pointer to hold the model intensity values
	int m_nModelSize;			///< Initial model size - to be read from configration files;
	int m_nRadius;				///< Radius of the threshold range 
	int m_nForeBackThreshold;	///< Threshhold to determine the foreground and background
private:
	// Inital model with sample slice
	 byte * getModel(){return m_pModel;}

	int m_nInitCount; 
	bool m_bModelInitialized;
	int m_nArraySize;
	int m_nImageSize; 
	void initialModelMemory();	///< Allocate a memory location for the model    
	void initialModelData(byte * pIntensity, int nWidth, int nHeight, int nIndex);
};

int getOffset(int nIndex, int nWidth, int nHeight, int nRandom);
	
// define parameters used in the thread for classification 
class ParameterClassification
{
	public:
	int m_nModelSize;  //采样点的数量，即P点的数量
	int m_nRadius; //检测半径
	int m_nThreshold; //阈值
	byte * m_pModel;  //图像数据指针
	int m_nWidth;  //图像的宽
	int m_nHeight; //图像的高
	 byte * m_pIntensity; //分块后图像数据的指针，可以移动
	int m_nRowIndex; //各个块的起始标志位
	int m_nRows;  //没有被MAX_THREAD整除的行，多余的行数
	byte * m_pForeground; //分块后前景的指针，可以移动
	byte * m_pDifferenceModel;
};

