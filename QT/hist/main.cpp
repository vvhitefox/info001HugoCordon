// minimum-prog.cpp
#include <iostream>
#include <opencv2/highgui.hpp>
using namespace cv;
int main()
{
  int value = 128;
  namedWindow( "Youpi");               // crée une fenêtre
  createTrackbar( "track", "Youpi", nullptr, 255, NULL); // un slider
  setTrackbarPos( "track", "Youpi", value ); // met à 128
  Mat f = imread("lena.png");          // lit l'image "lena.png"
  imshow( "Youpi", f );                // l'affiche dans la fenêtre
  while ( waitKey(50) < 0 ) ;          // attend une touche
  value = getTrackbarPos( "track", "Youpi" ); // récupère la valeur
  std::cout << "value=" << value << std::endl;
}
