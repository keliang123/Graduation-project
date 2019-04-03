#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <ctime>
#include <time.h>
#include <string>
using namespace cv;
using namespace std;

double area;
int MER, MER_length;
float temp = 0;
float temp_length = 0;
Mat frame, frame0,frame1;
Mat edges;
int data_ten=0; //取前十次
int data_ten_x = 0, data_ten_y = 0; //前十次x y平均值
int maxarea = 0;//最大连通域面积
int main()
{
	/*
	String rtsp_addr = "rtsp://admin:wzh199465@192.168.1.64:554/MPEG-4/ch1/main/av_stream";
	VideoCapture capture(rtsp_addr);
	capture.set(CV_CAP_PROP_FPS, 10);//帧率 帧/秒
	capture.open(rtsp_addr);
	*/
	
	VideoCapture capture;
	capture.open("01.avi");
	capture.set(CV_CAP_PROP_FPS, 10);//帧率 帧/秒
	//while (capture.isOpened())
	while(1)
	{
		capture >> frame;
		capture >> frame0;
		capture >> frame1;
		//namedWindow("...", CV_WINDOW_NORMAL);
		//imshow("...", frame);
		//char key = static_cast<char>(cvWaitKey(50));//控制视频流的帧率，10ms一帧
		//Mat frame = imread("01.jpg");
		 //Mat frame0 = imread("01.jpg");
		cvtColor(frame, edges, CV_BGR2GRAY);//灰度化
		//二值化
		Mat binImage;
		threshold(edges, binImage, 210, 255, CV_THRESH_BINARY);
		//使用3*3内核来降噪
		//blur(edges, edges, Size(3, 3));//进行模糊
		//medianBlur(edges, edges, 3); //3*3中值滤波
		//【5】形态学滤波
		Mat erodedImage, dilatedImage;
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		morphologyEx(binImage, erodedImage, MORPH_ERODE, element, Point(-1, -1), 3);
		morphologyEx(erodedImage, dilatedImage, MORPH_DILATE, element, Point(-1, -1), 3);
		//区域特征：查找轮廓
		vector<vector<Point>> contours;
		findContours(binImage, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		vector<RotatedRect> box(contours.size()); //定义最小外接矩形集合
		//在白色图像上绘制黑色轮廓
		//Mat dstImage(edges.size(), CV_8U, Scalar(255));
		//drawContours(dstImage, contours, -1, Scalar(0), 2);
		//限定轮廓的周长，提取有效轮廓
		int cmin = 30;
		int cmax = 100;
		vector<vector<Point>>::const_iterator itc = contours.begin();
		
		while (itc != contours.end())
		{
			if (itc->size() < cmin || itc->size() > cmax)
				itc = contours.erase(itc);
			else
			{
				//cout << "周长：" << itc->size() << endl;
				++itc;
			}
		}
		//cout << "周长：" << itc->size() << endl;
		int x0, y0, length0;
		int x1_cir[100], y1_cir[100];
		int x_cir_center = 0, y_cir_center = 0;
		int m_cir = 0; //存放圆重心坐标个数
		Point center_point_cir;   //矩形度判断圆
		Mat result(edges.size(), CV_8UC3, Scalar(0, 0, 0));
		for (int i = 0; i < contours.size(); i++)
		{
			box[i] = minAreaRect(Mat(contours[i]));  //计算每个轮廓最小外接矩形
			MER = box[i].size.width*box[i].size.height;//计算每个轮廓最小外接矩形面积
			MER_length = (box[i].size.width + box[i].size.height) * 2; //计算每个轮廓最小外接矩形周长
			float ratio = (float)(box[i].size.width) / (float)(box[i].size.height);
			x0 = box[i].center.x;
			y0 = box[i].center.y;   //轮廓中心
			temp = fabs(contourArea(contours[i]));      //连通域面积
			temp_length = fabs(arcLength(contours[i], true));  //连通域的周长
			double R = temp / MER;    //R矩形度 
			//检测圆形
			if (R > 0.7 && R < 0.8 && ratio>0.8 && ratio <1.2)
			{
				if (MER > maxarea)
				{
					maxarea = MER;
					//cout << "轮廓最小外接矩形面积：" << maxarea << endl;
				}
				drawContours(frame0, contours, i, Scalar(0, 0, 255), 2,8);
				result.copyTo(frame, result);
				Mat tmp(contours.at(i));
				Moments moment = moments(tmp, false);
				if (moment.m00 != 0)//除数不能为0
				{
					int x = cvRound(moment.m10 / moment.m00);//计算重心横坐标
					int y = cvRound(moment.m01 / moment.m00);//计算重心纵坐标
					x1_cir[m_cir] = x;
					y1_cir[m_cir] = y;
					m_cir++;
					//cout << "圆形连通域重心坐标是：（" << x << "," << y << ")" << endl;
					//circle(edges, Point(x, y), 5, Scalar(0));
				}
			}
		}
		//求各个重心坐标平均值
		cout << "*****************************" << endl;
		cout << "轮廓最小外接矩形面积：" << maxarea << endl;
		//cout << m_cir << endl;
		int s_cir = 0, r_cir = 0;
		int x_data_ten[100], y_data_ten[100];
		int x_count_ten=0, y_count_ten=0;
		for (int i = 0; i < m_cir; i++)
		{
			s_cir = s_cir + x1_cir[i];
			r_cir = r_cir + y1_cir[i];
		}
		if (m_cir != 0)
		{
			x_cir_center = (int)s_cir / m_cir;
			y_cir_center = (int)r_cir / m_cir;
			x_data_ten[data_ten] = x_cir_center;
			y_data_ten[data_ten] = y_cir_center;
			cout << "圆重心坐标平均值是：（" << x_cir_center << "," << y_cir_center << ")" << endl;
			center_point_cir = Point(x_cir_center, y_cir_center);
			data_ten++;
			
		}
		else cout << "没有检测到圆" << endl;
		cout << "data_ten: " << data_ten << endl;
		
		if (data_ten ==10)
		{
			for (int m = 0; m < data_ten; m++)
			{
				x_count_ten = x_count_ten + x_data_ten[m];
				y_count_ten = y_count_ten + y_data_ten[m];
			}
			
			data_ten_x = x_count_ten / data_ten;
			data_ten_y = y_count_ten / data_ten;
			cout << "前十次平均x y值：(" << data_ten_x << "," << data_ten_y << ")," << endl;
		}
		//else continue;
		cout << "前十次平均x y值：(" << data_ten_x << "," << data_ten_y << ")," << endl;
		float len;//和前十次平均值相比 相对移动距离
		if (m_cir != 0)
		{
			len =0.0064* sqrt(fabs(x_cir_center - data_ten_x)*fabs(x_cir_center - data_ten_x) + fabs(y_cir_center - data_ten_y)*fabs(y_cir_center - data_ten_y));
			cout << "len: " << len << endl;
		}
		//namedWindow("阈值分割", CV_WINDOW_NORMAL);
		//imshow("阈值分割", binImage);
		//imshow("23-3次腐蚀运算", erodedImage);
		//imshow("23-3次膨胀运算", dilatedImage);
		//namedWindow("23-区域轮廓图", CV_WINDOW_NORMAL);
		//imshow("23-区域轮廓图", dstImage);
		namedWindow("标志物轮廓", CV_WINDOW_NORMAL);
		imshow("标志物轮廓", frame0);
		//namedWindow("23-灰度图", CV_WINDOW_NORMAL);
		//imshow("23-灰度图", edges);
		circle(frame1,center_point_cir, 1, Scalar(0, 0, 255), 16, 8, 0);    //在原图上画出圆心
		//circle(frame0, center_point_tri, 2, Scalar(0, 0, 255), 2, 8, 0);
		namedWindow("检测结果", CV_WINDOW_NORMAL);
		imshow("检测结果", frame1);
		waitKey(20);
	}
	//waitKey(30);
	return 0;
}