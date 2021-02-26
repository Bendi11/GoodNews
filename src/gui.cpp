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

void RssView::doLoop(void)
{
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
 
        if(ImGui::BeginMenu("Options")) //Begin the options bar
        {
            if(ImGui::MenuItem("Quit", "Alt+f4")) //Prompt the user to quit the application
            {
                run = false; //Quit the program
            }

            ImGui::EndMenu(); //Stop drawing to the menu
        }

        ImGui::EndMainMenuBar();

        

        ImGui::Render();
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y); //Set the OpenGL rendering size to the window size
        glClearColor(0.5, 0.5, 0.52, 1.0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(win); //Draw the double buffered frame
    }
}
