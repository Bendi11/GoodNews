
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/wxhtml.h>
#include <wx/fs_inet.h>

/**
 * @brief Class that wxWidgets implements, opens a wxFrame to draw GUI to
 * 
 */
class RssApp : public wxApp
{
public:
    virtual bool OnInit(); 

private:

};

/**
 * @brief Class that implements all GUI elements
 * 
 */
class RssFrame : public wxFrame
{
public:
    RssFrame(void);

private:

    void OnExit(wxCommandEvent& e);
    
};
