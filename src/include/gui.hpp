#pragma once

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "glad/glad.h"
#include <SDL.h>

#ifdef _WIN32
#include "windows.h"
#endif

#include <future> //For asynchronous processes not freezing the GUI
#include <chrono>

#include "rss.hpp"

/**
 * @brief Class that contains all methods for displaying RSS management
 * and viewing GUI
 * 
 */
class RssView
{
public:

    /**
     * @brief Method to start SDL2 and OpenGL, then start Dear ImGui
     * for those backends
     * 
     * @param w The optional width of the screen, defaults to 1280
     * @param h The optional height of the screen, defaults to 720
     */
    void init(unsigned int w = 1280, unsigned int h = 720);

    ~RssView(); //Destructor to cleanup all SDL2, OpenGL, Dear ImGui things

    /**
     * @brief Method to start the event and render loop, returns when 
     * the user exits the program by pressing the X on the window or Quit option in 
     * the menubar
     * 
     */
    void doLoop(void); 

private:

    unsigned int screenWidth; //Screen dimensions
    unsigned int screenHeight;

    SDL_Window* win = NULL; //The SDL2 window error
    SDL_GLContext glContext; //The SDL2 OpenGL context object

    RssFeedManager feedManager; //The internal RSS feed manager object 
    size_t displayedFeed;       //The feed that is displayed in the channel view panel

    size_t maxImageWidth = 200; //The maximum an image width can be

    /**
     * @brief Method to display a window with list of all subscribed RSS channel titles
     * 
     */
    void feedSelectWin(void);

    /**
     * @brief Method to display the currently selected channel's 
     * items in a window
     * 
     */
    void displayChannel(void);

    std::future<void> bgProcess; //Background process to run asynchronously
    std::string processString;   //The string describing what the background process is doing
};
