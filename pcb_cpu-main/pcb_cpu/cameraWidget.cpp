//�޸�ʱ�䣺2020/8/13/21��00
#include "cameraWidget.h"
#include"string.h"


Camera::Camera(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::Camera)
{
	ui->setupUi(this);

	//����ʹ�ܳ�ʼ��
	ui->bntConnectCamrea->setEnabled(false);
	ui->bntCloseCamera->setEnabled(false);
	ui->bntSingleTigger->setEnabled(false);
	ui->bntContinueTigger->setEnabled(false);

	// �̶߳���ʵ����
	myThread = new MyThread();
	int devices_num = 0;
	// ���̵߳��ź���۽��а�
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

/*************************************************** ����ۺ��� *************************************************** */

// ch:���²����豸��ť:ö�� | en:Click Find Device button:Enumeration 
void Camera::OnBnClickedEnumButton()
{
	
	ui->comboBox_Devices->clear();                              //����豸�б���е���Ϣ  
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));    // ch:��ʼ���豸��Ϣ�б�  

	int nRet = MV_OK;
	nRet = CMyCamera::EnumDevices(&m_stDevList);                // ch:ö�������������豸,����豸����
	
	devices_num = m_stDevList.nDeviceNum;

	if (devices_num > 0)
	{
		// �豸������0��ʹ�ܰ���
		ui->bntConnectCamrea->setEnabled(true);
	}

	for (unsigned int i = 0, j = 0; j < m_stDevList.nDeviceNum; j++, i++)
	{
		m_pcMyCamera[i] = new CMyCamera;                        // �������ָ�����
		// ��������ʼ��
		m_pcMyCamera[i]->m_pBufForDriver = NULL;
		m_pcMyCamera[i]->m_pBufForSaveImage = NULL;
		m_pcMyCamera[i]->m_nBufSizeForDriver = 0;
		m_pcMyCamera[i]->m_nBufSizeForSaveImage = 0;
		m_pcMyCamera[i]->m_nTLayerType = m_stDevList.pDeviceInfo[j]->nTLayerType;

		nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[j]); //�����
	}



	ui->bntCloseCamera->setEnabled(true);
	ui->bntSingleTigger->setEnabled(true);
	ui->bntContinueTigger->setEnabled(true);


	if (devices_num > 0)
	{
		ui->Image_Label_R->setText("do");
		//����Ϊ����ģʽ
		m_pcMyCamera[0]->setTriggerMode(1);
		//���ô���ԴΪ����
		m_pcMyCamera[0]->setTriggerSource(7);
		//�����ع�ʱ��
		m_pcMyCamera[0]->setExposureTime(30000);
		//��������ɼ�
		m_pcMyCamera[0]->StartGrabbing();

		myThread->getCameraPtr(m_pcMyCamera[0]); //�̻߳�ȡ���ָ��
		myThread->getImagePtr(images[0]);        //�̻߳�ȡͼ��ָ��
	}
}


// �������
void Camera::bntOpen_clicked() 
{
	///*
	//ʹ�ܰ���
	//*/
	//ui->bntCloseCamera->setEnabled(true);
	//ui->bntSingleTigger->setEnabled(true);
	//ui->bntContinueTigger->setEnabled(true);


	//if (devices_num > 0)
	//{
	//	ui->Image_Label_R->setText("do");
	//	//����Ϊ����ģʽ
	//	m_pcMyCamera[0]->setTriggerMode(1);
	//	//���ô���ԴΪ����
	//	m_pcMyCamera[0]->setTriggerSource(7);
	//	//�����ع�ʱ��
	//	m_pcMyCamera[0]->setExposureTime(30000);
	//	//��������ɼ�
	//	m_pcMyCamera[0]->StartGrabbing();

	//	myThread->getCameraPtr(m_pcMyCamera[0]); //�̻߳�ȡ���ָ��
	//	myThread->getImagePtr(images[0]);        //�̻߳�ȡͼ��ָ��
	//}

}





//�ɼ�����ͼ��
void Camera::bntSingle_Tigger_clicked()
{

}

//�����ɼ�ͼ��
void Camera::bntContinue_Tigger_clicked()
{
	
	if (!myThread->isRunning())
	{
		myThread->start();
 
	}
 
	//m_pcMyCamera[0]->softTrigger();
	//m_pcMyCamera[0]->ReadBuffer(*images[0]);//��ȡMat��ʽ��ͼ��
}


void Camera::Left_Img_display()
{

	cv::imshow("src", *this->images[0]);
	cv::Mat rgb;
	cv::cvtColor(*images[0], rgb, CV_BGR2RGB);

	////�ж��Ǻڰס���ɫͼ��
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

	//��ʾͼ��
	ui->Image_Label_L->setPixmap(QPixmap::fromImage(QmyImage));


}


void Camera::bntClose_clicked()//ֹͣ�ɼ�
{
	/*
	��������
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
