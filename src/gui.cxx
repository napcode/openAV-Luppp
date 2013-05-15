
#include "gui.hxx"
#include "avtk/avtk_image.h"

#include <sstream>

// Hack, move to gtrack.cpp
int GTrack::privateID = 0;

using namespace std;

Gui::Gui()
{
  window = new Fl_Double_Window(650,280);
  window->color(FL_BLACK);
  window->label("Luppp 5");
  
  
  Avtk::Image* header = new Avtk::Image(0,0,600,36,"header.png");
  
  for (int i = 0; i < 5; i++ )
  {
    stringstream s;
    s << "Track " << i+1;
    tracks.push_back( new GTrack(8 + i * 118, 40, 110, 230, s.str().c_str() ) );
  }
  
  box = new Fl_Box(655, 5, 200, 60, "BPM = 120");
  box->box(FL_UP_BOX);
  box->labelsize(36);
  box->labeltype(FL_SHADOW_LABEL);
  
  window->end();
}

int Gui::show()
{
  window->show();
  return Fl::run();
}
