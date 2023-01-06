#include <cstdio>
#include <iostream>
#include <algorithm>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
//                              //
    //  COLOR DISTRIBUTION  //
//                              //
struct ColorDistribution {
  float data[ 8 ][ 8 ][ 8 ]; // l'histogramme
  int nb;                     // le nombre d'échantillons

  ColorDistribution() { reset(); }
  ColorDistribution& operator=( const ColorDistribution& other ) = default;
  // Met à zéro l'histogramme
  void reset();
  // Ajoute l'échantillon color à l'histogramme:
  // met +1 dans la bonne case de l'histogramme et augmente le nb d'échantillons
  void add( Vec3b color );
  // Indique qu'on a fini de mettre les échantillons:
  // divise chaque valeur du tableau par le nombre d'échantillons
  // pour que case représente la proportion des picels qui ont cette couleur.
  void finished();
  // Retourne la distance entre cet histogramme et l'histogramme other
  float distance( const ColorDistribution& other ) const;
};

void ColorDistribution::reset(){
    this->nb = 0;
    for(int i = 0; i < 8;i++)
        for(int j = 0; j < 8;j++)
            for(int k = 0; k < 8;k++)
                this->data[i][j][k] = 0;
}

void ColorDistribution::finished(){
    for(int i = 0; i < 8;i++)
        for(int j = 0; j < 8;j++)
            for(int k = 0; k < 8;k++)
                this->data[i][j][k] /= this->nb;
}

void mapCol(Vec3b& c,int i = 8,int j = -1,int k = -1){
    if(j == -1) j = i;
    if(k == -1) k = j;

    i = 256/i; j = 256/j; k = 256/k;

    c[0] /= i; c[1] /= j; c[2] /= k;
}

void ColorDistribution::add( Vec3b color ){
    mapCol(color);
    this->nb++;
    this->data[color[0]][color[1]][color[2]] += 1.0f;
}

float ColorDistribution::distance(const ColorDistribution& h2) const
{
    float retval = 0.0f;
    for(int i = 0; i < 8;i++)
        for(int j = 0; j < 8;j++)
            for(int k = 0; k < 8;k++){
                if((this->data[i][j][k] + h2.data[i][j][k]) != 0)
                    retval +=(
                    ((this->data[i][j][k] - h2.data[i][j][k])*(this->data[i][j][k] - h2.data[i][j][k]))
                    /
                    (this->data[i][j][k] + h2.data[i][j][k])
                             );
            }
    return retval;
}


ColorDistribution
getColorDistribution( Mat input, Point pt1, Point pt2 )
{
  ColorDistribution cd;
  for ( int y = pt1.y; y < pt2.y; y++ )
    for ( int x = pt1.x; x < pt2.x; x++ )
      cd.add( input.at<Vec3b>( y, x ) );
  cd.finished();
  return cd;
}

ColorDistribution
getColorDistribution( Mat input)
{
    Point pt1 = Point(0,0);
    Point pt2 = Point(input.rows-1,input.cols-1);
    return getColorDistribution(input, pt1, pt2);
}

std::vector<ColorDistribution> col_hists; // histogrammes du fond
std::vector<ColorDistribution> col_hists_object; // histogrammes de l'objet

void setBackgroundHists(Mat& in){
    int height = in.rows;
    int width  = in.cols;

    const int bbloc = 128;
    for (int y=0; y <= height-bbloc; y += bbloc )
      for (int x=0; x <= width-bbloc; x += bbloc )
        {
          ColorDistribution hist = getColorDistribution(in,Point(x,y),Point(x+bbloc,y+bbloc));
          col_hists.push_back(hist);
        }
    int nb_hists_background = col_hists.size();
}

bool debug = false;

const Vec3b black(0,0,0),red(0,0,255),blue(255,0,0);
std::vector<Vec3b> colors(2);
const int size  = 50;
const int width = 640;
const int height= 480;
Point pt1( width/2-size/2, height/2-size/2 );
Point pt2( width/2+size/2, height/2+size/2 );

void setObjHist(Mat& in){
    ColorDistribution hist = getColorDistribution(in,pt1,pt2);
    col_hists_object.push_back(hist);
}

float minDistance( const ColorDistribution& h,
                   const std::vector< ColorDistribution >& hists )
{
        float min = 10000;
        float dist;
        int i = 0;
        int minI = 0;

        for(auto hist : hists){
            i++;
            dist = h.distance(hist);
            if(dist < min){
                min = dist;
                minI = i;
            }
        }

        return dist;
}

