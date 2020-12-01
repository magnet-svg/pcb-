#include "camWidget.h"
#include <QWidget>
#include <QValidator>

#define TRIGGER_SOURCE  7
#define EXPOSURE_TIME   40000
#define FRAME           30
#define TRIGGER_ON      1
#define TRIGGER_OFF     0
#define START_GRABBING_ON   1
#define START_GRABBING_OFF  0
#define IMAGE_NAME_LEN          64


CamWidget::CamWidget(QWidget *parent):QMainWindow(parent),
	ui(new Ui::CamWidget)
{
	ui->setupUi(this);

	//CalibrateView_ui = new CalibrateView_ui;


	/*����ʹ�ܳ�ʼ��*/
	// �����ʼ���ؼ�
	ui->bntEnumDevices->setEnabled(true);
	ui->bntCloseDevices->setEnabled(false);
	ui->bntOpenDevices->setEnabled(false);
	// ͼ��ɼ��ؼ�
	ui->rbnt_Continue_Mode->setEnabled(false);
	ui->rbnt_SoftTigger_Mode->setEnabled(false);
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(false);
	ui->bntSoftwareOnce->setEnabled(false);
	// ����ͼ��ؼ�
	ui->bntSave_BMP->setEnabled(false);
	ui->bntSave_JPG->setEnabled(false);
	// �����ؼ�
	ui->lineEdit_SetExposure->setEnabled(false);
	ui->lineEdit_SetGain->setEnabled(false);
	ui->lineEdit_setFrame->setEnabled(false);

	//// �̶߳���ʵ����
    myThread_LeftCamera =  new MyThread;  //������̶߳���
	myThread_RightCamera = new MyThread; //������̶߳���
		// ͼ��ָ��ʵ����
	myImage_L = new cv::Mat();                // ͼ��ָ��ʵ���� ��������Ҳ�У��� 
	myImage_R = new cv::Mat();                // ͼ��ָ��ʵ���� 

	// ��ʼ������
	int devices_num = 0;
	int m_nTriggerMode = TRIGGER_ON;
	int m_bStartGrabbing = START_GRABBING_ON;
	int m_bContinueStarted = 0;
	MV_SAVE_IAMGE_TYPE m_nSaveImageType = MV_Image_Bmp;

	// ���̵߳��ź���۽��а�
	connect(myThread_LeftCamera, SIGNAL(Display(const Mat* ,int)), this, SLOT(display_myImage_L(const Mat* ,int)));
	connect(myThread_RightCamera, SIGNAL(Display(const Mat*, int)), this, SLOT(display_myImage_R(const Mat* ,int)));
	// �����ʼ��
	connect(ui->bntEnumDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedEnumButton()));
	connect(ui->bntOpenDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedOpenButton()));
	connect(ui->bntCloseDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedCloseButton()));
	// ͼ��ɼ�
	connect(ui->rbnt_Continue_Mode, SIGNAL(clicked()), this, SLOT(OnBnClickedContinusModeRadio()));
	connect(ui->rbnt_SoftTigger_Mode, SIGNAL(clicked()), this, SLOT(OnBnClickedTriggerModeRadio())); // �׳���ȱ��() ���£��ź����δ����
	connect(ui->bntStartGrabbing, SIGNAL(clicked()), this, SLOT(OnBnClickedStartGrabbingButton()));
	connect(ui->bntStopGrabbing, SIGNAL(clicked()), this, SLOT(OnBnClickedStopGrabbingButton()));
	connect(ui->bntSoftwareOnce, SIGNAL(clicked()), this, SLOT(OnBnClickedSoftwareOnceButton()));
	// ��������
	connect(ui->lineEdit_SetGain, SIGNAL(editingFinished()), this, SLOT(SetGain()));	            //���� 0 dB ~15 dB
	connect(ui->lineEdit_SetExposure, SIGNAL(editingFinished()), this, SLOT(SetExposureTime()));	//	�ع�ʱ�� Bayer ��ʽ 16 ��s ~1 sec
	connect(ui->lineEdit_setFrame, SIGNAL(editingFinished()), this, SLOT(SetFrameRate()));          // ���֡��30pfs
	//
	connect(ui->bntSave_BMP, SIGNAL(clicked()), this, SLOT(OnBnClickedSaveBmpButton()));	//	�ع�ʱ�� Bayer ��ʽ 16 ��s ~1 sec
	connect(ui->bntSave_JPG, SIGNAL(clicked()), this, SLOT(OnBnClickedSaveJpgButton()));          // ���֡��30pfs
    // ����ؼ���ֵ����
	ui->lineEdit_SetExposure->setValidator(new QIntValidator(100, 100000, this));
	ui->lineEdit_SetGain->setValidator(new QDoubleValidator(0.0, 15.0,2, this));//����ΪС������λ��
	ui->lineEdit_setFrame->setValidator(new QIntValidator(1, 30, this));

}

