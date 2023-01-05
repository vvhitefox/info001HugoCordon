// minimum-prog.cpp
#include <iostream>
#include <opencv2/highgui.hpp>
using namespace cv;
int main(int argc,char** args)
{
  int value = 128;
  if(argc < 1){
      std::cout << "usage : minimum-program <imagePath>"<<"\n";
      std::cout << "" << (std::string) args[1];
      return 0;
  }
  std::string imagePath = (std::string) args[1];
  namedWindow( "Youpi",WINDOW_NORMAL);               // crée une fenêtre
  createTrackbar( "track", "Youpi", nullptr, 255, NULL); // un slider
  setTrackbarPos( "track", "Youpi", value ); // met à 128
  Mat f = imread("lena.png");          // lit l'image "lena.png"
  std::cout << "AHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << "\n";
  imshow( "Youpi", f );                // l'affiche dans la fenêtre
  while ( waitKey(50) < 0 ) ;          // attend une touche
  value = getTrackbarPos( "track", "Youpi" ); // récupère la valeur
  std::cout << "value=" << value << std::endl;
}
