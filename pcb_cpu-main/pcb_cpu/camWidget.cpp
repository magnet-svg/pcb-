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


	/*按键使能初始化*/
	// 相机初始化控件
	ui->bntEnumDevices->setEnabled(true);
	ui->bntCloseDevices->setEnabled(false);
	ui->bntOpenDevices->setEnabled(false);
	// 图像采集控件
	ui->rbnt_Continue_Mode->setEnabled(false);
	ui->rbnt_SoftTigger_Mode->setEnabled(false);
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(false);
	ui->bntSoftwareOnce->setEnabled(false);
	// 保存图像控件
	ui->bntSave_BMP->setEnabled(false);
	ui->bntSave_JPG->setEnabled(false);
	// 参数控件
	ui->lineEdit_SetExposure->setEnabled(false);
	ui->lineEdit_SetGain->setEnabled(false);
	ui->lineEdit_setFrame->setEnabled(false);

	//// 线程对象实例化
    myThread_LeftCamera =  new MyThread;  //左相机线程对象
	myThread_RightCamera = new MyThread; //右相机线程对象
		// 图像指针实例化
	myImage_L = new cv::Mat();                // 图像指针实例化 ，不创建也行？？ 
	myImage_R = new cv::Mat();                // 图像指针实例化 

	// 初始化变量
	int devices_num = 0;
	int m_nTriggerMode = TRIGGER_ON;
	int m_bStartGrabbing = START_GRABBING_ON;
	int m_bContinueStarted = 0;
	MV_SAVE_IAMGE_TYPE m_nSaveImageType = MV_Image_Bmp;

	// 将线程的信号与槽进行绑定
	connect(myThread_LeftCamera, SIGNAL(Display(const Mat* ,int)), this, SLOT(display_myImage_L(const Mat* ,int)));
	connect(myThread_RightCamera, SIGNAL(Display(const Mat*, int)), this, SLOT(display_myImage_R(const Mat* ,int)));
	// 相机初始化
	connect(ui->bntEnumDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedEnumButton()));
	connect(ui->bntOpenDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedOpenButton()));
	connect(ui->bntCloseDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedCloseButton()));
	// 图像采集
	connect(ui->rbnt_Continue_Mode, SIGNAL(clicked()), this, SLOT(OnBnClickedContinusModeRadio()));
	connect(ui->rbnt_SoftTigger_Mode, SIGNAL(clicked()), this, SLOT(OnBnClickedTriggerModeRadio())); // 易出错：缺了() 导致，信号与槽未绑定上
	connect(ui->bntStartGrabbing, SIGNAL(clicked()), this, SLOT(OnBnClickedStartGrabbingButton()));
	connect(ui->bntStopGrabbing, SIGNAL(clicked()), this, SLOT(OnBnClickedStopGrabbingButton()));
	connect(ui->bntSoftwareOnce, SIGNAL(clicked()), this, SLOT(OnBnClickedSoftwareOnceButton()));
	// 参数设置
	connect(ui->lineEdit_SetGain, SIGNAL(editingFinished()), this, SLOT(SetGain()));	            //增益 0 dB ~15 dB
	connect(ui->lineEdit_SetExposure, SIGNAL(editingFinished()), this, SLOT(SetExposureTime()));	//	曝光时间 Bayer 格式 16 μs ~1 sec
	connect(ui->lineEdit_setFrame, SIGNAL(editingFinished()), this, SLOT(SetFrameRate()));          // 最高帧率30pfs
	//
	connect(ui->bntSave_BMP, SIGNAL(clicked()), this, SLOT(OnBnClickedSaveBmpButton()));	//	曝光时间 Bayer 格式 16 μs ~1 sec
	connect(ui->bntSave_JPG, SIGNAL(clicked()), this, SLOT(OnBnClickedSaveJpgButton()));          // 最高帧率30pfs
    // 输入控件数值限制
	ui->lineEdit_SetExposure->setValidator(new QIntValidator(100, 100000, this));
	ui->lineEdit_SetGain->setValidator(new QDoubleValidator(0.0, 15.0,2, this));//精度为小数点两位数
	ui->lineEdit_setFrame->setValidator(new QIntValidator(1, 30, this));

}