CamWidget::~CamWidget()
{
	delete ui;
}

/*************************************************** ����ۺ��� *************************************************** */
// ch:���²����豸��ť:ö�� | en:Click Find Device button:Enumeration 
void CamWidget::OnBnClickedEnumButton()
{
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:��ʼ���豸��Ϣ�б�  
	int nRet = MV_OK;
	nRet = CMyCamera::EnumDevices(&m_stDevList);                // ch:ö�������������豸,����豸����

	devices_num = m_stDevList.nDeviceNum;
	if (devices_num > 0)
	{
		ui->bntOpenDevices->setEnabled(true);		          // �豸������0��ʹ�ܴ��豸����
	}


}


// �����,�������
void CamWidget::OpenDevices()
{
	int nRet = MV_OK;
	// �������ָ�����
	for (unsigned int i = 0, j = 0; j < m_stDevList.nDeviceNum; j++, i++)
	{
		m_pcMyCamera[i] = new CMyCamera;                       
		// ��������ʼ��
		m_pcMyCamera[i]->m_pBufForDriver = NULL;
		m_pcMyCamera[i]->m_pBufForSaveImage = NULL;
		m_pcMyCamera[i]->m_nBufSizeForDriver = 0;
		m_pcMyCamera[i]->m_nBufSizeForSaveImage = 0;
		m_pcMyCamera[i]->m_nTLayerType = m_stDevList.pDeviceInfo[j]->nTLayerType;

		nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[j]); //�����
		//���ô���ģʽ
		m_pcMyCamera[i]->setTriggerMode(TRIGGER_ON);
		//���ô���ԴΪ����
		m_pcMyCamera[i]->setTriggerSource(TRIGGER_SOURCE);
		//�����ع�ʱ��
		m_pcMyCamera[i]->setExposureTime(EXPOSURE_TIME);
		// ����֡��
		m_pcMyCamera[i]->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
		m_pcMyCamera[i]->SetFloatValue("AcquisitionFrameRate", FRAME);

	}
}

void CamWidget::OnBnClickedOpenButton()
{
	// ʹ�� "��ʼ�ɼ�" ����
	//ui->bntStartGrabbing->setEnabled(true);
	ui->bntOpenDevices->setEnabled(false);
	ui->bntCloseDevices->setEnabled(true);
	ui->rbnt_Continue_Mode->setEnabled(true);
	ui->rbnt_SoftTigger_Mode->setEnabled(true);

	ui->rbnt_Continue_Mode->setCheckable(true);
	// �����ݿؼ�
	ui->lineEdit_SetExposure->setEnabled(true);
	ui->lineEdit_SetGain->setEnabled(true);
	ui->lineEdit_setFrame->setEnabled(true);

	OpenDevices();
}


// ch:�ر��豸 | en:Close Device
void CamWidget::CloseDevices()
{
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{

		// �ر��̡߳����
		if (myThread_LeftCamera->isRunning())
		{
			myThread_LeftCamera->requestInterruption();
			myThread_LeftCamera->wait();
			m_pcMyCamera[0]->StopGrabbing();
			//myThread_LeftCamera->~MyThread();// �����߳�

		}

		if (myThread_RightCamera->isRunning())
		{

			myThread_RightCamera->requestInterruption();
			myThread_RightCamera->wait();
			m_pcMyCamera[1]->StopGrabbing();
		}
		m_pcMyCamera[i]->Close();
	}

	// ch:�ر�֮����ö��һ�� | en:Enumerate after close
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:��ʼ���豸��Ϣ�б�  
	int devices_num = MV_OK;
	devices_num = CMyCamera::EnumDevices(&m_stDevList);                // ch:ö�������������豸,����豸����

}


