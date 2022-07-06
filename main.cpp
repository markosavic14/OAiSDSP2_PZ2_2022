#include "Enums.hpp"

#include <visualizer.hpp>

#include <iostream>
#include <cstdlib>
using namespace std;

#include <unistd.h>

#define DEBUG(x) do{ std::cout << #x << " = " << x << std::endl; }while(0)

#include "opencv2/opencv.hpp"
using namespace cv;

//RED
#define RED_H_MIN_1 0
#define RED_H_MAX_1 15
#define RED_H_MIN_2 159
#define RED_H_MAX_2 180
#define RED_S_MIN 132
#define RED_S_MAX 255
#define RED_V_MIN 69
#define RED_V_MAX 255

//BLUE
#define BLUE_H_MIN 90
#define BLUE_H_MAX 128
#define BLUE_S_MIN 50
#define BLUE_S_MAX 255
#define BLUE_V_MIN 70
#define BLUE_V_MAX 255

//WHITE
#define WHITE_H_MIN 0
#define WHITE_H_MAX 180
#define WHITE_S_MIN 0
#define WHITE_S_MAX 18
#define WHITE_V_MIN 231
#define WHITE_V_MAX 255

void getContours(Mat ivice,Mat original){

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(ivice,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);//nalazi konture
	int area;
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());
	Mat ogCopy(original);
	Mat mask = Mat::zeros(original.rows, original.cols, CV_8UC1);
	int i=0;
	for(int i=0;i<contours.size();i++){
		area=contourArea(contours[i]);
		cout<<area<<endl;
		if(area>1000){
			float peri=arcLength(contours[i],true);
			approxPolyDP(contours[i],conPoly[i],0.025*peri,true);
			
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			drawContours(original,contours,i,Scalar(161, 111, 245),5);
			boundRect[i] = boundingRect(conPoly[i]);
			
			Mat crop(ogCopy.rows, ogCopy.cols, CV_8UC3);
			crop.setTo(Scalar(161, 111, 245));
			ogCopy.copyTo(crop,mask);
			normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

			Mat imgcrop=crop(boundRect[i]);
			imshow("cut"+ to_string(i),imgcrop);
			i+=1;
			imshow("Slika sa maskiranom pozadinom",crop);
		}
	}
	
}

int main() {
	//visualizer::load_cfg("data/main.visualizer.yaml");
	//Mat src = cv::imread("data/stop_sign.jpg");
	//Mat src = cv::imread("data/stop.jpg");
	//Mat src = cv::imread("data/no_parking.jpg");
	//Mat src = cv::imread("data/black_spot.jpg");
	//Mat src = cv::imread("data/danger.jpg");
	//Mat src = cv::imread("data/wrong_way.jpg");
	//Mat src = cv::imread("data/no_priority.jpg");
	//Mat src = cv::imread("data/priority.jpg");
	//Mat src = cv::imread("data/signs.png"); //test, ne treba koristiti ovo sem za debug
	Mat imgGray;
	Mat imgCanny;
	Mat Img;
	Mat imgHSV;
	Mat hsvMask,hsvMask1,hsvMask2;
	if(src.empty()){
		throw runtime_error("Cannot open image!");
	}

	resize(src,Img,Size(),1,1);//neutral

	cvtColor(Img,imgGray,COLOR_BGR2GRAY);//konverzija u crno belo
	cvtColor(Img,imgHSV,COLOR_BGR2HSV);//konverzija u hsv prostor


	////////////////////////////////////////////////////////////////////////////////// crvena boja
	Scalar lower1(RED_H_MIN_1,RED_S_MIN,RED_V_MIN);//donja granica 
	Scalar upper1(RED_H_MAX_1,RED_S_MAX,RED_V_MAX);//gornja granica
	inRange(imgHSV,lower1,upper1,hsvMask1);//pravi crno belu sliku od boje

	Scalar lower2(RED_H_MIN_2,RED_S_MIN,RED_V_MIN);//donja granica
	Scalar upper2(RED_H_MAX_2,RED_S_MAX,RED_V_MAX);//gornja granica
	inRange(imgHSV,lower2,upper2,hsvMask2);//pravi crno belu sliku od boje

	hsvMask=hsvMask1 | hsvMask2;// od dva dela slike(jer je crvena na pocetku i kraju range-a) dobijamo jedan

	GaussianBlur(hsvMask,hsvMask,Size(3,3),20,5);//blur pred odredjivanje ivica
	Canny(hsvMask,imgCanny,100,300);// canny detektor ivica
	Mat kernel=getStructuringElement(MORPH_RECT,Size(7,7));// kernel za naglasavanje ivica
	dilate(imgCanny,imgCanny,kernel);// dilate nam poboljsa vidljivost ivica
	
	getContours(imgCanny,Img);

	imshow("Image",Img);
	DEBUG(4);


	waitKey(0);

	return 0;
}
