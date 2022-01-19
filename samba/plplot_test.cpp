
#include "plplot_test.hpp"
#include "wxPLplotwindow.h"

#include <cmath>

#define MAX( a, b )    ( ( a ) < ( b ) ? ( b ) : ( a ) )
#define MIN( a, b )    ( ( a ) < ( b ) ? ( a ) : ( b ) )

#ifndef ROUND
#define ROUND( a )    (PLINT) ( ( a ) < 0. ? ( ( a ) - 0.5 ) : ( ( a ) + 0.5 ) )
#endif


template< class WXWINDOW >
void Plot( wxPLplotwindow<WXWINDOW> *plotwindow );

class wxPlDemoFrame : public wxPLplotwindow<wxFrame>
{
public:
    wxPlDemoFrame( const wxString &title );
  void OnTimerUpdate(wxTimerEvent& /*event*/);
private:
    void OnIdle( wxIdleEvent &event );
    virtual void plot();
    bool m_plotted;
  wxTimer timer_;
  double val_;
  int count_;
};

wxPlDemoFrame::wxPlDemoFrame( const wxString &title )
{
    Create( NULL, wxID_ANY, title );
    //give our frame a nice icon
    Connect( wxEVT_IDLE, wxIdleEventHandler( wxPlDemoFrame::OnIdle ) );
    timer_.SetOwner(this);
    Connect( wxEVT_TIMER, wxTimerEventHandler( wxPlDemoFrame::OnTimerUpdate));
    m_plotted = false;
    timer_.Start(500);
    val_ = 1.0;
    count_ = 0;
}

void wxPlDemoFrame::OnTimerUpdate(wxTimerEvent& /*event*/)
{
  if (IsReady())
    {
      plot();
    }
}

void wxPlDemoFrame::OnIdle( wxIdleEvent &event )
{
    //We do our plotting in here, but only the first time it is called
    //This allows us to keep checking if the window is ready and only
    //plot if it is.
    /*if ( !m_plotted && IsReady() )
    {
        m_plotted = true;
        plot();
	}*/
}

void wxPlDemoFrame::plot()
{
    if ( !IsReady() )
    {
        wxMessageBox( wxT( "Somehow we attempted to plot before the wxPLplotwindow was ready. The plot will not be drawn." ) );
        return;
    }

    int        i;
    PLFLT      *freql = new PLFLT[101];
    PLFLT      *ampl  = new PLFLT[101];
    PLFLT      *phase = new PLFLT[101];
    PLFLT      f0, freq;
    PLINT      nlegend;
    const char *text[2], *symbols[2];
    PLINT      opt_array[2];
    PLINT      text_colors[2];
    PLINT      line_colors[2];
    PLINT      line_styles[2];
    PLFLT      line_widths[2];
    PLINT      symbol_numbers[2], symbol_colors[2];
    PLFLT      symbol_scales[2];
    PLFLT      legend_width, legend_height;        
    val_ += 1.0;
    if (val_ > 30.0) val_ = 1.0;
    
    wxPLplotstream* pls = GetStream();
        double p_def = 0, p_ht = 0;
    pls->gchr(p_def, p_ht);
    //std::cout << "1: " << p_def << "   " << p_ht << std::endl;

    //PLPLOT_wxLogDebug( "wxPlDemoFrame::plot()" );
    assert( pls );
    pls->adv( 1 );
    pls->clear();
    //pls->sdidev(0.1, PL_NOTSET, 0, 0);
    pls->fontld( 1 );
        pls->schr( 0.0, 2.0 );
    const size_t np = 500;
    PLFLT        x[np], y[np];
    PLFLT        xmin, xmax;
    PLFLT        ymin = 1e30, ymax = 1e-30;

    xmin = 0.0;
    xmax = 100.0;
    for ( size_t i = 0; i < np; i++ )
    {
        x[i] = ( xmax - xmin ) * i / np + xmin;
        y[i] = 1.0;
        y[i] = sin( x[i] ) * sin( x[i] / val_ );
        ymin = -1.05;
        ymax = -ymin;
    }
    pls->gchr(p_def, p_ht);
    //std::cout << "2:  " <<p_def << "   " << p_ht << std::endl;
    
        pls->gchr(p_def, p_ht);
	//std::cout << "3:  " <<p_def << "   " << p_ht << std::endl;
    f0 = 1.0;
    for ( i = 0; i <= 100; i++ )
    {
        freql[i] = -2.0 + i / 20.0;
	freq     = pow( 10.0, freql[i] );
        ampl[i]  = 20.0 * log10( 1.0 / sqrt( 1.0 + pow( ( freq / f0 ), 2. ) ) );
        phase[i] = -( 180.0 / M_PI ) * atan( (PLFLT) ( freq / f0 ) );
    }

    //pls->vpor( 0.15, 0.85, 0.1, 0.9 );
    pls->vpor( 0.1, 0.95, 0.22, 0.95 );
        pls->gchr(p_def, p_ht);
	//std::cout << "4:  " <<p_def << "   " << p_ht << std::endl;
    pls->wind( xmin, xmax, ymin, ymax );
    //pls->sdiplt(-0.2, -0.2, 1.2, 1.2);
        pls->gchr(p_def, p_ht);
	//std::cout << "5:  " <<p_def << "   " << p_ht << std::endl;
    pls->col0( 15 );
    pls->box( "bnst", 0, 0, "bnst", 0.0, 0 );

    pls->gchr(p_def, p_ht);
    //std::cout << "6:  " <<p_def << "   " << p_ht << std::endl;

        pls->col0( 3 );
    
    pls->line( np, x, y );
    //pls->col0( 2 );
    //pls->ptex( 1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade" );

    // Put labels on.
    //     pls->schr( 0.0, 2.5 );
        pls->col0( 15 );
    pls->lab( "x", "y", "sin(x)/x" ); 
    pls->col0( 1 );
    //pls->mtex( "b", 3.2, 0.5, 0.5, "Frequency" );
    //pls->mtex( "t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter" );
    //pls->col0( 2 );
    //pls->mtex( "l", 5.0, 0.5, 0.5, "Amplitude (dB)" );



    RenewPlot();

    if (count_ < -10)
      {
	timer_.Stop();
      }
    else
      {
	count_ += 1;
      }
}


//! This method is called right at the beginning and opens a frame for us.
//
bool PLPlotApp::OnInit()
{
    //*******The first method using the child class defined above******
    wxPlDemoFrame *inheritedFrame = new wxPlDemoFrame( wxT( "wxPLplotDemo - Interactive Frame With Inheritance" ) );
    inheritedFrame->Show();

    return true;
}

