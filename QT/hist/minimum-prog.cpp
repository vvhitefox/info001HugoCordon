// minimum-prog.cpp
#include <iostream>
#include <stdlib.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
using namespace cv;


std::vector<double> histogramme( Mat image );
std::vector<double> histogramme_cumule( const std::vector<double>& h_I );

std::vector<double> histogramme( Mat image ){
    int64 size = image.rows*image.cols;
    std::vector<double> retval;
    retval.resize(256);
    retval.assign(256,0);

    int64 sum = 0;

    for(int i = 0; i < image.rows ; i++)
        for(int j = 0; j < image.cols ; j++){
            int a = image.at<uchar>(i,j);
            retval[a] += 1.0;
        }

    return retval;
}
std::vector<double> histogramme_cumule( const std::vector<double>& h_I,int64 size){
    std::vector<double> retval = h_I;

    for(int i = 1;i < h_I.size();i++){
        retval[i] = ((double)h_I[i])+retval[i-1];
    }
    for(int i = 1;i < h_I.size();i++){
        retval[i] /= size;
    }

    return retval;
}
typedef uchar Pixel;


cv::Mat afficheHistogrammes( const std::vector<double>& h_I,
                             const std::vector<double>& H_I )
{
   cv::Mat image( 256, 512, CV_8UC1 );
   const std::vector<double>& hist = h_I;
   const std::vector<double>&  histCum = H_I;
   double mx = *std::max_element(std::begin(hist), std::end(hist)).base();

   image.forEach<Pixel>([mx,hist,histCum](Pixel& pixel, const int position[]) -> void {
       int i=position[1];
       int j=255-position[0];
       if(i < 256)
            pixel = (j < (hist[i]/mx)*255)?255:0;
       else
            pixel = (j < histCum[i-255]*255)?255:0;
   });

   return image;
}

float distance_color_l2(Vec3f a, Vec3f b){
    return sqrt((a[0] - b[0])*(a[0] - b[0])
               +(a[1] - b[1])*(a[1] - b[1])
               +(a[2] - b[2])*(a[2] - b[2])
            );
}

Vec3f closest(Vec3f& p,std::vector<Vec3f> colors){
    Vec3f min= colors[0];
    float minDist = INFINITY;
    for(auto a : colors){
        float dist = distance_color_l2(a,p);
        if(dist < minDist){
            min = a;
            minDist = dist;
        }
    }
    Vec3f op = p;
    p = min;
    Vec3f tmp = op-p;
    return tmp;
}

int closest(uchar& p){
    int op = (int)p;
    if(op > 127)
        p = 255;
    else
        p = 0;
    return ((int)op-(int)p);
}

void ovrflow(Vec3f& a,Vec3f b,std::vector<Vec3f> colors){
    a = a+b;
}

int ovrflow(uchar& a,int8_t b){
    if(a + b > 255)
      return a = 255;
    if(a + b < 0)
      return a =  0;
    return a = a+b;
}

void tramage_floyd_steinbergBW( cv::Mat input, cv::Mat output ){
    int a = 0;
    int64 pE;
    for(int x = 0; x < output.rows; x++){
        for(int y = 0;y < output.cols; y++){

            pE = closest(output.at<uint8_t>(x,y));

            if(x != (output.rows-1) && y != 0 && y != (output.cols-1)){
                ovrflow(output.at<uint8_t>(x+0,y+1),(pE * 7) / 16);
                ovrflow(output.at<uint8_t>(x+1,y+1),(pE * 1) / 16);
                ovrflow(output.at<uint8_t>(x+1,y+0),(pE * 5) / 16);
                ovrflow(output.at<uint8_t>(x+1,y-1),(pE * 3) / 16);

            }
        }
    }
}

void tramage_floyd_steinbergCOL( cv::Mat input, cv::Mat output ){
    Mat Bands[3];
    split(input,Bands);
    for(int i = 0;i< 3;i++){
        tramage_floyd_steinbergBW( Bands[i], Bands[i] );
    }
    std::vector<Mat> channels = {Bands[0], Bands[1],Bands[2]};
    merge(channels,output);
}

