// #include "stdafx.h"
#include "MyCamera.h"
#include <opencv.hpp>
#include"opencv2/opencv.hpp"
#include"opencv2/imgproc/types_c.h"

CMyCamera::CMyCamera()
{
    m_hDevHandle        = NULL;
    m_nTLayerType       = MV_UNKNOW_DEVICE;
}

CMyCamera::~CMyCamera()
{
    if (m_hDevHandle)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle    = NULL;
    }
}

// ö�����
int CMyCamera::EnumDevices(MV_CC_DEVICE_INFO_LIST* pstDevList)
{
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, pstDevList);
    if (MV_OK != nRet)  
    {
        return nRet;
    }

    return MV_OK;
}

// ch:���豸 | en:Open Device
int     CMyCamera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
    if (NULL == pstDeviceInfo)
    {
        return MV_E_PARAMETER;
    }

    int nRet = MV_OK;
    if(m_hDevHandle == NULL)
    {
        nRet  = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
        if (MV_OK != nRet)
        {
            return nRet;
        }
    }

    nRet = MV_CC_OpenDevice(m_hDevHandle);
    if (MV_OK != nRet)
    {
        MV_CC_DestroyHandle(m_hDevHandle);
        m_hDevHandle = NULL;

        return nRet;
    }

    return MV_OK;
}


// ch:�ر��豸 | en:Close Device
int     CMyCamera::Close()
{
    int nRet = MV_OK;

    if (NULL == m_hDevHandle)
    {
        return MV_E_PARAMETER;
    }

    MV_CC_CloseDevice(m_hDevHandle);
    nRet = MV_CC_DestroyHandle(m_hDevHandle);
    m_hDevHandle = NULL;

    return nRet;
}


// ch:����ץͼ | en:Start Grabbing
int     CMyCamera::StartGrabbing()
{
    return MV_CC_StartGrabbing(m_hDevHandle);
}


// ch:ֹͣץͼ | en:Stop Grabbing
int     CMyCamera::StopGrabbing()
{
    return MV_CC_StopGrabbing(m_hDevHandle);
}


int     CMyCamera::GetOneFrameTimeout(unsigned char* pData, unsigned int* pnDataLen, unsigned int nDataSize, MV_FRAME_OUT_INFO_EX* pFrameInfo, int nMsec)
{
    if (NULL == pnDataLen)
    {
        return MV_E_PARAMETER;
    }

    int nRet = MV_OK;

    *pnDataLen  = 0;

    nRet = MV_CC_GetOneFrameTimeout(m_hDevHandle, pData, nDataSize, pFrameInfo, nMsec);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pnDataLen = pFrameInfo->nFrameLen;

    return nRet;
}


// ch:������ʾ���ھ�� | en:Set Display Window Handle
int     CMyCamera::Display(void* hWnd)
{
    return MV_CC_Display(m_hDevHandle, hWnd);
}


int CMyCamera::SaveImage(MV_SAVE_IMAGE_PARAM_EX* pstParam)
{
    if (NULL == pstParam)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SaveImageEx2(m_hDevHandle, pstParam);
}

// ch:ע��ͼ�����ݻص� | en:Register Image Data CallBack
int CMyCamera::RegisterImageCallBack(void(__stdcall* cbOutput)(unsigned char * pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, 
                                                                void* pUser),void* pUser)
{
    return MV_CC_RegisterImageCallBackEx(m_hDevHandle, cbOutput, pUser);
}


// ch:ע����Ϣ�쳣�ص� | en:Register Message Exception CallBack
int     CMyCamera::RegisterExceptionCallBack(void(__stdcall* cbException)(unsigned int nMsgType, void* pUser),void* pUser)
{
    return MV_CC_RegisterExceptionCallBack(m_hDevHandle, cbException, pUser);
}


// ch:��ȡInt�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::GetIntValue(IN const char* strKey, OUT unsigned int *pnValue)
{
    if (NULL == strKey || NULL == pnValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_INTVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_INTVALUE));
    int nRet = MV_CC_GetIntValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pnValue = stParam.nCurValue;

    return MV_OK;
}


// ch:����Int�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::SetIntValue(IN const char* strKey, IN unsigned int nValue)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetIntValue(m_hDevHandle, strKey, nValue);
}


// ch:��ȡFloat�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::GetFloatValue(IN const char* strKey, OUT float *pfValue)
{
    if (NULL == strKey || NULL == pfValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_FLOATVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_FLOATVALUE));
    int nRet = MV_CC_GetFloatValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pfValue = stParam.fCurValue;

    return MV_OK;
}


// ch:����Float�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::SetFloatValue(IN const char* strKey, IN float fValue)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}


// ch:��ȡEnum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::GetEnumValue(IN const char* strKey, OUT unsigned int *pnValue)
{
    if (NULL == strKey || NULL == pnValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_ENUMVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_ENUMVALUE));
    int nRet = MV_CC_GetEnumValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    *pnValue = stParam.nCurValue;

    return MV_OK;
}


// ch:����Enum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}


// ch:��ȡBool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
    if (NULL == strKey || NULL == pbValue)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}