// ch:���¹ر��豸��ť���ر��豸 ,�������پ��| en:Click Close button: Close Device
void CamWidget::OnBnClickedCloseButton()
{
	//ui->label_Real_fps->setText("close camera doing");
	ui->bntOpenDevices->setEnabled(true);
	ui->bntCloseDevices->setEnabled(false);
	// ͼ��ɼ��ؼ�
	ui->rbnt_Continue_Mode->setEnabled(false);
	ui->rbnt_SoftTigger_Mode->setEnabled(false);
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(false);
	// ����ͼ��ؼ�
	ui->bntSave_BMP->setEnabled(false);
	ui->bntSave_JPG->setEnabled(false);

	// �ر��豸�������߳�
	CloseDevices();
}



// ��ʼ�����ɼ�ͼ��
void CamWidget::OnBnClickedStartGrabbingButton()
{
	m_bContinueStarted = 1; // Ϊ����ģʽ���һ�£��л�����ģʽʱ��ִ��ֹͣ�ɼ�ͼ����

	// ͼ��ɼ��ؼ�
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(true);
	// ����ͼ��ؼ�
	ui->bntSave_BMP->setEnabled(true);
	ui->bntSave_JPG->setEnabled(true);

	int camera_Index = 0;

	// ���ж�ʲôģʽ�����ж��Ƿ����ڲɼ�
	if (m_nTriggerMode == TRIGGER_ON)
	{
		// ch:��ʼ�ɼ�֮��Ŵ���workthread�߳� | en:Create workthread after start grabbing
		for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
		{
			//��������ɼ�
		
				m_pcMyCamera[i]->StartGrabbing();

		

			camera_Index = i;
			if (camera_Index == 0)
			{
				myThread_LeftCamera->getCameraPtr(m_pcMyCamera[0]); //�̻߳�ȡ�����ָ��
				myThread_LeftCamera->getImagePtr(myImage_L);        //�̻߳�ȡ��ͼ��ָ��
				myThread_LeftCamera->getCameraIndex(1);             //����� Index==0

				if (!myThread_LeftCamera->isRunning())
				{
					myThread_LeftCamera->start();
					m_pcMyCamera[0]->softTrigger();
					m_pcMyCamera[0]->ReadBuffer(*myImage_L);//��ȡMat��ʽ��ͼ��
				}

			}

			if (camera_Index == 1)
			{
				myThread_RightCamera->getCameraPtr(m_pcMyCamera[1]); //�̻߳�ȡ�����ָ��
				myThread_RightCamera->getImagePtr(myImage_R);        //�̻߳�ȡ��ͼ��ָ��
				myThread_RightCamera->getCameraIndex(1);             //����� Index==1

				if (!myThread_RightCamera->isRunning())
				{
					myThread_RightCamera->start();
					m_pcMyCamera[1]->softTrigger();
					m_pcMyCamera[1]->ReadBuffer(*myImage_R);//��ȡMat��ʽ��ͼ��
				}
			}
		}




	}
}



