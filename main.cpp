//TODO values for green, input select


#include "Enums.hpp"

#include <visualizer.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
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

//YELLOW
#define YELLOW_H_MIN 10
#define YELLOW_H_MAX 35
#define YELLOW_S_MIN 165
#define YELLOW_S_MAX 255
#define YELLOW_V_MIN 100
#define YELLOW_V_MAX 255

//GREEN
#define GREEN_H_MIN 68
#define GREEN_H_MAX 102
#define GREEN_S_MIN 72
#define GREEN_S_MAX 126
#define GREEN_V_MIN 156
#define GREEN_V_MAX 255


void drawPoints(Mat img, vector<Point> points, Scalar color){
	Mat kopija;
	resize(img, kopija,Size(), 1, 1);
	for(int i = 0; i< points.size(); i++){
		circle(kopija, points[i], 5, color, FILLED);
		putText(kopija, to_string(i), points[i], FONT_HERSHEY_PLAIN, 3, color, 3);
	}
	imshow("Aproksimacija", kopija);
}

Mat findColor(Mat img, int hmin1, int hmax1, int hmin2, int hmax2, int smin, int smax, int vmin, int vmax){

	Mat hsvMask, hsvMask1, hsvMask2, imgHSV;

	cvtColor(img,imgHSV,COLOR_BGR2HSV);//konverzija u hsv prostor

	Scalar lower_1(hmin1, smin, vmin);
	Scalar upper_1(hmax1, smax, vmax);
	Scalar lower_2(hmin2, smin, vmin);
	Scalar upper_2(hmax2, smax, vmax);
	inRange(imgHSV,lower_1,upper_1,hsvMask1);
	inRange(imgHSV, lower_2, upper_2, hsvMask2);

	hsvMask=hsvMask1 | hsvMask2;

	return hsvMask;
}


Mat getEdges(Mat hsvMask){
	Mat imgCanny;

	GaussianBlur(hsvMask,hsvMask,Size(3,3),20,5);//blur pred odredjivanje ivica
	Canny(hsvMask,imgCanny,100,300);// canny detektor ivica
	Mat kernel=getStructuringElement(MORPH_RECT,Size(7,7));// kernel za naglasavanje ivica
	dilate(imgCanny,imgCanny,kernel);// dilate nam poboljsa vidljivost ivica

	return imgCanny;
}

int cut_size = 0;


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
		if(area<1000) continue;
		float peri=arcLength(contours[i],true);
		approxPolyDP(contours[i],conPoly[i],0.025*peri,true);

		cout<<"conpoly:";
		cout<<conPoly[i]<<endl;

		if((conPoly[i].size() != 3) && (conPoly[i].size() != 4) && (conPoly[i].size() != 8)) continue;
		
		drawContours(mask, contours, i, Scalar(255), CV_FILLED);
		drawContours(original,contours,i,Scalar(161, 111, 245),5);
		boundRect[i] = boundingRect(conPoly[i]);

		drawPoints(original, conPoly[i], Scalar(0,255,0));
		
		Mat crop(ogCopy.rows, ogCopy.cols, CV_8UC3);
		crop.setTo(Scalar(161, 111, 245));
		ogCopy.copyTo(crop,mask);
		normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

		Mat imgcrop=crop(boundRect[i]);
		imshow("cut"+ to_string(cut_size),imgcrop);
		cut_size += 1;
		imshow("Slika sa maskiranom pozadinom",crop);
	}
}


int main() {
	Mat src, ImgOriginal, imgCanny, hsvMask;
	int hmin1, hmax1, hmin2, hmax2, smin, smax, vmin, vmax;
	
	//blue
	//src = cv::imread("data/blue/crosswalk.jpg");
	//src = cv::imread("data/blue/parking.jpg");
	//src = cv::imread("data/blue/straight_ahead_round.jpeg");	

	//green
	//src = cv::imread("data/green/highway_entry.png");
	//src = cv::imread("data/green/highway_exit.jpeg");

	//red
	//src = cv::imread("data/red/no_parking.jpg");
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
	src = cv::imread("data/mixed/procced_straight.jpg");
	if(src.empty()) throw runtime_error("Cannot open image!");

	namedWindow("Trackbars", (200, 200));

	string s_w = "0 : RED\n1 : BLUE\n2 : GREEN\n3 : YELLOW";
	createTrackbar(s_w, "Trackbars", 0, 3);
	setTrackbarPos(s_w, "Trackbars", 0);

	string r_w = "Resize";
	createTrackbar(r_w, "Trackbars", 0, 10);
	setTrackbarPos(r_w, "Trackbars", 5);


	while(true){
		int s_b = getTrackbarPos(s_w, "Trackbars");

		switch(s_b){
			case 0: //red
				hmin1 = RED_H_MIN_1;
				hmax1 = RED_H_MAX_1;
				hmin2 = RED_H_MIN_2;
				hmax2 = RED_H_MAX_2;
				smin = RED_S_MIN;
				smax = RED_S_MAX;
				vmin = RED_V_MIN;
				vmax = RED_V_MAX;
				break;
			case 1://blue
				hmin1 = BLUE_H_MIN;
				hmax1 = BLUE_H_MAX;
				hmin2 = 0;
				hmax2 = 0;
				smin = BLUE_S_MIN;
				smax = BLUE_S_MAX;
				vmin = BLUE_V_MIN;
				vmax = BLUE_V_MAX;
				break;
			case 2://green
				hmin1 = GREEN_H_MIN;
				hmax1 = GREEN_H_MAX;
				hmin2 = 0;
				hmax2 = 0;
				smin = GREEN_S_MIN;
				smax = GREEN_S_MAX;
				vmin = GREEN_V_MIN;
				vmax = GREEN_V_MAX;
				break;
			case 3://yellow
				hmin1 = YELLOW_H_MIN;
				hmax1 = YELLOW_H_MAX;
				hmin2 = 0;
				hmax2 = 0;
				smin = YELLOW_S_MIN;
				smax = YELLOW_S_MAX;
				vmin = YELLOW_V_MIN;
				vmax = YELLOW_V_MAX;
				break;
			default:
				break;
		}

		int r_b = getTrackbarPos(r_w, "Trackbars") + 1;

		resize(src,ImgOriginal,Size(), r_b*0.2, r_b*0.2);
	
		hsvMask = findColor(ImgOriginal, hmin1, hmax1, hmin2, hmax2, smin, smax, vmin, vmax);

		imgCanny = getEdges(hsvMask);

		getContours(imgCanny, ImgOriginal);

		imshow("Originalna slika",ImgOriginal);
		imshow("Canny", imgCanny);
		waitKey(0);
		destroyWindow("Originalna slika");
		destroyWindow("Canny");
		destroyWindow("Slika sa maskiranom pozadinom");
		while(cut_size >= 0){
			destroyWindow("cut"+ to_string(cut_size));
			cut_size -= 1;
		}
		cut_size = 0;
	}


	return 0;
}