// ch:����Bool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Set Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}


// ch:��ȡString�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
// en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::GetStringValue(IN const char* strKey, IN OUT char* strValue, IN unsigned int nSize)
{
    if (NULL == strKey || NULL == strValue)
    {
        return MV_E_PARAMETER;
    }

    MVCC_STRINGVALUE stParam;
    memset(&stParam, 0, sizeof(MVCC_STRINGVALUE));
    int nRet = MV_CC_GetStringValue(m_hDevHandle, strKey, &stParam);
    if (MV_OK != nRet)
    {
        return nRet;
    }

    strcpy_s(strValue, nSize, stParam.chCurValue);

    return MV_OK;
}


// ch:����String�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
// en:Set String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}


// ch:ִ��һ��Command������� UserSetSave����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Execute Command once, such as UserSetSave, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int     CMyCamera::CommandExecute(IN const char* strKey)
{
    if (NULL == strKey)
    {
        return MV_E_PARAMETER;
    }

    return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}

int CMyCamera::GetOptimalPacketSize()
{
    return MV_CC_GetOptimalPacketSize(m_hDevHandle);
}





//int     CMyCamera::GetAllMatchInfo(IN void* hDevHandle, IN unsigned int nTLayerType, OUT unsigned int *nLostFrame, OUT unsigned int *nFrameCount )
//{
//    int nRet = MV_OK;
//    m_hDevHandle = hDevHandle;
//    if (MV_GIGE_DEVICE == nTLayerType)
//    {
//        MV_ALL_MATCH_INFO struMatchInfo = {0};
//        MV_MATCH_INFO_NET_DETECT stMatchInfoNetDetect;
//        struMatchInfo.pInfo = &stMatchInfoNetDetect;
//
//        struMatchInfo.nType = MV_MATCH_TYPE_NET_DETECT; // ch:���������Ͷ�����Ϣ | en:Net flow and lsot packet information
//        memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));
//        struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
//
//        nRet = MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
//        if (MV_OK != nRet)
//        {
//            return nRet;
//        }
//
//        MV_MATCH_INFO_NET_DETECT *pInfo = (MV_MATCH_INFO_NET_DETECT*)struMatchInfo.pInfo;
//        *nFrameCount = pInfo->nNetRecvFrameCount;
//        *nLostFrame = stMatchInfoNetDetect.nLostFrameCount;
//    }
//    else if (MV_USB_DEVICE == nTLayerType)
//    {
//        MV_ALL_MATCH_INFO struMatchInfo = {0};
//        MV_MATCH_INFO_USB_DETECT stMatchInfoNetDetect;
//        struMatchInfo.pInfo = &stMatchInfoNetDetect;
//
//        struMatchInfo.nType = MV_MATCH_TYPE_USB_DETECT; // ch:���������Ͷ�����Ϣ | en:Net flow and lsot packet information
//        memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));
//        struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
//
//        nRet = MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
//        if (MV_OK != nRet)
//        {
//            return nRet;
//        }
//
//        MV_MATCH_INFO_USB_DETECT *pInfo = (MV_MATCH_INFO_USB_DETECT*)struMatchInfo.pInfo;
//        *nFrameCount = pInfo->nRevicedFrameCount;
//        *nLostFrame = pInfo->nErrorFrameCount;
//    }
//
//    return MV_OK;
//}







// ************************************************************************************************
//��������
int CMyCamera::softTrigger()
{
	int tempValue = MV_CC_SetCommandValue(m_hDevHandle, "TriggerSoftware");
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//��ȡ����е�ͼ��
//int ReadBuffer(cv::Mat &image);
//��ȡ����е�ͼ��
int CMyCamera::ReadBuffer(cv::Mat &image)
{
	cv::Mat* getImage = new cv::Mat();
	unsigned int nRecvBufSize = 0;
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	int tempValue = MV_CC_GetIntValue(m_hDevHandle, "PayloadSize", &stParam);
	if (tempValue != 0)
	{
		return -1;
	}
	nRecvBufSize = stParam.nCurValue;
	unsigned char* pDate;
	pDate = (unsigned char *)malloc(nRecvBufSize);

	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	tempValue = MV_CC_GetOneFrameTimeout(m_hDevHandle, pDate, nRecvBufSize, &stImageInfo, 500);
	if (tempValue != 0)
	{
		return -1;
	}
	m_nBufSizeForSaveImage = stImageInfo.nWidth * stImageInfo.nHeight * 3 + 2048;
	unsigned char* m_pBufForSaveImage;
	m_pBufForSaveImage = (unsigned char*)malloc(m_nBufSizeForSaveImage);


	bool isMono;
	switch (stImageInfo.enPixelType)
	{
	case PixelType_Gvsp_Mono8:
	case PixelType_Gvsp_Mono10:
	case PixelType_Gvsp_Mono10_Packed:
	case PixelType_Gvsp_Mono12:
	case PixelType_Gvsp_Mono12_Packed:
		isMono = true;
		break;
	default:
		isMono = false;
		break;
	}
	if (isMono)
	{
		*getImage = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pDate);
		//imwrite("d:\\����opencv_Mono.tif", image);
	}
	else
	{
		//ת��ͼ���ʽΪBGR8
		MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
		memset(&stConvertParam, 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
		stConvertParam.nWidth = stImageInfo.nWidth;                 //ch:ͼ��� | en:image width
		stConvertParam.nHeight = stImageInfo.nHeight;               //ch:ͼ��� | en:image height
		//stConvertParam.pSrcData = m_pBufForDriver;                  //ch:�������ݻ��� | en:input data buffer
		stConvertParam.pSrcData = pDate;                  //ch:�������ݻ��� | en:input data buffer
		stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;         //ch:�������ݴ�С | en:input data size
		stConvertParam.enSrcPixelType = stImageInfo.enPixelType;    //ch:�������ظ�ʽ | en:input pixel format
		stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed; //ch:������ظ�ʽ | en:output pixel format  ������OPENCV��ͼ���ʽ
		//stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;   //ch:������ظ�ʽ | en:output pixel format
		stConvertParam.pDstBuffer = m_pBufForSaveImage;                    //ch:������ݻ��� | en:output data buffer
		stConvertParam.nDstBufferSize = m_nBufSizeForSaveImage;            //ch:��������С | en:output buffer size
		MV_CC_ConvertPixelType(m_hDevHandle, &stConvertParam);

		*getImage = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, m_pBufForSaveImage);

	}
	(*getImage).copyTo(image);
	(*getImage).release();
	free(pDate);
	free(m_pBufForSaveImage);
	return 0;
}