// ch:���½����ɼ���ť | en:Click Stop button
void CamWidget::OnBnClickedStopGrabbingButton()
{

	ui->bntStartGrabbing->setEnabled(true);
	ui->bntStopGrabbing->setEnabled(false);

	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{

		//�ر����
		if (myThread_LeftCamera->isRunning())
		{
			m_pcMyCamera[0]->StopGrabbing();
			myThread_LeftCamera->requestInterruption();
			myThread_LeftCamera->wait();

		}
		if (myThread_RightCamera->isRunning())
		{
			m_pcMyCamera[1]->StopGrabbing();
			myThread_RightCamera->requestInterruption();
			myThread_RightCamera->wait();

		}
	}
}


 void CamWidget::Img_display()
{
//
//	//imshow("src", *this->myImage_L);
//
//	cv::Mat rgb;
//	cv::cvtColor(*myImage_L, rgb, CV_BGR2RGB);
//
//	////�ж��Ǻڰס���ɫͼ��
//	QImage QmyImage;
//	if (myImage_L->channels() > 1)
//	{
//		QmyImage = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888);
//	}
//	else
//	{
//		QmyImage = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_Indexed8);
//	}
//
//	QmyImage = (QmyImage).scaled(ui->label->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//
//	//��ʾͼ��
//	ui->label->setPixmap(QPixmap::fromImage(QmyImage));
//
//
}


 

void CamWidget::display_myImage_L(const Mat *imagePrt, int cameraIndex)
{

	//imshow("src", *this->myImage_L);

	cv::Mat rgb;
	cv::cvtColor(*imagePrt, rgb, CV_BGR2RGB);

	////�ж��Ǻڰס���ɫͼ��
	QImage QmyImage_L;
	if (myImage_L->channels() > 1)
	{
		QmyImage_L = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888);
	}
	else
	{
		QmyImage_L = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_Indexed8);
	}

	QmyImage_L = (QmyImage_L).scaled(ui->label_L->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	//��ʾͼ��
	ui->label_L->setPixmap(QPixmap::fromImage(QmyImage_L));


}


void CamWidget::display_myImage_R(const Mat *imagePrt, int cameraIndex)
{

	//imshow("src", *this->myImage_L);

	cv::Mat rgb;
	cv::cvtColor(*imagePrt, rgb, CV_BGR2RGB);

	////�ж��Ǻڰס���ɫͼ��
	QImage QmyImage_R;
	if (myImage_R->channels() > 1)
	{
		QmyImage_R = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888);
	}
	else
	{
		QmyImage_R = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_Indexed8);
	}


	QmyImage_R = (QmyImage_R).scaled(ui->label_R->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	//��ʾͼ��
	ui->label_R->setPixmap(QPixmap::fromImage(QmyImage_R));


}


// ch:��ȡ�ع�ʱ�� | en:Get Exposure Time
int CamWidget::GetExposureTime(void)
{
	int i = 0;
	return i;
}


// ch:�����ع�ʱ�� | en:Set Exposure Time
void CamWidget::SetExposureTime(void)
{
	//�����ع�ʱ��
	QString str = ui->lineEdit_SetExposure->text(); // ��ȡ
	int exposure_Time = str.toInt();

	for (int i = 0; i < devices_num; i++)
	{
		m_pcMyCamera[i]->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
		m_pcMyCamera[i]->SetFloatValue("ExposureTime", exposure_Time);
	}
}


// ch:��ȡ���� | en:Get Gain
int CamWidget::GetGain(void)
{
	int i = 0;
	return i;
}

// ch:�������� | en:Set Gain
void CamWidget::SetGain(void)
{
	QString str = ui->lineEdit_SetGain->text(); // ��ȡ
	float gain = str.toFloat();

	for (int i = 0; i < devices_num; i++)
	{
		m_pcMyCamera[i]->SetEnumValue("GainAuto", 0);
		int nRet = m_pcMyCamera[i]->SetFloatValue("Gain", gain);
	}
}


// ch:��ȡ֡�� | en:Get Frame Rate
int CamWidget::GetFrameRate(void)
{
	int i = 0;
	return i;
}

// ch:����֡�� | en:Set Frame Rate
void CamWidget::SetFrameRate(void)
{
	QString str = ui->lineEdit_setFrame->text(); // ��ȡ
	float frame = str.toInt();

	for (int i = 0; i < devices_num; i++)
	{
		int nRet = m_pcMyCamera[i]->SetBoolValue("AcquisitionFrameRateEnable", true);
		m_pcMyCamera[i]->SetFloatValue("AcquisitionFrameRate", frame);
		//ui->label_Real_fps->setText(str);
	}
}