CamWidget::~CamWidget()
{
	delete ui;
}

/*************************************************** 定义槽函数 *************************************************** */
// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void CamWidget::OnBnClickedEnumButton()
{
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:初始化设备信息列表  
	int nRet = MV_OK;
	nRet = CMyCamera::EnumDevices(&m_stDevList);                // ch:枚举子网内所有设备,相机设备数量

	devices_num = m_stDevList.nDeviceNum;
	if (devices_num > 0)
	{
		ui->bntOpenDevices->setEnabled(true);		          // 设备数大于0，使能打开设备按键
	}


}


// 打开相机,开启相机
void CamWidget::OpenDevices()
{
	int nRet = MV_OK;
	// 创建相机指针对象
	for (unsigned int i = 0, j = 0; j < m_stDevList.nDeviceNum; j++, i++)
	{
		m_pcMyCamera[i] = new CMyCamera;                       
		// 相机对象初始化
		m_pcMyCamera[i]->m_pBufForDriver = NULL;
		m_pcMyCamera[i]->m_pBufForSaveImage = NULL;
		m_pcMyCamera[i]->m_nBufSizeForDriver = 0;
		m_pcMyCamera[i]->m_nBufSizeForSaveImage = 0;
		m_pcMyCamera[i]->m_nTLayerType = m_stDevList.pDeviceInfo[j]->nTLayerType;

		nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[j]); //打开相机
		//设置触发模式
		m_pcMyCamera[i]->setTriggerMode(TRIGGER_ON);
		//设置触发源为软触发
		m_pcMyCamera[i]->setTriggerSource(TRIGGER_SOURCE);
		//设置曝光时间
		m_pcMyCamera[i]->setExposureTime(EXPOSURE_TIME);
		// 设置帧率
		m_pcMyCamera[i]->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
		m_pcMyCamera[i]->SetFloatValue("AcquisitionFrameRate", FRAME);

	}
}

void CamWidget::OnBnClickedOpenButton()
{
	// 使能 "开始采集" 按键
	//ui->bntStartGrabbing->setEnabled(true);
	ui->bntOpenDevices->setEnabled(false);
	ui->bntCloseDevices->setEnabled(true);
	ui->rbnt_Continue_Mode->setEnabled(true);
	ui->rbnt_SoftTigger_Mode->setEnabled(true);

	ui->rbnt_Continue_Mode->setCheckable(true);
	// 参数据控件
	ui->lineEdit_SetExposure->setEnabled(true);
	ui->lineEdit_SetGain->setEnabled(true);
	ui->lineEdit_setFrame->setEnabled(true);

	OpenDevices();
}


// ch:关闭设备 | en:Close Device
void CamWidget::CloseDevices()
{
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{

		// 关闭线程、相机
		if (myThread_LeftCamera->isRunning())
		{
			myThread_LeftCamera->requestInterruption();
			myThread_LeftCamera->wait();
			m_pcMyCamera[0]->StopGrabbing();
			//myThread_LeftCamera->~MyThread();// 销毁线程

		}

		if (myThread_RightCamera->isRunning())
		{

			myThread_RightCamera->requestInterruption();
			myThread_RightCamera->wait();
			m_pcMyCamera[1]->StopGrabbing();
		}
		m_pcMyCamera[i]->Close();
	}

	// ch:关闭之后再枚举一遍 | en:Enumerate after close
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:初始化设备信息列表  
	int devices_num = MV_OK;
	devices_num = CMyCamera::EnumDevices(&m_stDevList);                // ch:枚举子网内所有设备,相机设备数量

}


