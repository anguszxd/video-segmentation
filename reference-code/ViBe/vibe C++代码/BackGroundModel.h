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
	int m_nModelSize;  //���������������P�������
	int m_nRadius; //���뾶
	int m_nThreshold; //��ֵ
	byte * m_pModel;  //ͼ������ָ��
	int m_nWidth;  //ͼ��Ŀ�
	int m_nHeight; //ͼ��ĸ�
	 byte * m_pIntensity; //�ֿ��ͼ�����ݵ�ָ�룬�����ƶ�
	int m_nRowIndex; //���������ʼ��־λ
	int m_nRows;  //û�б�MAX_THREAD�������У����������
	byte * m_pForeground; //�ֿ��ǰ����ָ�룬�����ƶ�
	byte * m_pDifferenceModel;
};