// ch:��ȡ����ģʽ | en:Get Trigger Mode
int CamWidget::GetTriggerMode(void)
{
	int i = 0;
	return 0;
}

// ch:���ô���ģʽ | en:Set Trigger Mode
void CamWidget::SetTriggerMode(int m_nTriggerMode)
{

}



// ch:��������ģʽ��ť | en:Click Continues button
void CamWidget::OnBnClickedContinusModeRadio()
{
	ui->bntStartGrabbing->setEnabled(true);

	m_nTriggerMode = TRIGGER_ON;
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		m_pcMyCamera[i]->setTriggerMode(m_nTriggerMode);
		ui->label_Real_fps->setText("111");
	}

	//ui->rbnt_SoftTigger_Mode->setChecked(false);
}

// ch:���´���ģʽ��ť | en:Click Trigger Mode button
void CamWidget::OnBnClickedTriggerModeRadio()
{
	if (m_bContinueStarted == 1) // �������ɼ�ģʽ�Ѿ����ڲɼ���״̬�л�����
	{
		OnBnClickedStopGrabbingButton();//��ִ��ֹͣ�ɼ�
	}

	ui->bntStartGrabbing->setEnabled(false);
	ui->bntSoftwareOnce->setEnabled(true);

	m_nTriggerMode = TRIGGER_OFF;
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		m_pcMyCamera[i]->setTriggerMode(m_nTriggerMode);
		ui->label_Real_fps->setText("000");
	}
}


// ch:��������һ�ΰ�ť | en:Click Execute button
void CamWidget::OnBnClickedSoftwareOnceButton()
{
	// ����ͼ��ؼ�
	ui->bntSave_BMP->setEnabled(true);
	ui->bntSave_JPG->setEnabled(true);

	if(m_nTriggerMode == TRIGGER_OFF)
	{
		int nRet = MV_OK;
		for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
		{
			//��������ɼ�
			m_pcMyCamera[i]->StartGrabbing();

			if ( i == 0 )
			{		
				nRet = m_pcMyCamera[i]->CommandExecute("TriggerSoftware");
				m_pcMyCamera[i]->ReadBuffer(*myImage_L);
				display_myImage_L(myImage_L, i);//�����ͼ��
				//m_pcMyCamera[i]->StopGrabbing();
			}
			if (i == 1)
			{			//��������ɼ�
				nRet = m_pcMyCamera[i]->CommandExecute("TriggerSoftware");
				m_pcMyCamera[i]->ReadBuffer(*myImage_R);
				display_myImage_R(myImage_R, i);
				//m_pcMyCamera[i]->StopGrabbing();
			}
		}
	}
}

// ch:���±���bmpͼƬ��ť | en:Click Save BMP button
void CamWidget::OnBnClickedSaveBmpButton()
{
	m_nSaveImageType = MV_Image_Bmp;
	SaveImage();

}

// ch:���±���jpgͼƬ��ť | en:Click Save JPG button
void CamWidget::OnBnClickedSaveJpgButton()
{
    m_nSaveImageType = MV_Image_Jpeg;
	SaveImage();
}