// ch:按下关闭设备按钮：关闭设备 ,包含销毁句柄| en:Click Close button: Close Device
void CamWidget::OnBnClickedCloseButton()
{
	//ui->label_Real_fps->setText("close camera doing");
	ui->bntOpenDevices->setEnabled(true);
	ui->bntCloseDevices->setEnabled(false);
	// 图像采集控件
	ui->rbnt_Continue_Mode->setEnabled(false);
	ui->rbnt_SoftTigger_Mode->setEnabled(false);
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(false);
	// 保存图像控件
	ui->bntSave_BMP->setEnabled(false);
	ui->bntSave_JPG->setEnabled(false);

	// 关闭设备，销毁线程
	CloseDevices();
}



// 开始连续采集图像
void CamWidget::OnBnClickedStartGrabbingButton()
{
	m_bContinueStarted = 1; // 为触发模式标记一下，切换触发模式时先执行停止采集图像函数

	// 图像采集控件
	ui->bntStartGrabbing->setEnabled(false);
	ui->bntStopGrabbing->setEnabled(true);
	// 保存图像控件
	ui->bntSave_BMP->setEnabled(true);
	ui->bntSave_JPG->setEnabled(true);

	int camera_Index = 0;

	// 先判断什么模式，再判断是否正在采集
	if (m_nTriggerMode == TRIGGER_ON)
	{
		// ch:开始采集之后才创建workthread线程 | en:Create workthread after start grabbing
		for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
		{
			//开启相机采集
		
				m_pcMyCamera[i]->StartGrabbing();

		

			camera_Index = i;
			if (camera_Index == 0)
			{
				myThread_LeftCamera->getCameraPtr(m_pcMyCamera[0]); //线程获取左相机指针
				myThread_LeftCamera->getImagePtr(myImage_L);        //线程获取左图像指针
				myThread_LeftCamera->getCameraIndex(1);             //右相机 Index==0

				if (!myThread_LeftCamera->isRunning())
				{
					myThread_LeftCamera->start();
					m_pcMyCamera[0]->softTrigger();
					m_pcMyCamera[0]->ReadBuffer(*myImage_L);//读取Mat格式的图像
				}

			}

			if (camera_Index == 1)
			{
				myThread_RightCamera->getCameraPtr(m_pcMyCamera[1]); //线程获取右相机指针
				myThread_RightCamera->getImagePtr(myImage_R);        //线程获取右图像指针
				myThread_RightCamera->getCameraIndex(1);             //右相机 Index==1

				if (!myThread_RightCamera->isRunning())
				{
					myThread_RightCamera->start();
					m_pcMyCamera[1]->softTrigger();
					m_pcMyCamera[1]->ReadBuffer(*myImage_R);//读取Mat格式的图像
				}
			}
		}




	}
}



