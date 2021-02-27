#include "include/gui.hpp"

void RssView::init(unsigned int w, unsigned int h)
{   
    screenWidth = w;
    screenHeight = h;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) //Attempt to start SDL2 and check for any errors
    {
        logE("Failed to start SDL2! Error: %s", SDL_GetError()); //Log the error
        exit(-1); //Quit the program
    }

    const char* glslVersion = "#version 130"; //The string specifying version at the top of GLSL files
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); //Set the openGL profile to core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //Enable double buffering, so we can draw to a frame without it being drawn onscreen
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);    

    //Create the SDL2 window and check for any errors
    win = SDL_CreateWindow("Good News", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if(win == NULL) //Check if the SDL2 window creation failed
    {
        logE("Failed to create SDL2 window! Error: %s", SDL_GetError()); //Log the error
        exit(-1); //Quit the program
    }

    glContext = SDL_GL_CreateContext(win); //Create the SDL2 OpenGL context
    SDL_GL_MakeCurrent(win, glContext); 

    logI("Finished starting SDL2"); //Log the information that we finished loading SDL2 things
    
    if(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0) //Load all OpenGL plugins and check for errors
    {
        logE("Failed to load OpenGL plugins! Error %d", glad_glGetError()); //Get Error and log it
        exit(-1);
    }
    logI("Finished loading OpenGL extensions"); //Log that we finished loading extensions

    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); 
    ImGui_ImplSDL2_InitForOpenGL(win, glContext); //Init Dear ImGui for SDL2 and OpenGL
    logI("Dear ImGui init for SDL2");
    ImGui_ImplOpenGL3_Init(glslVersion);          //Init OpenGL3 rendering backend for Dear ImGui
    logI("Dear ImGui OpenGL 3 rendering backend started");

    ImGuiIO& io = ImGui::GetIO();
    normal = io.Fonts->AddFontFromFileTTF("times-new-roman.ttf", 18.f);
    //bold = io.Fonts->AddFontFromFileTTF("FreeSansBold-Xgdd.ttf", 24);

    bgProcess = std::async(std::launch::async, &RssFeedManager::loadChannelsFromRecord, &feedManager); //Load all RSS feeds in the background
    processString = "Loading RSS channels...";


}

RssView::~RssView()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown(); //Shutdown Dear ImGui
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    logI("OpenGL and SDL2 clean shutdown");
    SDL_Quit();
}