// ch:����ͼƬ | en:Save Image
void CamWidget::SaveImage()
{

	// ch:��ȡ1��ͼ | en:Get one frame
	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
	unsigned int nDataLen = 0;
	int nRet = MV_OK;
	for (int i = 0; i < devices_num; i++)
	{
		// ch:���ڵ�һ�α���ͼ��ʱ���뻺�棬�� CloseDevice ʱ�ͷ�
		// en:Request buffer first time save image, release after CloseDevice
		if (NULL == m_pcMyCamera[i]->m_pBufForDriver)
		{
			unsigned int nRecvBufSize = 0;
			unsigned int nRet = m_pcMyCamera[i]->GetIntValue("PayloadSize", &nRecvBufSize);


			m_pcMyCamera[i]->m_nBufSizeForDriver = nRecvBufSize;  // һ֡���ݴ�С

			m_pcMyCamera[i]->m_pBufForDriver = (unsigned char *)malloc(m_pcMyCamera[i]->m_nBufSizeForDriver);

		}

		nRet = m_pcMyCamera[i]->GetOneFrameTimeout(m_pcMyCamera[i]->m_pBufForDriver, &nDataLen, m_pcMyCamera[i]->m_nBufSizeForDriver, &stImageInfo, 1000);
		if (MV_OK == nRet)
		{
			// ch:���ڵ�һ�α���ͼ��ʱ���뻺�棬�� CloseDevice ʱ�ͷ�
			// en:Request buffer first time save image, release after CloseDevice
			if (NULL == m_pcMyCamera[i]->m_pBufForSaveImage)
			{
				// ch:BMPͼƬ��С��width * height * 3 + 2048(Ԥ��BMPͷ��С)
				// en:BMP image size: width * height * 3 + 2048 (Reserved BMP header size)
				m_pcMyCamera[i]->m_nBufSizeForSaveImage = stImageInfo.nWidth * stImageInfo.nHeight * 3 + 2048;

				m_pcMyCamera[i]->m_pBufForSaveImage = (unsigned char*)malloc(m_pcMyCamera[i]->m_nBufSizeForSaveImage);

			}
			// ch:���ö�Ӧ��������� | en:Set camera parameter
			MV_SAVE_IMAGE_PARAM_EX stParam = { 0 };
		    stParam.enImageType = m_nSaveImageType; // ch:��Ҫ�����ͼ������ | en:Image format to save;
			stParam.enPixelType = stImageInfo.enPixelType;  // �����Ӧ�����ظ�ʽ | en:Pixel format
			stParam.nBufferSize = m_pcMyCamera[i]->m_nBufSizeForSaveImage;  // �洢�ڵ�Ĵ�С | en:Buffer node size
			stParam.nWidth = stImageInfo.nWidth;         // �����Ӧ�Ŀ� | en:Width
			stParam.nHeight = stImageInfo.nHeight;          // �����Ӧ�ĸ� | en:Height
			stParam.nDataLen = stImageInfo.nFrameLen;
			stParam.pData = m_pcMyCamera[i]->m_pBufForDriver;
			stParam.pImageBuffer = m_pcMyCamera[i]->m_pBufForSaveImage;
			stParam.nJpgQuality = 90;       // ch:jpg���룬���ڱ���Jpgͼ��ʱ��Ч������BMPʱSDK�ں��Ըò���

			nRet = m_pcMyCamera[i]->SaveImage(&stParam);

			char chImageName[IMAGE_NAME_LEN] = { 0 };
			if (MV_Image_Bmp == stParam.enImageType)
			{
				if (i == 0)
				{
					/*sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d_L.bmp", stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);*/
					sprintf_s(chImageName, IMAGE_NAME_LEN, "%03d_L.bmp", stImageInfo.nFrameNum);
				}
				if (i == 1)
				{
					//sprintf_s(chImageName, IMAGE_NAME_LEN, "%03d_R.bmp", stImageInfo.nFrameNum);
					sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d_R.bmp", stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
				}

			}
			else if (MV_Image_Jpeg == stParam.enImageType)
			{
				if (i == 0)
				{
					sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d_L.bmp", stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
				}
				if (i == 1)
				{
					sprintf_s(chImageName, IMAGE_NAME_LEN, "Image_w%d_h%d_fn%03d_R.bmp", stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
				}
			}

			FILE* fp = fopen(chImageName, "wb");

			fwrite(m_pcMyCamera[i]->m_pBufForSaveImage, 1, stParam.nImageLen, fp);
			ui->label_debug->setText("save imgs");
			fclose(fp);
		}
	}
}


/* ***************************************************************************************************************************** */
//// ch:��ȡ����Դ | en:Get Trigger Source
//int CamWidget::GetTriggerSource(void)
//{
//	int i = 0;
//	return i;
//}
//
//// ch:���ô���Դ | en:Set Trigger Source
//void CamWidget::SetTriggerSource(void)
//{
//
//}
//
//
//
//
//
//// ch:���»�ȡ������ť | en:Click Get Parameter button
//void CamWidget::OnBnClickedGetParameterButton()
//{
//
//}
