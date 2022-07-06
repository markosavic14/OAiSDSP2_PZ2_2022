#include "Enums.hpp"

#include <visualizer.hpp>

#include <iostream>
#include <cstdlib>
using namespace std;

#include <unistd.h>

#define DEBUG(x) do{ std::cout << #x << " = " << x << std::endl; }while(0)

#include "opencv2/opencv.hpp"
using namespace cv;

int hmin1=0, hmin2=165, smin=132, vmin=69;
int hmax1=15, hmax2=180, smax=255, vmax=255;

void getContours(Mat ivice,Mat original){

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(ivice,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);//nalazi konture
	//drawContours(original,contours,-1,Scalar(255,0,255),5);//crta sve konture koje nadje
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
			//fillConvexPoly(original,contours[i],Scalar(161, 111, 245));
			//drawContours(original,conPoly,i,Scalar(255,0,255),3);
			//drawContours(bground,contours,i,Scalar(0, 111, 245),1);
			boundRect[i] = boundingRect(conPoly[i]);
			
			//rectangle(original,boundRect[i].tl(),boundRect[i].br(),Scalar(161, 111, 245),3);

			Mat crop(ogCopy.rows, ogCopy.cols, CV_8UC3);
			crop.setTo(Scalar(161, 111, 245));
			ogCopy.copyTo(crop,mask);
			normalize(mask.clone(), mask, 0.0, 255.0, CV_MINMAX, CV_8UC1);

			Mat imgcrop=crop(boundRect[i]);
			imshow("cut"+ to_string(i),imgcrop);
			i+=1;
			imshow("Slika sa maskiranom pozadinom",crop);
			//imshow("Maska",mask);

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
	Mat src = cv::imread("data/no_priority.jpg");
	//Mat src = cv::imread("data/signs.png"); //test, ne treba koristiti ovo sem za debug
	Mat imgGray;
	Mat imgCanny;
	Mat Img;
	Mat imgHSV;
	Mat hsvMask,hsvMask1,hsvMask2;
	if(src.empty()){
		throw runtime_error("Cannot open image!");
	}
	DEBUG(0);
	//resize(src,Img,Size(),0.25,0.25);//za bas velike slike
	//resize(src,Img,Size(),0.5,0.5);//za velike slike
	resize(src,Img,Size(),1,1);//neutral
	//resize(src,Img,Size(),2,2);// za male slike

	//Rect roi(pocetna,krajnja);
	//imgCrop=img(roi)


	cvtColor(Img,imgGray,COLOR_BGR2GRAY);//konverzija u crno belo
	cvtColor(Img,imgHSV,COLOR_BGR2HSV);//konverzija u hsv prostor

	/*GaussianBlur(imgGray,imgGray,Size(3,3),5,0);//blur pred odredjivanje ivica
	Canny(imgGray,imgCanny,70,150);// canny detektor ivica
	Mat kernel=getStructuringElement(MORPH_RECT,Size(3,3));// kernel za naglasavanje ivica
	dilate(imgCanny,imgCanny,kernel);// dilate nam poboljsa vidljivost ivica
	*/
	
	/* namedWindow("Slajderi",(640,200));
	createTrackbar("Hue Min","Slajderi",&hmin,179);
	createTrackbar("Hue Max","Slajderi",&hmax,179);
	createTrackbar("Sat Min","Slajderi",&smin,255);
	createTrackbar("Sat Max","Slajderi",&smax,255);
	createTrackbar("Val Min","Slajderi",&vmin,255);
	createTrackbar("Val Max","Slajderi",&vmax,255); */

	//while(1){
		////////////////////////////////////////////////////////////////////////////////// crvena boja
		Scalar lower1(hmin1,smin,vmin);//donja granica 
		Scalar upper1(hmax1,smax,vmax);//gornja granica
		inRange(imgHSV,lower1,upper1,hsvMask1);//pravi crno belu sliku od boje

		Scalar lower2(hmin2,smin,vmin);//donja granica
		Scalar upper2(hmax2,smax,vmax);//gornja granica
		inRange(imgHSV,lower2,upper2,hsvMask2);//pravi crno belu sliku od boje

		hsvMask=hsvMask1+hsvMask2;// od dva dela slike(jer je crvena na pocetku i kraju range-a) dobijamo jedan

		GaussianBlur(hsvMask,hsvMask,Size(3,3),20,5);//blur pred odredjivanje ivica
		Canny(hsvMask,imgCanny,100,300);// canny detektor ivica
		Mat kernel=getStructuringElement(MORPH_RECT,Size(7,7));// kernel za naglasavanje ivica
		dilate(imgCanny,imgCanny,kernel);// dilate nam poboljsa vidljivost ivica
		
		getContours(imgCanny,Img);

		imshow("Image",Img);
		//////////////////////////////////////////////////////////////////////////////////
		//imshow("Image",src);
		//imshow("Image hsv", imgHSV);
		//imshow("Red filter",hsvMask);
		DEBUG(4);
		//imshow("Black and white image",imgGray);

		//imshow("Canny",imgCanny);


		waitKey(0);
	//}


	/*
	const uint16_t width = 10;
	const uint16_t height = 20;
	static uint8_t pix[width*height*3];
	for(uint16_t y = 0; y < height; y++){
		for(uint16_t x = 0; x < width; x++){
			uint32_t i = (y*width + x)*3;
			// Red.
			pix[i+0] = 0;
			pix[i+1] = 0;
			pix[i+2] = 255;
		}
	}

	for(uint16_t y = 3; y < height-3; y++){
		for(uint16_t x = 3; x < width-3; x++){
			uint32_t i = (y*width + x)*3;
			// Blue.
			pix[i+0] = 255;
			pix[i+1] = 0;
			pix[i+2] = 0;
		}
	}

	visualizer::img::show(
		"src",
		pix,
		width,
		height
	);


	visualizer::img::show(
		"h",
		src
	);

	int th_start_h0;

	while(true){
		th_start_h0 = 20;
		visualizer::slider::slider(
			"/win0/upper_half/upper_rigth_corner/th_start_h0",
			th_start_h0,
			[&](int& value){
				DEBUG(th_start_h0);
			}
		);

		for(int i = 0; i < 3; i++){
			visualizer::slider::update();
			DEBUG(th_start_h0);
			sleep(1);
		}
	}
	*/

	return 0;
}