int objetPlusProche(Mat in,
                    const std::vector< ColorDistribution >& col_hists,
                    const std::vector< ColorDistribution >& col_hists_object,
                    const std::vector< Vec3b >& colors,
                    Point p1,Point p2)
{
    int object = 0;
    ColorDistribution hB = getColorDistribution(in,p1,p2);

    int minhist = minDistance(hB,col_hists);
    int minobj = minDistance(hB,col_hists_object);

    if(minobj < minhist){
        object = 1;
    }

    return object;
}

Mat recoObject( Mat in,
                const std::vector< ColorDistribution >& col_hists,
                const std::vector< ColorDistribution >& col_hists_object,
                const std::vector< Vec3b >& colors,
                const int bloc )
{
    Mat out = Mat(in.rows,in.cols,CV_8UC3,Vec3b(0,0,0));

    for (int y=0; y <= height-bloc; y += bloc )
      for (int x=0; x <= width-bloc; x += bloc )
        {
            Point a = Point(x,y);
            Point b = Point(x+bloc,y+bloc);

            int blocInObject = objetPlusProche(in,col_hists,col_hists_object,colors,a,b); // 0 fond // 1 objet
            cv::rectangle( out, a, b, colors[blocInObject], -1 );
        }

    if(debug)
      imshow( "input", out );

    return out;
}

int main( int argc, char** argv )
{
  Mat img_input, img_seg, img_d_bgr, img_d_hsv, img_d_lab;
  VideoCapture* pCap = nullptr;

  bool save = false;

  bool reco = false;
  colors[0] = black;
  colors[1] = red;

  // Ouvre la camera
  pCap = new VideoCapture( 0 );
  if( ! pCap->isOpened() ) {
    cout << "Couldn't open image / camera ";
    return 1;
  }
  // Force une camera 640x480 (pas trop grande).
  pCap->set( CAP_PROP_FRAME_WIDTH, 640 );
  pCap->set( CAP_PROP_FRAME_HEIGHT, 480 );
  (*pCap) >> img_input;
  if( img_input.empty() ) return 1; // probleme avec la camera

  namedWindow( "input", 1 );
  imshow( "input", img_input );
  bool freeze = false;
  float dist = 0.0f;
  while ( true )
    {
      char c = (char)waitKey(50); // attend 50ms -> 20 images/s
      if ( pCap != nullptr && ! freeze )
        (*pCap) >> img_input;     // récupère l'image de la caméra
      if ( c == 27 || c == 'q' )  // permet de quitter l'application
        break;
      if ( c == 'f' ) // permet de geler l'image
        freeze = ! freeze;
      if( c == 'b')
          setBackgroundHists(img_input);
      if( c == 'a'){
        setObjHist(img_input);
      }
      if( c == 'd'){
          debug = !debug;
      }
      if(c == 'r')
          reco = true;
      if(c == 't')
          reco = false;
      if(c == 's')
          save = true;
      if( c == 'v'){
        Point middleP = Point(img_input.rows/2,img_input.cols/2);
        Point endP = Point(img_input.rows-1,img_input.cols-1);
        ColorDistribution h1 = getColorDistribution(img_input,Point(0,0),middleP);
        ColorDistribution h2 = getColorDistribution(img_input,middleP,endP);
        dist = h1.distance(h2);
        cout<<dist;
      }

      Mat output = img_input;
      if ( reco )
      { // mode reconnaissance
        Mat gray;
        Mat reco = recoObject( img_input, col_hists, col_hists_object, colors, 16 );
 /*       cvtColor(img_input, gray, COLOR_BGR2GRAY);

        cvtColor(gray, img_input, COLOR_GRAY2BGR);*/
        output = 0.5 * reco + 0.5 * img_input; // mélange reco + caméra
      }
      else
        cv::rectangle( img_input, pt1, pt2, Scalar( { 255.0, 255.0, 255.0 } ), 1 );

      cv::putText(img_input, //target image
                  std::to_string(dist) , //text
                  cv::Point(10, 20), //top-left position
                  cv::FONT_HERSHEY_DUPLEX,
                  1.0,
                  CV_RGB(118, 185, 0), //font color
                  2);

      if(!debug)
        imshow( "input", output );
      if(save){
        save = false;
        imwrite("detection.png",output);
      }

      //imshow( "input", img_input ); // affiche le flux video
    }
  return 0;
}
