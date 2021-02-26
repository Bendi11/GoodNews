#pragma once

#include "logger.hpp"

#include <string>
#include <fstream> //For record file reading
#include <vector>
#include <list>
#include <exception>
#include <chrono> //For timestamps since last checked an RSS channel

#include "pugixml.hpp"
#include "cpr/cpr.h"

/**
 * @brief Rss Image class, used to encapsulate all data about an image in
 * a channel, item, etc.
 * 
 */
struct RssImage
{
    bool filled = false; //If the image data is filled in or this is empty

    std::string title; //Required title of the image
    std::string url; //Required URL to download image from

    std::string description; //Optional description of image contents

    int width = 88; //Optional w of image, default is 88 px
    int height = 31; //Option h of image, default is 31 px
    int ch = 3;

    /**
     * @brief Method to construct an RssImage from an XML image 
     * specification
     * 
     * @param xmlNode The <image> </image> pugixml node to get the image data from
     * @return RssImage The constructed RssImage 
     * @throw std::runtime_error if a required attribute is not found/image couldn't be loaded
     */
    static RssImage fromXML(const pugi::xml_node& xmlNode);


    /**
     * @brief Method to construct an RSS image from an RSS item's <enclosure> child
     * 
     * @param xmlNode The <enclosure> child node of the <item> node
     * @return RssImage when all required fields are met
     */
    static RssImage fromXMLEnclosure(const pugi::xml_node& xmlNode); 


    /**
     * @brief Method to load image data from a given url explicitly,
     * used to give people with slow connections an option to not
     * download images
     * 
     * @param t_url The url to download from
     * @throw std::runtime_error if the request failed / image failed to load
     */
    void loadImgFromUrl(const std::string t_url); 

    
};


/**
 * @brief RSS item class, with all attributes possibly given for
 * an RSS item
 * 
 */
struct RssItem
{
    std::string title; //Required title of the item
    std::string description; //Required description of the item 
    std::string link; //Required link to the item contents

    std::string pubDate; //Optional The last publication date of the item
    std::string author;  //Optional author of this RSS item
    RssImage enclosure; //Optional media file included in item

    /**
     * @brief Method to construct an RSS item from an xml <item> node
     * 
     * @param xmlNode The read only xml node to read all item data into
     * @return RssItem The constructed RSS item object
     * @throw std::runtime_error when a required field is missing
     */
    static RssItem fromXML(const pugi::xml_node& xmlNode); 
};

/**
 * @brief Rss channel class for storing all attributes of one RSS channel
 * 
 */
struct RssChannel //One Rss subscribed channel
{
    std::string title; //The title of the channel, required in all RSS channels
    std::string description; //Description of channel, required
    std::string link; //Required hyperlink to the RSS channel

    size_t ttl; //Time to live, number of minutes until a refresh of the feed is needed
    size_t lastChecked = 0; //Not part of the RSS channel, but helpful to record when this channel was downloaded for ttl caching; ms since 1970 this was checked at

    RssImage image; //Optional image to go with channel
    std::vector<RssItem> items; //Required list of all attached items 

    /**
     * @brief Method to construct an Rss Channel from a pugixml node
     * 
     * @param xmlNode The XML node that this RSS feed should be constructed from
     * @param link The link that this RSS feed originated from
     * @return RssChannel object that was constructed from the XML
     * @throw std::runtime_error when xml construction fails
     */
    static RssChannel fromXML(const pugi::xml_document& xmlDoc, const std::string link); 

    /**
     * @brief Method to download an RSS feed from a URL and 
     * parse the feed to a channel object
     * 
     * @param url The URL to load the RSS feed from
     * @return RssChannel The constructed RSS channel 
     * @throw std::runtime_error if GET request, XML parsing, or RSS construction fails
     */
    static RssChannel fromUrl(const std::string url);

};


/**
 * @brief Class to manage a collection of RSS channels,
 * recording their ttls, saving the last loaded time to see if we need to
 * redownload the feed, caching the feed in a file, etc.
 * 
 */
class RssFeedManager
{
public:

    /**
     * @brief Method to add a channel to the list of subscribed channels, 
     * downloading from a URL
     * 
     * @param link The link to download the RSS feed from
     * @throw the error message if the operation fails
     */
    void addChannel(const std::string link); 

    /**
     * @brief Method to remove a channel from the list of subscribed channels,
     * also removing it from the record file
     * 
     * @param title The title of the feed to remove
     */
    void removeChannel(const std::string title); 

    RssFeedManager(void);
    ~RssFeedManager();

    std::list<RssChannel> channels; //List of all subscribed channels

    /**
     * @brief Method to use subscribed.txt file to load all RSS feeds, either
     * by downloading them if their ttl is not given or lower than last checked,
     * or using cached RSS files if the feed hasn't refreshed yet
     * 
     */
    void loadChannelsFromRecord(void);
private:

    std::fstream recordFile;        //Record of subscribed channels, their ttls and last checked times

    /**
     * @brief Method called in the destructor for RssFeedManager class
     * writes all last checked dates to the record file, plus all ttls, titles,
     * and urls of subscribed RSS feeds
     * 
     */
    void writeRecord(void);    //Function to write the last downloaded date to the record file
};