void tramage_floyd_steinbergCMYK( cv::Mat input, cv::Mat output ){
    input.convertTo(output, CV_32FC3, 1.0/255.0);

    Vec3f pE;
    Vec3f arr[] = {Vec3f( {1.0, 1.0, 0.0}),Vec3f( {1.0, 0.0, 1.0}),Vec3f( {0.0, 1.0, 1.0}),Vec3f( {0.0, 0.0, 0.0}),Vec3f( {1.0, 1.0, 1.0})};
    std::vector<Vec3f> colors = std::vector<Vec3f>(arr,arr + 5);
    for(int x = 0; x < output.rows; x++){
        for(int y = 0;y < output.cols; y++){


            pE = closest(output.at<Vec3f>(x,y),colors);


            if(x != (output.rows-1) && y != 0 && y != (output.cols-1)){
                ovrflow(output.at<Vec3f>(x+0,y+1),(pE * (7.0 / 16.0)),colors);
                ovrflow(output.at<Vec3f>(x+1,y+1),(pE * (1.0 / 16.0)),colors);
                ovrflow(output.at<Vec3f>(x+1,y+0),(pE * (5.0 / 16.0)),colors);
                ovrflow(output.at<Vec3f>(x+1,y-1),(pE * (3.0 / 16.0)),colors);
            }
        }
    }
    output.convertTo(output, CV_8UC3, 255);
}



void tramage_floyd_steinberg( cv::Mat input, cv::Mat output ){
    if(input.channels() > 1)
        tramage_floyd_steinbergCOL(input, output);
    else
        tramage_floyd_steinbergBW(input, output);
}



int main(int argc,char** args)
{
  int old_value = 0;
  int value = 128;
  if(argc < 1){
      std::cout << "usage : minimum-program <imagePath>"<<"\n";
      std::cout << "" << (std::string) args[1];
      return 0;
  }
  std::string imagePath = (std::string) args[1];
  namedWindow( "Youpi",WINDOW_NORMAL);               // crée une fenêtre
  namedWindow( "trammage",WINDOW_NORMAL);
  namedWindow("trammage2",WINDOW_NORMAL);
  namedWindow( "imgCorr",WINDOW_NORMAL);
  createTrackbar( "track", "Youpi", nullptr, 255, NULL); // un slider
  setTrackbarPos( "track", "Youpi", value ); // met à 128
  Mat f = imread(imagePath);          // lit l'image "lena.png"
  std::cout << "AHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << "\n";
  Mat imgCorrCol;
  Mat fColor = f.clone();
  cvtColor(f,imgCorrCol,COLOR_BGR2HSV);
  if(f.channels() != 1)
    cvtColor(f,f, COLOR_RGB2GRAY);
              // l'affiche dans la fenêtre
  std::vector<double> hist;
  hist = histogramme(f);
  std::vector<double> histCum = histogramme_cumule(hist,f.rows*f.cols);
  Mat h = afficheHistogrammes(hist,histCum);
  namedWindow( "histogramme",WINDOW_NORMAL);
  imshow("histogramme",h);

  Mat fCorr = f.clone();
  int size=f.rows*f.cols;

  fCorr.forEach<Pixel>([histCum,f,size](Pixel& pixel, const int position[]) -> void {
       auto v = f.at<uchar>(position[0],position[1]);
       pixel = 255*(histCum[v]);
  });

  hist = histogramme(fCorr);
  histCum = histogramme_cumule(hist,f.rows*f.cols);
  Mat h2 = afficheHistogrammes(hist,histCum);
  namedWindow( "histogrammeCorr",WINDOW_NORMAL);
  imshow("histogrammeCorr",h2);

  Mat Bands[3];
  cvtColor(fColor,fColor,COLOR_BGR2HSV);
  split(fColor, Bands);
  std::vector<Mat> channels = {Bands[0],Bands[1],fCorr};
  merge(channels,imgCorrCol);
  cvtColor(fColor,fColor,COLOR_HSV2BGR);

  resize(f,f,Size(),0.5,0.5);
  resize(fCorr,fCorr,Size(),0.5,0.5);
  resize(fColor,fColor,Size(),0.5,0.5);
  resize(imgCorrCol,imgCorrCol,Size(),0.5,0.5);
  cvtColor(imgCorrCol,imgCorrCol,COLOR_HSV2BGR);

  Mat tram2 = Mat(fColor.rows,fColor.cols,CV_32FC3);
  tramage_floyd_steinbergCMYK(fColor,tram2);
  Mat tram = Mat(fColor.rows,fColor.cols,CV_8UC3);
  tramage_floyd_steinbergCOL(fColor,tram);

  imshow("trammage",tram);
  imshow("trammage2",tram2);
  imshow( "Youpi", fColor );
  imshow( "imgCorr", imgCorrCol );
  while ( waitKey(50) < 0 ){
      //value = getTrackbarPos( "track", "Youpi" );
      if ( value != old_value )
         {
           old_value = value;
           std::cout << "value=" << value << std::endl;
         }
  };          // attend une touche
  //value = getTrackbarPos( "track", "Youpi" ); // récupère la valeur
  std::cout << "value=" << value << std::endl;
}
