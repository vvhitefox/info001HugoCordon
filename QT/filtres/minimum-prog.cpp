// minimum-prog.cpp
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <opencv2/highgui.hpp>
using namespace cv;

namespace filtres {

/*
 {,,}
,{,,}
,{,,}
*/

float m9[3][3] = { {1.0/16.0,2.0/16.0,1.0/16.0}
               ,   {2.0/16.0,4.0/16.0,2.0/16.0}
               ,   {1.0/16.0,2.0/16.0,1.0/16.0}};
    Mat moyen = Mat( 3, 3, CV_32FC1,m9);

float c9[3][3] = {
        {0.0,1.0,0.0}
       ,{1.0,-4.0,1.0}
       ,{0.0,1.0,0.0}};
    Mat Laplacien = Mat(3, 3, CV_32FC1,c9);

    float deriv1[3][3] = {{-0.25f,-0.5f,-0.25f},{0,0,0},{0.25f,0.5f,0.25f}};
    float deriv2[3][3] = {{0.25f,0,-0.25f},{0.5f,0,-0.5f},{0.25,0,-0.25}};
    Mat derivX = Mat(3,3,CV_32FC1,filtres::deriv1);
    Mat derivY = Mat(3,3,CV_32FC1,filtres::deriv2);
};

Mat filtreM(Mat& M,Mat& f){
    filter2D(f,f,-1,M,Point2i(-1,-1));
    return f;
}

Mat convo(Mat M,Mat f,Mat g){
    Mat sim = M.clone();
    if(g.channels() > 1)
        cvtColor(f,g,COLOR_BGR2GRAY);
    else
        g = f.clone();

    flip(M,sim,-1);
    filter2D(g,g,-1,M,Point2i(-1,-1),128,4);

    return g;
}

Mat convo(Mat M,Mat f){
    Mat sim = M.clone();
    if(f.channels() > 1)
        cvtColor(f,f,COLOR_BGR2GRAY);

    flip(M,sim,-1);
    filter2D(f,f,-1,M,Point2i(-1,-1),128,4);

    return f;
}

Mat Gradient(Mat M,Mat f){
    Mat a = Mat::zeros(f.rows,f.cols,CV_8UC1);
    Mat b = Mat::zeros(f.rows,f.cols,CV_8UC1);
    Mat z = Mat::zeros(f.rows,f.cols,CV_8UC1);

    Mat c = Mat(f.rows,f.cols,CV_8UC3);
    if(f.channels() > 1)
        cvtColor(f,f,COLOR_BGR2GRAY);
    a = convo(filtres::derivX,f,a);
    b = convo(filtres::derivY,f,b);
    std::vector<Mat> abz = std::vector<Mat>({a,b,z});
    merge(abz,c);
    c.forEach<Point3_<uchar>>([](Point3_<uchar>& color, const int position[]) -> void {
        color.z = sqrt(color.y*color.y + color.x*color.x);
    });
    auto m = std::vector<Mat>({a,b,z});
    cv::split(c,m);
    f = m[2].clone();

    return f;
}

Mat MarrHildreth(Mat in,int T){
    Mat out = Mat(in.rows, in.cols, CV_8U,1);
    if(in.channels() > 1)
        cvtColor(in,out,COLOR_BGR2GRAY);
    convo(filtres::Laplacien,out);
    int v = 2;
    out.forEach<uchar>([out,v](uchar& color,const int position[]) -> void{
        int x = position[0];
        int y = position[1];
        int height = out.cols;
        int width  = out.rows;
        int sign = ((color-127) < 0)?1:-1;
        int sign2 = 0;
        bool change = 0;
        for(int i = -v + (v/2); i < v/2;i++){
            if(x-i > 0 && x+i < height){
                uchar colH = out.at<uchar>(x+i,y);
                sign2 = ((colH-127) < 0)?1:-1;
                change = (sign != sign2);
                if(change)
                    break;
            }
            if(y-i > 0 && y+i < height){
                uchar colV = out.at<uchar>(x,y+i);
                sign2 = ((colV-127) < 0)?1:-1;
                change = (sign != sign2);
                if(change)
                    break;
            }
        }
        color = change?color:0;
    });
    std::cout << out;
    threshold(out,out,T,255,CV_8UC1);

    return out;
}

int main(int argc,char** args)
{
  int value = 128;
  if(argc < 2){
      std::cout << "usage : minimum-program <imagePath>"<<"\n";
      std::cout << "" << (std::string) args[1];
      return 0;
  }
  std::string imagePath = (std::string) args[1];
  namedWindow( "Youpi",WINDOW_NORMAL);               // crée une fenêtre
  createTrackbar( "track", "Youpi", nullptr, 255, NULL); // un slider
  setTrackbarPos( "track", "Youpi", value ); // met à 128
  Mat f = imread("lena.png");          // lit l'image "lena.png"
  imshow( "Youpi", f );              // l'affiche dans la fenêtre
  int keycode;
  int lastkeycode;
  while ((keycode = waitKey(50)) != 'q' ){
    int T = getTrackbarPos( "track", "Youpi" );
    char codes[] = {'p','a','m','x','y','g','c'};
    if(std::find(std::begin(codes), std::end(codes), keycode) != std::end(codes)){
        lastkeycode == keycode;
    }
    switch(keycode){
        lastkeycode = keycode;
        case 'p':
            f = MarrHildreth(f,T);
            break;
        case 'a':
            filtreM(filtres::moyen,f);
            break;
        case 'm':
            medianBlur(f,f,3);
            break;
        case 'x':
            f = convo(filtres::derivX,f,f);
            break;
        case 'y':
            f= convo(filtres::derivY,f,f);
            break;
        case 'g':
            f = Gradient(f,f);
            break;
        case 'c':
            float alpha = (float)getTrackbarPos( "track", "Youpi" )/255.0f;
            float c92[3][3] = {{0,-alpha,0}
                               ,{-alpha,1.0f+4.0f*alpha,-alpha}
                               ,{0,-alpha,0}};
            Mat alphaXL = Mat(3,3,CV_32FC1,c92);
            std::cout<<alphaXL<<"\n";
            filtreM(alphaXL,f);
            break;
    }
    imshow( "Youpi", f );
  }
  std::stringstream ss;
  ss << "imageKey"<<lastkeycode << ".png";
  imwrite(ss.str(),f);
  return 0;
  std::cout << "value=" << value << std::endl;
}
