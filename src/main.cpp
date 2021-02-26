#include "include/gui.hpp"

#define LOG_IMPL
#include "include/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char* argv[])
{
    RssView r;
    r.init();
    r.doLoop();
    return 0;
}
