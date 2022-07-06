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
#define BLUE_H_MIN 97
#define BLUE_H_MAX 118
#define BLUE_S_MIN 57
#define BLUE_S_MAX 255
#define BLUE_V_MIN 77
#define BLUE_V_MAX 255

//WHITE
#define WHITE_H_MIN 0
#define WHITE_H_MAX 180
#define WHITE_S_MIN 0
#define WHITE_S_MAX 18
#define WHITE_V_MIN 231
#define WHITE_V_MAX 255


Mat getEdges(Mat hsvMask){
	Mat imgCanny;

	GaussianBlur(hsvMask,hsvMask,Size(3,3),20,5);//blur pred odredjivanje ivica
	Canny(hsvMask,imgCanny,100,300);// canny detektor ivica
	Mat kernel=getStructuringElement(MORPH_RECT,Size(7,7));// kernel za naglasavanje ivica
	dilate(imgCanny,imgCanny,kernel);// dilate nam poboljsa vidljivost ivica

	return imgCanny;
}

void getContours(Mat ivice, Mat original){
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
		cout<<"cut " + to_string(i)<<endl;
		cout<<"area:";
		cout<<area<<endl;
		if(area>1000){
			float peri=arcLength(contours[i],true);
			approxPolyDP(contours[i],conPoly[i],0.025*peri,true);

			cout<<"conpoly:";
			cout<<conPoly[i]<<endl;

			if((conPoly[i].size() != 3) && (conPoly[i].size() != 4) && (conPoly[i].size() != 8)){
				continue;
			}
			
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			drawContours(original,contours,i,Scalar(161, 111, 245),5);
			boundRect[i] = boundingRect(conPoly[i]);
			
			Mat crop(ogCopy.rows, ogCopy.cols, CV_8UC3);
			crop.setTo(Scalar(161, 111, 245));
			ogCopy.copyTo(crop,mask);
			normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

			Mat imgcrop=crop(boundRect[i]);
			imshow("cut"+ to_string(i),imgcrop);
			imshow("Slika sa maskiranom pozadinom",crop);
		}
		cout<<endl<<endl<<endl;
	}
}


int main() {
	Mat src, ImgOriginal, imgCanny, imgHSV, hsvMask, red_hsvMask1, red_hsvMask2, blue_hsvMask, white_hsvMask;
	
	//blue
	//src = cv::imread("data/blue/crosswalk.jpg");
	//src = cv::imread("data/blue/parking.jpg");
	//src = cv::imread("data/blue/straight_ahead_round.jpeg");	

	//green
	//src = cv::imread("data/green/highway_entry.png");
	//src = cv::imread("data/green/highway_exit.jpeg");

	//red
	src = cv::imread("data/red/no_parking.jpg");
	//src = cv::imread("data/red/no_priority.jpg");
	//src = cv::imread("data/red/stop_sign.jpg");
	//src = cv::imread("data/red/stop.jpg");
	//src = cv::imread("data/red/wrong_way.jpg");
	//src = cv::imread("data/red/signs.png");

	//white
	//src = cv::imread("data/white/priority.jpg");

	//mixed
	//src = cv::imread("data/mixed/Auto-put-petlja-Nis-jug.jpg");
	//src = cv::imread("data/mixed/pedestrian_zone.jpg");
	//src = cv::imread("data/mixed/procced_straight.jpg");

		
	if(src.empty()) throw runtime_error("Cannot open image!");
	cout<<"Img size:"<<src.size()<<endl;
	resize(src,ImgOriginal,Size(),0.5,0.5);//neutral
	cvtColor(ImgOriginal,imgHSV,COLOR_BGR2HSV);//konverzija u hsv prostor


	Scalar red_lower_1(RED_H_MIN_1,RED_S_MIN,RED_V_MIN);
	Scalar red_upper_1(RED_H_MAX_1,RED_S_MAX,RED_V_MAX);
	inRange(imgHSV,red_lower_1,red_upper_1,red_hsvMask1);

	Scalar red_lower_2(RED_H_MIN_2,RED_S_MIN,RED_V_MIN);
	Scalar red_upper_2(RED_H_MAX_2,RED_S_MAX,RED_V_MAX);
	inRange(imgHSV,red_lower_2,red_upper_2,red_hsvMask2);

	Scalar blue_lower(BLUE_H_MIN,BLUE_S_MIN,BLUE_V_MIN);
	Scalar blue_upper(BLUE_H_MAX,BLUE_S_MAX,BLUE_V_MAX);
	inRange(imgHSV,blue_lower,blue_upper,blue_hsvMask);

	// Scalar white_lower(WHITE_H_MIN, WHITE_S_MIN, WHITE_V_MIN);
	// Scalar white_upper(WHITE_H_MAX, WHITE_S_MAX, WHITE_V_MAX);
	// inRange(imgHSV,white_lower,white_upper,white_hsvMask);;

	hsvMask=red_hsvMask1 | red_hsvMask2 | blue_hsvMask;// od vise delova maske dobijamo jednu

	imgCanny = getEdges(hsvMask);
	imshow("Canny", imgCanny);
	getContours(imgCanny, ImgOriginal);


	imshow("Image",ImgOriginal);
	waitKey(0);
	return 0;
}
