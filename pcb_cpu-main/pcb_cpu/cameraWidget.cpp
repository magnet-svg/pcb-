//修改时间：2020/8/13/21：00
#include "cameraWidget.h"
#include"string.h"


Camera::Camera(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::Camera)
{
	ui->setupUi(this);

	//按键使能初始化
	ui->bntConnectCamrea->setEnabled(false);
	ui->bntCloseCamera->setEnabled(false);
	ui->bntSingleTigger->setEnabled(false);
	ui->bntContinueTigger->setEnabled(false);

	// 线程对象实例化
	myThread = new MyThread();
	int devices_num = 0;
	// 将线程的信号与槽进行绑定
	connect(myThread, SIGNAL(mess()), this, SLOT(Left_Img_display()));
	connect(ui->bntEnumDevices, SIGNAL(clicked()), this, SLOT(OnBnClickedEnumButton()));
	connect(ui->bntConnectCamrea, SIGNAL(clicked()), this, SLOT(bntOpen_clicked()));
	connect(ui->bntContinueTigger, SIGNAL(clicked()), this, SLOT(bntContinue_Tigger_clicked()));
	connect(ui->bntCloseCamera, SIGNAL(clicked()), this, SLOT(bntClose_clicked()));
	connect(ui->bntSingleTigger, SIGNAL(clicked()), this, SLOT(bntSingle_Tigger_clicked()));

}

Camera::~Camera()
{
	delete ui;
}

/*************************************************** 定义槽函数 *************************************************** */

// ch:按下查找设备按钮:枚举 | en:Click Find Device button:Enumeration 
void Camera::OnBnClickedEnumButton()
{
	
	ui->comboBox_Devices->clear();                              //清除设备列表框中的信息  
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:初始化设备信息列表  

	int nRet = MV_OK;
	nRet = CMyCamera::EnumDevices(&m_stDevList);                // ch:枚举子网内所有设备,相机设备数量
	
	devices_num = m_stDevList.nDeviceNum;

	if (devices_num > 0)
	{
		// 设备数大于0，使能按键
		ui->bntConnectCamrea->setEnabled(true);
	}

	for (unsigned int i = 0, j = 0; j < m_stDevList.nDeviceNum; j++, i++)
	{
		m_pcMyCamera[i] = new CMyCamera;                        // 创建相机指针对象
		// 相机对象初始化
		m_pcMyCamera[i]->m_pBufForDriver = NULL;
		m_pcMyCamera[i]->m_pBufForSaveImage = NULL;
		m_pcMyCamera[i]->m_nBufSizeForDriver = 0;
		m_pcMyCamera[i]->m_nBufSizeForSaveImage = 0;
		m_pcMyCamera[i]->m_nTLayerType = m_stDevList.pDeviceInfo[j]->nTLayerType;

		nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[j]); //打开相机
	}



	ui->bntCloseCamera->setEnabled(true);
	ui->bntSingleTigger->setEnabled(true);
	ui->bntContinueTigger->setEnabled(true);


	if (devices_num > 0)
	{
		ui->Image_Label_R->setText("do");
		//设置为触发模式
		m_pcMyCamera[0]->setTriggerMode(1);
		//设置触发源为软触发
		m_pcMyCamera[0]->setTriggerSource(7);
		//设置曝光时间
		m_pcMyCamera[0]->setExposureTime(30000);
		//开启相机采集
		m_pcMyCamera[0]->StartGrabbing();

		myThread->getCameraPtr(m_pcMyCamera[0]); //线程获取相机指针
		myThread->getImagePtr(images[0]);        //线程获取图像指针
	}
}


// 连接相机
void Camera::bntOpen_clicked() 
{
	///*
	//使能按键
	//*/
	//ui->bntCloseCamera->setEnabled(true);
	//ui->bntSingleTigger->setEnabled(true);
	//ui->bntContinueTigger->setEnabled(true);


	//if (devices_num > 0)
	//{
	//	ui->Image_Label_R->setText("do");
	//	//设置为触发模式
	//	m_pcMyCamera[0]->setTriggerMode(1);
	//	//设置触发源为软触发
	//	m_pcMyCamera[0]->setTriggerSource(7);
	//	//设置曝光时间
	//	m_pcMyCamera[0]->setExposureTime(30000);
	//	//开启相机采集
	//	m_pcMyCamera[0]->StartGrabbing();

	//	myThread->getCameraPtr(m_pcMyCamera[0]); //线程获取相机指针
	//	myThread->getImagePtr(images[0]);        //线程获取图像指针
	//}

}





//采集单张图像
void Camera::bntSingle_Tigger_clicked()
{

}

//连续采集图像
void Camera::bntContinue_Tigger_clicked()
{
	
	if (!myThread->isRunning())
	{
		myThread->start();
 
	}
 
	//m_pcMyCamera[0]->softTrigger();
	//m_pcMyCamera[0]->ReadBuffer(*images[0]);//读取Mat格式的图像
}


void Camera::Left_Img_display()
{

	cv::imshow("src", *this->images[0]);
	cv::Mat rgb;
	cv::cvtColor(*images[0], rgb, CV_BGR2RGB);

	////判断是黑白、彩色图像
	QImage QmyImage;
	if (images[0]->channels() > 1)
	{
		QmyImage = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_RGB888);
	}
	else
	{
		QmyImage = QImage((const unsigned char*)(rgb.data), rgb.cols, rgb.rows, QImage::Format_Indexed8);
	}

	QmyImage = (QmyImage).scaled(ui->Image_Label_L->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	//显示图像
	ui->Image_Label_L->setPixmap(QPixmap::fromImage(QmyImage));


}


void Camera::bntClose_clicked()//停止采集
{
	/*
	按键禁用
	*/
	ui->bntSingleTigger->setEnabled(false);
	ui->bntContinueTigger->setEnabled(false);


	if (myThread->isRunning())
	{
		myThread->requestInterruption();
		myThread->wait();
		//camera->stopCamera();
		//camera->closeCamera();
	}
}