void RssView::feedSelectWin(void)
{
    ImVec2 paneSize = ImVec2(ImGui::GetIO().DisplaySize.x / 4, ImGui::GetIO().DisplaySize.y);
    ImGui::SetNextWindowSize(paneSize); //Set the size to 1 / 4 of the screen size
    ImGui::SetNextWindowPos(ImVec2(0, 0 + mainMenuSize.y) ); //Display just under the main menu bar
    if(!ImGui::Begin("Select RSS Channel", (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize)) //Display the selection window
    return;

    size_t idx = 0; //Enumerated index of each channel to select one based on index

    ImGui::Text("RSS Channels");
    ImGui::ListBoxHeader("", ImVec2(paneSize.x, paneSize.y * (3 / 4))); //Start drawing to a new listbox of RSS channels
    for(auto& ch : feedManager.channels)
    {
        if(ImGui::Selectable(ch.title.c_str())) //If the user selects this RSS channel, display it
        {
            displayedFeed = idx;
        }
        idx++;
    }
    ImGui::ListBoxFooter();

    if(ImGui::Button("Remove selected RSS feed")) //If the user wants to delete this subscription
    {
        if(displayedFeed < feedManager.channels.size()) //Only remove the channel if it is valid
            feedManager.removeChannel(feedManager.channels[displayedFeed].title); //Remove the channel with the specified index
    }

    ImGui::Spacing();

    
    static std::string rssUrl; //The URL to load the RSS feed from
    ImGui::Text("RSS Feed URL: ");
    ImGui::InputText("", &rssUrl); //Prompt the user to enter a URL 
    if(ImGui::Button("Add RSS Feed"))
    {
        if(bgProcess.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) //If the background process is done, launch a new one
        {
            bgProcess = std::async(std::launch::async, &RssFeedManager::addChannel, &feedManager, rssUrl); //Add the channel to our list of feeds
            processString = "Adding RSS Feed From " + rssUrl; //Set the process string to explain what the user is waiting for
            rssUrl.clear(); //Empty the URL field
        }
    }


    ImGui::End();
}

void RssView::displayChannel(void)
{
    if(displayedFeed >= feedManager.channels.size()) return; //Don't display anything if the index is invalid
    RssChannel& displayed = feedManager.channels[displayedFeed]; //Get a reference to the displayed channel 

    ImVec2 paneSize = ImVec2(ImGui::GetIO().DisplaySize.x * 3.f/4.f, ImGui::GetIO().DisplaySize.y); //Size of this pane

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 4, 0 + mainMenuSize.y)); //Display just below the main menu bar
    ImGui::SetNextWindowSize(paneSize);
    ImGui::Begin(displayed.title.c_str(), (bool*)0, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize); //Begin drawing to a window with the name of the RSS channel

    maxImageWidth = (size_t)( (int)paneSize.x / 3); //Set images to be 1 / 3 the size of the window

    /*if(displayed.image.filled)
    {
        ImGui::Image((void*)(intptr_t)displayed.image.txID, ImVec2(maxImageWidth, ((float)displayed.image.height / (float)displayed.image.width) * maxImageWidth));
    }*/

    ImGui::TextColored(ImVec4(0.1f, 0.1f, 1.0f, 1.0f), "Link: %s", displayed.link.c_str()); //Display the link to go to if the user wants to know more
    if(ImGui::IsItemClicked()) //Check if the link was clicked and open a browser to seach for the link
    {
        #ifdef _WIN32
        ShellExecuteA(NULL, "open", displayed.link.c_str(), NULL, NULL, SW_SHOWNORMAL);
        #endif
    }

    ImGui::TextWrapped("Channel Description: %s", displayed.description.c_str());

    ImGui::Separator(); //Sepatate the channel attributes and the items

    size_t idx = 0;
    for(RssItem& item : displayed.items) //Display every item in the channel
    {
        ImGui::TextColored(ImVec4(0.97f, 0.76f, 0.01f, 1.0f), "Title: %s", item.title.c_str()); //Draw the title of the item
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Link: %s", item.link.c_str());      //Draw the link of the item
        if(ImGui::IsItemClicked()) //Check if the link was clicked and open a browser to view it
        {
            #ifdef _WIN32
            ShellExecuteA(NULL, "open", item.link.c_str(), NULL, NULL, SW_SHOWNORMAL);
            #endif
        }

        ImGui::TextWrapped("Description: %s", item.description.c_str());
        if(item.enclosure.filled) //If the image is filled with data, draw it
        {
            ImGui::Image((void *)(intptr_t)item.enclosure.txID, ImVec2((float)maxImageWidth, ((float)item.enclosure.height / (float)item.enclosure.width) * maxImageWidth)); //Draw the image
            ImGui::TextWrapped("Description: %s", item.enclosure.description.c_str()); //Draw the description of the image
        }
        else if(!item.enclosure.url.empty()) //If there is a url to download image data from, prompt the user to download it
        {
            if(ImGui::Button( ("Download Image #" + std::to_string(idx) ).c_str())) //Prompt the user to download the image
            {
                item.enclosure.loadImgFromUrl(item.enclosure.url); //Load the image at the URL
            }
        }
        idx++;
        ImGui::Separator();
        ImGui::Spacing();
    }

    ImGui::End();
}


void RssView::doLoop(void)
{
    //if(!bgProcess.valid())
        //bgProcess = std::async(unused);

    bool run = true; //If we should continue in the rendering loop
    SDL_Event userInput; //SDL input event queue to send to Dear ImGui
    
    while(run) //Start main loop
    {
        while(SDL_PollEvent(&userInput)) //Poll through all input events in SDL2
        {
            ImGui_ImplSDL2_ProcessEvent(&userInput); //Send the event to Dear ImGui
            if(userInput.type == SDL_QUIT || (userInput.type == SDL_WINDOWEVENT_CLOSE && userInput.window.windowID == SDL_GetWindowID(win))) //Quit if the user wants to
            run = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(win);
        ImGui::NewFrame();

        ImGui::BeginMainMenuBar(); //Start the main menu bar
        mainMenuSize = ImGui::GetWindowSize(); //Get how large the main menu bar is being drawn
 
        if(ImGui::BeginMenu("Options")) //Begin the options bar
        {
            if(ImGui::MenuItem("Quit", "Alt+f4")) //Prompt the user to quit the application
            {
                run = false; //Quit the program
            }
            if(ImGui::MenuItem("Settings"))
            {
                bShowSettings = (bShowSettings) ? false : true; //Toggle the value of 'show settings'
            }

            ImGui::EndMenu(); //Stop drawing to the menu
        }

        if(bgProcess.wait_for(std::chrono::seconds(0)) != std::future_status::ready) //If a background process is running, display what it is doing
        {
            ImGui::Text("Background Process: %s", processString.c_str());
        }

        ImGui::EndMainMenuBar();

        if(bShowSettings)
        {
            ImGui::Begin("Settings", &bShowSettings); //Show settings window if the user wants to edit settings
            ImGui::Checkbox("Load all images when loading a new RSS feed", &bLoadAllImages); //Allow the user to toggle if we should load every image when loading a new feed
            ImGui::End();
        }
        

        feedSelectWin();
        displayChannel();

        ImGui::Render();
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y); //Set the OpenGL rendering size to the window size
        glClearColor(0.5f, 0.5f, 0.52f, 1.0f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(win); //Draw the double buffered frame
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
