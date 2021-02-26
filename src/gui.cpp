#include "include/gui.hpp"
#include <wx/fs_mem.h>
#include <wx/webview.h>

bool RssApp::OnInit()
{ 
    wxInitAllImageHandlers();
    wxImage::AddHandler(new wxJPEGHandler);
    
    wxFileSystem::AddHandler(new wxMemoryFSHandler);
    wxFileSystem::AddHandler(new wxInternetFSHandler);
    RssFrame* frame = new RssFrame(); //Create a new Rss View window
    frame->Show(true); //Show the window
    return true;
}

RssFrame::RssFrame() : wxFrame(NULL, wxID_ANY, "Good News")
{
    wxFlexGridSizer* sizer = new wxFlexGridSizer(2, 2, wxSize(20, 10));

    //wxWebView* wv = wxWebView::New(this, wxID_ANY, wxString::FromAscii(wxWebViewDefaultURLStr), wxDefaultPosition, wxSize(640, 360));
    //wv->SetPage("<p><a href=\"https://mars.nasa.gov/news/8870/\"> <img src=\"https://mars.nasa.gov/system/news_items/main_images/8870_1_-_PIA24422_-_Navcam_360_-_Maki_7_Navcam_360_08_N_LRGB_0002_RAS_0010052_CYL_S_UNCORCLJ01-stretched-v2.jpg\" style=\"padding-right:10px; padding-bottom:5px;\" align=\"left\" alt=\"Read article: NASA's Mars Perseverance Rover Provides Front-Row Seat to Landing, First Audio Recording of Red Planet \" width=\"100\" height=\"75\" border=\"0\" /></a><br /><br />\"The agency’s newest rover captured first-of-its kind footage of its Feb. 18 touchdown and has recorded audio of Martian wind.\"t</p><br clear=\"all\"/><br />", "");
    
    //sizer->Add(wv);

    wxButton* b = new wxButton(this, wxID_ANY, "Test Button");
    sizer->Add(b);

    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT);
    
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    
    SetSizer(sizer);
    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &RssFrame::OnExit, this, wxID_EXIT);
    this->SetSize(sizer->GetMinSize());
    this->SetMinSize(sizer->GetMinSize());
}

void RssFrame::OnExit(wxCommandEvent& e)
{
    Close(true);
}