//��ȡͼ��߶�ֵ
int CMyCamera::getHeight()
{
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	int tempValue = MV_CC_GetIntValue(m_hDevHandle, "Height", &stParam);
	int value = stParam.nCurValue;
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return value;
	}
}

//��ȡͼ����ֵ
int CMyCamera::getWidth()
{
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	int tempValue = MV_CC_GetIntValue(m_hDevHandle, "Width", &stParam);
	int value = stParam.nCurValue;
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return value;
	}
}

//��ȡ����ع�ʱ��
float CMyCamera::getExposureTime()
{
	MVCC_FLOATVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	int tempValue = MV_CC_GetFloatValue(m_hDevHandle, "ExposureTime", &stParam);
	float value = stParam.fCurValue;
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return value;
	}
}

//����ͼ��ROI�߶�
int CMyCamera::setHeight(unsigned int height)
{
	int tempValue = MV_CC_SetIntValue(m_hDevHandle, "Height", height);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//����ͼ��ROI���
int CMyCamera::setWidth(unsigned int width)
{
	int tempValue = MV_CC_SetIntValue(m_hDevHandle, "Width", width);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//����ͼ��ˮƽƫ��OffsetX
int CMyCamera::setOffsetX(unsigned int offsetX)
{
	int tempValue = MV_CC_SetIntValue(m_hDevHandle, "OffsetX", offsetX);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//����ͼ����ֱƫ��OffsetY
int CMyCamera::setOffsetY(unsigned int offsetY)
{
	int tempValue = MV_CC_SetIntValue(m_hDevHandle, "OffsetY", offsetY);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//�����Ƿ�Ϊ����ģʽ
int CMyCamera::setTriggerMode(unsigned int TriggerModeNum)
{
	//0��Off  1��On
	int tempValue = MV_CC_SetEnumValue(m_hDevHandle, "TriggerMode", TriggerModeNum);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//���ô���Դ
int CMyCamera::setTriggerSource(unsigned int TriggerSourceNum)
{
	//0��Line0  1��Line1  7��Software
	int tempValue = MV_CC_SetEnumValue(m_hDevHandle, "TriggerSource", TriggerSourceNum);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//����֡�ʿ���ʹ��
int CMyCamera::setFrameRateEnable(bool comm)
{
	int tempValue = MV_CC_SetBoolValue(m_hDevHandle, "AcquisitionFrameRateEnable", comm);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//��������ʱ��
int CMyCamera::setHeartBeatTime(unsigned int time)
{
	//����ʱ����СΪ500ms
	if (time < 500)
		time = 500;
	int tempValue = MV_CC_SetIntValue(m_hDevHandle, "GevHeartbeatTimeout", time);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//�����ع�ʱ��
int CMyCamera::setExposureTime(float ExposureTimeNum)
{
	int tempValue = MV_CC_SetFloatValue(m_hDevHandle, "ExposureTime", ExposureTimeNum);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}
//�ر��Զ��ع�
int CMyCamera::setExposureAuto(bool exposureAutoFlag)
{
	int tempValue = MV_CC_SetEnumValue(m_hDevHandle, "ExposureAuto", exposureAutoFlag);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//�ر��Զ�����
int CMyCamera::setGainAuto(bool gainAutoFlag)
{
	int tempValue = MV_CC_SetEnumValue(m_hDevHandle, "GainAuto", gainAutoFlag);
	if (tempValue != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//�����������
void CMyCamera::clearBuffer()
{
	//stopCamera();
	//startCamera();
}