// ch:按下结束采集按钮 | en:Click Stop button
void CamWidget::OnBnClickedStopGrabbingButton()
{

	ui->bntStartGrabbing->setEnabled(true);
	ui->bntStopGrabbing->setEnabled(false);

	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{

		//关闭相机
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
//	////判断是黑白、彩色图像
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
//	//显示图像
//	ui->label->setPixmap(QPixmap::fromImage(QmyImage));
//
//
}


 

void CamWidget::display_myImage_L(const Mat *imagePrt, int cameraIndex)
{

	//imshow("src", *this->myImage_L);

	cv::Mat rgb;
	cv::cvtColor(*imagePrt, rgb, CV_BGR2RGB);

	////判断是黑白、彩色图像
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
	//显示图像
	ui->label_L->setPixmap(QPixmap::fromImage(QmyImage_L));


}


void CamWidget::display_myImage_R(const Mat *imagePrt, int cameraIndex)
{

	//imshow("src", *this->myImage_L);

	cv::Mat rgb;
	cv::cvtColor(*imagePrt, rgb, CV_BGR2RGB);

	////判断是黑白、彩色图像
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
	//显示图像
	ui->label_R->setPixmap(QPixmap::fromImage(QmyImage_R));


}


// ch:获取曝光时间 | en:Get Exposure Time
int CamWidget::GetExposureTime(void)
{
	int i = 0;
	return i;
}


// ch:设置曝光时间 | en:Set Exposure Time
void CamWidget::SetExposureTime(void)
{
	//设置曝光时间
	QString str = ui->lineEdit_SetExposure->text(); // 读取
	int exposure_Time = str.toInt();

	for (int i = 0; i < devices_num; i++)
	{
		m_pcMyCamera[i]->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
		m_pcMyCamera[i]->SetFloatValue("ExposureTime", exposure_Time);
	}
}


// ch:获取增益 | en:Get Gain
int CamWidget::GetGain(void)
{
	int i = 0;
	return i;
}

// ch:设置增益 | en:Set Gain
void CamWidget::SetGain(void)
{
	QString str = ui->lineEdit_SetGain->text(); // 读取
	float gain = str.toFloat();

	for (int i = 0; i < devices_num; i++)
	{
		m_pcMyCamera[i]->SetEnumValue("GainAuto", 0);
		int nRet = m_pcMyCamera[i]->SetFloatValue("Gain", gain);
	}
}


// ch:获取帧率 | en:Get Frame Rate
int CamWidget::GetFrameRate(void)
{
	int i = 0;
	return i;
}

// ch:设置帧率 | en:Set Frame Rate
void CamWidget::SetFrameRate(void)
{
	QString str = ui->lineEdit_setFrame->text(); // 读取
	float frame = str.toInt();

	for (int i = 0; i < devices_num; i++)
	{
		int nRet = m_pcMyCamera[i]->SetBoolValue("AcquisitionFrameRateEnable", true);
		m_pcMyCamera[i]->SetFloatValue("AcquisitionFrameRate", frame);
		//ui->label_Real_fps->setText(str);
	}
}


// ch:获取触发模式 | en:Get Trigger Mode
int CamWidget::GetTriggerMode(void)
{
	int i = 0;
	return 0;
}

// ch:设置触发模式 | en:Set Trigger Mode
void CamWidget::SetTriggerMode(int m_nTriggerMode)
{

}



// ch:按下连续模式按钮 | en:Click Continues button
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

// ch:按下触发模式按钮 | en:Click Trigger Mode button
void CamWidget::OnBnClickedTriggerModeRadio()
{
	if (m_bContinueStarted == 1) // 从连续采集模式已经正在采集的状态切换过来
	{
		OnBnClickedStopGrabbingButton();//先执行停止采集
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


// ch:按下软触发一次按钮 | en:Click Execute button
void CamWidget::OnBnClickedSoftwareOnceButton()
{
	// 保存图像控件
	ui->bntSave_BMP->setEnabled(true);
	ui->bntSave_JPG->setEnabled(true);

	if(m_nTriggerMode == TRIGGER_OFF)
	{
		int nRet = MV_OK;
		for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
		{
			//开启相机采集
			m_pcMyCamera[i]->StartGrabbing();

			if ( i == 0 )
			{		
				nRet = m_pcMyCamera[i]->CommandExecute("TriggerSoftware");
				m_pcMyCamera[i]->ReadBuffer(*myImage_L);
				display_myImage_L(myImage_L, i);//左相机图像
				//m_pcMyCamera[i]->StopGrabbing();
			}
			if (i == 1)
			{			//开启相机采集
				nRet = m_pcMyCamera[i]->CommandExecute("TriggerSoftware");
				m_pcMyCamera[i]->ReadBuffer(*myImage_R);
				display_myImage_R(myImage_R, i);
				//m_pcMyCamera[i]->StopGrabbing();
			}
		}
	}
}

// ch:按下保存bmp图片按钮 | en:Click Save BMP button
void CamWidget::OnBnClickedSaveBmpButton()
{
	m_nSaveImageType = MV_Image_Bmp;
	SaveImage();

}

// ch:按下保存jpg图片按钮 | en:Click Save JPG button
void CamWidget::OnBnClickedSaveJpgButton()
{
    m_nSaveImageType = MV_Image_Jpeg;
	SaveImage();
}

// ch:保存图片 | en:Save Image
void CamWidget::SaveImage()
{

	// ch:获取1张图 | en:Get one frame
	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
	unsigned int nDataLen = 0;
	int nRet = MV_OK;
	for (int i = 0; i < devices_num; i++)
	{
		// ch:仅在第一次保存图像时申请缓存，在 CloseDevice 时释放
		// en:Request buffer first time save image, release after CloseDevice
		if (NULL == m_pcMyCamera[i]->m_pBufForDriver)
		{
			unsigned int nRecvBufSize = 0;
			unsigned int nRet = m_pcMyCamera[i]->GetIntValue("PayloadSize", &nRecvBufSize);


			m_pcMyCamera[i]->m_nBufSizeForDriver = nRecvBufSize;  // 一帧数据大小

			m_pcMyCamera[i]->m_pBufForDriver = (unsigned char *)malloc(m_pcMyCamera[i]->m_nBufSizeForDriver);

		}

		nRet = m_pcMyCamera[i]->GetOneFrameTimeout(m_pcMyCamera[i]->m_pBufForDriver, &nDataLen, m_pcMyCamera[i]->m_nBufSizeForDriver, &stImageInfo, 1000);
		if (MV_OK == nRet)
		{
			// ch:仅在第一次保存图像时申请缓存，在 CloseDevice 时释放
			// en:Request buffer first time save image, release after CloseDevice
			if (NULL == m_pcMyCamera[i]->m_pBufForSaveImage)
			{
				// ch:BMP图片大小：width * height * 3 + 2048(预留BMP头大小)
				// en:BMP image size: width * height * 3 + 2048 (Reserved BMP header size)
				m_pcMyCamera[i]->m_nBufSizeForSaveImage = stImageInfo.nWidth * stImageInfo.nHeight * 3 + 2048;

				m_pcMyCamera[i]->m_pBufForSaveImage = (unsigned char*)malloc(m_pcMyCamera[i]->m_nBufSizeForSaveImage);

			}
			// ch:设置对应的相机参数 | en:Set camera parameter
			MV_SAVE_IMAGE_PARAM_EX stParam = { 0 };
		    stParam.enImageType = m_nSaveImageType; // ch:需要保存的图像类型 | en:Image format to save;
			stParam.enPixelType = stImageInfo.enPixelType;  // 相机对应的像素格式 | en:Pixel format
			stParam.nBufferSize = m_pcMyCamera[i]->m_nBufSizeForSaveImage;  // 存储节点的大小 | en:Buffer node size
			stParam.nWidth = stImageInfo.nWidth;         // 相机对应的宽 | en:Width
			stParam.nHeight = stImageInfo.nHeight;          // 相机对应的高 | en:Height
			stParam.nDataLen = stImageInfo.nFrameLen;
			stParam.pData = m_pcMyCamera[i]->m_pBufForDriver;
			stParam.pImageBuffer = m_pcMyCamera[i]->m_pBufForSaveImage;
			stParam.nJpgQuality = 90;       // ch:jpg编码，仅在保存Jpg图像时有效。保存BMP时SDK内忽略该参数

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
//// ch:获取触发源 | en:Get Trigger Source
//int CamWidget::GetTriggerSource(void)
//{
//	int i = 0;
//	return i;
//}
//
//// ch:设置触发源 | en:Set Trigger Source
//void CamWidget::SetTriggerSource(void)
//{
//
//}
//
//
//
//
//
//// ch:按下获取参数按钮 | en:Click Get Parameter button
//void CamWidget::OnBnClickedGetParameterButton()
//{
//
//}
