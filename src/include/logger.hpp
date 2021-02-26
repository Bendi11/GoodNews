#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdio.h> //For C type FILE objects

/**
 * @brief Function to get current source file name without path
 * 
 * @param path Use __FILE__ to get current source file name
 */
const char* file_name(const char* path);


/**
 * @brief log error macro, writes a fatal error flag with timestamp, message, and file that
 * the error originates from
 * 
 * @param fmt The format string
 * @param ... The variadic format attributes
 */
#define logE(fmt, ...) __logtofile("ERROR", file_name(__FILE__), fmt, __VA_ARGS__)


/**
 * @brief log warning macro, writes warning flag to log file with timestamp, message, and file the warning originates from
 * 
 * @param fmt The format string to write, same syntax as printf
 * @param ... The variadic format data to write
 */
#define logW(fmt, ...) __logtofile("WARNING", file_name(__FILE__), fmt, __VA_ARGS__)

/**
 * @brief log information macro, writes an information flag with timestamp, message, and file that information
 * comes from
 * 
 * @param fmt The format string to write information, same syntax as printf for formatting
 * @param ... The variadic format data to write
 */
#define logI(fmt, ...) __logtofile("INFO", file_name(__FILE__), fmt, __VA_ARGS__)

/**
 * @brief Internal method to print to log file with a prefix and file name,
 * used by logging macros 
 * 
 * @param prefix ERROR, INFO, etc.
 * @param fName Recommended to use __FILENAME__ macro to get file name without path
 * @param fmt The format string to print as the information
 * @param ... The format data
 */
void __logtofile(const char* prefix, const char* fName, const char* fmt, ...);

#endif //LOGGER_HPP

#ifdef LOG_IMPL

#include <ctime> 
#include <iomanip>
#include <stdarg.h>

static std::time_t currentTime = std::time(nullptr); //The current time used to print time info to file

FILE* m_logFile = fopen("log.txt", "w"); //The log file object used to print information to

void __logtofile(const char* prefix, const char* fName, const char* fmt, ...)
{
    currentTime = std::time(nullptr); //Get the current time
    va_list args; //The list of variadic function arguments
    va_start(args, fmt);

    char time[100]; //String holding stringified time
    std::strftime(time, 100, "%y-%m-%d %OH:%OM:%OS", std::localtime(&currentTime)); //Write string time to the character buffer

    fprintf(m_logFile, "%s", time); //Print time information
    fprintf(m_logFile, " [%s] ", prefix); //Print that the prefix given
    fprintf(m_logFile, "(%s): ", fName); //Log the file name that is emitting the info
    vfprintf(m_logFile, fmt, args); //Print the format string given to function
    fprintf(m_logFile, "\n");

    fflush(m_logFile);
}

#ifdef _WIN32 // '\' style directory separator in file paths
#define PATH_SEPARATOR '\\'
#else         // '/' style directory separator in file paths
#define PATH_SEPARATOR '/'
#endif

const char* file_name(const char* path) 
{
    const char* file = path;
    while (*path) 
    {
        if (*path++ == PATH_SEPARATOR) 
        {
            file = path;
        }
    }
    return file;
}

#endif