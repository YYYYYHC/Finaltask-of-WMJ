#include <iostream>
#include <thread>
#include <mutex>
#include<opencv2/opencv.hpp>
#include<math.h>
#include<boost/asio.hpp>
#include<boost/bind/bind.hpp>
#include<sstream>

using namespace std;
using namespace cv;
using namespace boost::asio;
using namespace boost::placeholders;

void input();          
void process(); 

VideoCapture capture;
Mat frame,frame_t,output;
int flag = 0,judge=0;      
std::mutex mtx;   
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
vector<Point> point;
vector<float> vec;
Point Mid;
string COM;
string Convert(float Num)
{
	ostringstream oss;
	oss<<Num;
	string str(oss.str());
	return str;
}

void connect()
{
	io_service iosev;
	serial_port sp(iosev,COM);
	sp.set_option(serial_port::baud_rate(115200));
	sp.set_option(serial_port::flow_control(serial_port::flow_control::none));
	sp.set_option(serial_port::parity(serial_port::parity::none));
	sp.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
	sp.set_option(serial_port::character_size(8));
	string trans = "x="+Convert(vec[0])+" y="+Convert(vec[1]);
	write(sp,buffer(trans,trans.length()));
}

double disof(Point a,Point b)
{
	return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));

}

void findMid()
{
	Point lu=contours[0][0];
	Point ld=contours[0][0];
	Point ru=contours[0][0];
	Point rd=contours[0][0];
	for(int i=0;i<contours.size();i++)
	{
		for(int j=0;j<contours[i].size();j++)
		{

			lu = 0.1*contours[i][j].x+1.9*contours[i][j].y<0.1*lu.x+1.9*lu.y?contours[i][j]:lu;
			ld = 0.1*contours[i][j].x-1.9*contours[i][j].y<0.1*ld.x-1.9*ld.y?contours[i][j]:ld;
			ru = -contours[i][j].x+contours[i][j].y<-ru.x+ru.y?contours[i][j]:ru;
			rd = contours[i][j].x+contours[i][j].y>rd.x+rd.y?contours[i][j]:rd;

		}


	}
	Mat a=(Mat_<float>(2,2)<<lu.y-rd.y,-lu.x+rd.x,ru.y-ld.y,-ru.x+ld.x);
	Mat b=(Mat_<float>(2,1)<<rd.x*lu.y-lu.x*rd.y,ld.x*ru.y-ru.x*ld.y);
	Mat c=a.inv()*b;
	vec = c.reshape(1,1);
	cout<<"midx="<<vec[0]<<" midy="<<vec[1];
	Point mid=Point(vec[0],vec[1]);
	circle(frame,mid,5,Scalar(0,255,0),2,8);
	
}

/*
void findMid()
{
	cout<<"contours.size="<<contours.size()<<endl;
	vector<Point> pointu;
	vector<Point> pointd;
	for(int i=0;i<contours.size();i++)
	{
		pointu[i]=contours[i][0];
		pointd[i]=contours[i][0];
		for(int j=0;j<contours[i].size();j++)
			{
				pointu[i]=contours[i][j].y>pointu[i].y?contours[i][j]:pointu[i]; 
		
				pointd[i]=contours[i][j].y<pointd[i].y?contours[i][j]:pointd[i];
			}
	}
	int flag1,flag2;

	for(int i=0;i<pointu.size();i++)
		cout<<"u"<<i<<"= "<<pointu[i].x<<" "<<pointu[i].y<<endl;

	for(int i=0;i<contours.size();i++)
	{
		double target=10000;
		for(int j=0;j<contours.size();j++)
		{
			if(j==i) continue;
			double tojudge =fabs(disof(pointu[i],pointu[j])-disof(pointd[i],pointd[j]));
			if(tojudge<target) 
			{
				target=tojudge;
				flag1=i;flag2=j;
			}
		}
	}

	Point ld=pointd[flag1];
	Point lu=pointu[flag1];
	Point rd=pointd[flag2];
	Point ru=pointu[flag2];
//	circle(frame,ld,5,Scalar(0,255,255),2,8);
//	namedWindow("pointout",0);
//	imshow("pointout",frame);


}
*/
int main(int argc,char **argv)
{
		COM=argv[1];
		
		frame=capture.open("IMG_2223.mp4");
    	if(!capture.isOpened())
     	{
	    	printf("can not open \n");
	    	return -1;
    	} 	 	
    	else flag=1;
        std::thread thrd_1(input);    
        std::thread thrd_2(process);     
        thrd_1.join();        
        thrd_2.join();        
        return 0;
}

void input()  // read each frame in and save it in 'frame'
{
    while (capture.read(frame))
    {
    	frame_t=frame.clone(); 
        std::lock_guard<std::mutex> mtx_locker(mtx);  //lock
        if (flag)
            {
        		flag=0;
				output=frame.clone();
			}
		if(waitKey(1)==27) break;
    }

}

void process()//process 'output' to get results
{
    while (!judge)
    {
     
        std::lock_guard<std::mutex> mtx_locker(mtx);   
        if (!flag)
            {
				Mat imghsv;	
				cvtColor(output,imghsv,COLOR_BGR2HSV);				
				
				Mat thresholdimg;
				inRange(imghsv,Scalar(0,140,130),Scalar(10,255,255),thresholdimg);
				output = thresholdimg.clone();
				
				findContours(output,contours,hierarchy,RETR_TREE,CHAIN_APPROX_SIMPLE,Point());
				
				for(size_t i =0;i<contours.size();i++)
					drawContours(frame_t,contours,i,Scalar(255,0,0),5,8,Mat(),0,Point());
				
				findMid();
				connect();
				cout << "processed"<< endl;
        		namedWindow("result",0);
				imshow("result",frame);
				waitKey(5);
				if(waitKey(1)==27) break;
				flag=1;
			}
    }
}


