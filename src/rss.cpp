#include "include/rss.hpp"

#include "stb_image.h"

/**
 * @brief Function to require an XML node to exist and return its value
 * 
 * @param xmlNode The pugixml XML node object
 * @param name The name of the XML node required to exist
 * @return const pugi::xml_node& The XML node that was found
 * @throw std::runtime_error if the element doesn't exist
 */
inline const pugi::xml_node REQUIRENODE(const pugi::xml_node& xmlNode, std::string name)
{
    if(!xmlNode.child(name.c_str()) ) //Get the XML child and see if it exists
    {
        throw std::runtime_error("No XML node named " + name + " found!"); //Throw an error with the message
    }

    return xmlNode.child(name.c_str()); //Return the xml node found
}


/**
 * @brief Function to require an XML attribute from a node
 * 
 * @param xmlNode The XML node that the attribute is required to be from
 * @param name The name of the XML attribute
 * @return const pugi::xml_attribute The attribute, if it exists
 * @throw std::runtime_error if the attribute doesn't exist
 */
inline const pugi::xml_attribute REQUIREATTRIB(const pugi::xml_node& xmlNode, std::string name)
{
    //Throw an error message if the attribute doesn't exist
    if(xmlNode.attribute(name.c_str()).empty()) throw std::runtime_error( (std::string("XML node \'") + xmlNode.name() + "\' missing required attribute: \'" + name + "\'").c_str() ); 

    return xmlNode.attribute(name.c_str()); //If no throw, then return the attribute 
}   

/**
 * @brief Function to clean whitespace from strings
 * 
 * @param str The string address to strip whitespace from
 */
void cleanWhiteSpace(std::string& str)
{
   str.erase(std::remove(str.begin(), str.end(), '\n'), str.end()); //Strip any whitespace from the string
}

/**
 * @brief Function to remove any and all HTML tags from a string
 * 
 * @param str The string reference to remove tags from
 */
void cleanHTML(std::string& str)
{
    while (str.find("<") != std::string::npos)
    {
        auto startpos = str.find("<");
        auto endpos = str.find(">") + 1;

        if (endpos != std::string::npos)
        {
            str.erase(startpos, endpos - startpos);
        }
    }
}

RssImage RssImage::fromXML(const pugi::xml_node& xmlNode)
{
    RssImage retImg; //The returned image struct with all data filled in
    if(xmlNode.empty()) return retImg; //Return non filled img struct if the node is empty

    try
    {
        retImg.title = REQUIRENODE(xmlNode, "title").text().as_string(); //Get the title of the RSS image
        retImg.url = REQUIRENODE(xmlNode, "url").text().as_string(); //Get the image source URL 

        retImg.width = (xmlNode.child("width").empty()) ? retImg.width : xmlNode.child("width").text().as_uint(); //Get the width or keep it the same if it isn't specifief
        retImg.height = (xmlNode.child("height").empty()) ? retImg.height : xmlNode.child("height").text().as_uint(); //Same with height

        try
        {
            retImg.loadImgFromUrl(retImg.url); //Get the error from loading the image
        }
        catch(const std::exception& e) //Check if any errors occured ant throw them
        {
            throw e;
        }

        retImg.description = xmlNode.child("description").text().as_string(); //Get the optional description of the image
    }
    catch(const std::exception& e) //Catch any REQUIRENODE errors and return a bad img struct if they occur
    {
        logE("Failed to get image from RSS: %s", e.what());
        return retImg;
    }

    retImg.filled = true; //If no errors were thrown at all, set the filled attribute to true on the image
    return retImg;
    
}

void RssImage::loadImgFromUrl(const std::string t_url)
{
    //Make a GET request for the image data to load from the enclosure URL
    cpr::Response imgResp = cpr::Get(cpr::Url{t_url},
                                    cpr::Timeout(5000)                                     
                                    ); 
    //Log any errors that occur from getting the image
    if(imgResp.error.code != cpr::ErrorCode::OK) throw std::runtime_error(std::string("HTTP GET request for image failed! EC: ") + std::to_string((unsigned int)imgResp.error.code) + " Reason: " + imgResp.error.message);
    
    FILE* tmp = fopen("cache", "wb"); //Open the image cache file in write mode
    fwrite(imgResp.text.data(), 1, imgResp.text.size(), tmp); //Write the image data to a cache file
    fclose(tmp); //Close the cache file

    unsigned char* imgDat = stbi_load("cache", &width, &height, &ch, 4); //Re load the image data from the cache file
    if(imgDat == NULL) throw std::runtime_error("Failed to load image data from cache file!"); //Throw an error if stb_image somehow fails

    glGenTextures(1, &txID); //Generate a texture ID in openGL
    glBindTexture(GL_TEXTURE_2D, txID); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgDat); //Generate an OpenGL texture using the image data
    
    stbi_image_free(imgDat); //No memory leaks here 

    filled = true; //We filled this image with data, so set it 
}

RssImage RssImage::fromXMLEnclosure(const pugi::xml_node& xmlNode)
{
    RssImage retImg; //The constructed RSS image object 

    try
    {
        std::string contentType = ""; //The content type specified by the enclosure node
        contentType = REQUIREATTRIB(xmlNode, "type").as_string(); //Get the content type, hoping for image
        if(contentType.compare(0, strlen("image/"), "image/") != 0) //If the content type is not an image, throw an error and exit
        {
            throw std::runtime_error( ("Enclosure found in RSS item, but it is an unsupported content type: " + contentType).c_str() );
        }


        retImg.title = "Attachment";
        retImg.url = REQUIREATTRIB(xmlNode, "url").as_string(); //Get the required URL of the image
    }
    catch(const std::exception& e) //Catch any REQUIRENODE errors and return the bad image struct 
    {
        logW("%s", e.what());
        return retImg; 
    }

    //Note: filled is not set for the image here, because it is explicitly set when we later call loadImgFromUrl
    return retImg;
}

RssItem RssItem::fromXML(const pugi::xml_node& xmlNode)
{
    RssItem retItem; //The returned RSS item object constructed from XML

    try
    {
        if(xmlNode.empty()) throw std::runtime_error("Attempted to construct an RSS item from an empty XML node"); //Make sure the XML node exists

        retItem.title = REQUIRENODE(xmlNode, "title").text().as_string(); //Get the required title of the RSS item
        retItem.link = REQUIRENODE(xmlNode, "link").text().as_string();   //Get the required link to the RSS item
        retItem.description = REQUIRENODE(xmlNode, "description").text().as_string(); //Get the required description of the RSS item



        retItem.author = xmlNode.child("author").text().as_string(); //Get the optional author of the item
        retItem.enclosure = RssImage::fromXMLEnclosure(xmlNode.child("enclosure") ); //Get the optional attachment for the item
        retItem.pubDate = xmlNode.child("pubDate").text().as_string(); //Get the optional publication date of the item

        cleanHTML(retItem.description); //Strip any HTML tags from the item desciption
        cleanHTML(retItem.title);
        
    }
    catch(const std::exception& e) //Propogate any error upwards to the caller if a required field is missing
    {
        throw e;
    }

    return retItem;
}

RssChannel RssChannel::fromXML(const pugi::xml_document& xmlDoc, const std::string link)
{
    if(xmlDoc.empty()) throw std::runtime_error("Attempted to parse an empty XML document!"); //Throw an error if the XML node is not valid

    RssChannel retChannel; //The constructed RSS channel object to return
    pugi::xml_node channelNode; //The pugixml channel item node

    
    try
    {
        channelNode = REQUIRENODE( REQUIRENODE(xmlDoc, "rss"), "channel"); //Require the RSS channel node 

        retChannel.title = REQUIRENODE(channelNode, "title").text().as_string(); //Get the title string of the RSS channel
        retChannel.description = REQUIRENODE(channelNode, "description").text().as_string(); //Get the description of the RSS channel
        retChannel.link = link; //Set the source URL for this channel

        //Clean up any whitespace from the title and link
        cleanWhiteSpace(retChannel.title);
        cleanWhiteSpace(retChannel.link);

        retChannel.ttl = ( ( channelNode.child("ttl").empty() ) ? 0ULL : channelNode.child("ttl").text().as_ullong() ); //Get optional ttl parameter

        retChannel.image = RssImage::fromXML(channelNode.child("image")); //Get the image attribute of the Rss Channel

        for(const pugi::xml_node& item : channelNode.children("item")) //For every item in the channel...
        {
            try
            {
                retChannel.items.push_back(RssItem::fromXML(item)); //Attempt to make an item from the XML data and add it to the list of items
            }
            catch(const std::exception& e) //Catch any error creating the item and log them, don't throw them
            {
                logE("Failed to construct item from XML: %s", e.what());
            }
            
        }

        if(retChannel.ttl != 0) //If TTL exists, then cache this file for performance
        {
            std::string cachePath = "cached/";            //The cached XML file path
            cachePath.append(retChannel.title);           //Append the file name  
            cachePath.append(".rss");                     //Append the rss extension for clarity

            xmlDoc.save_file(cachePath.c_str());          //Cache the XML source to a file
        }

        //Clean RSS channel title and description of any HTML tags
        cleanHTML(retChannel.description); 
        cleanHTML(retChannel.title);

    }
    catch(const std::exception& e) //If any error was thrown by REQUIRENODE, propogate them up
    {
        throw e;
    }

    return retChannel;
}   

RssChannel RssChannel::fromUrl(const std::string url)
{
    cpr::Response resp = cpr::Get(cpr::Url{url}, cpr::Timeout{5000}); //Get the RSS feed from the recorded URL
    if(resp.error.code != cpr::ErrorCode::OK) //If any error occured, throw it
    {
        throw std::runtime_error("HTTP GET request failed with error: " + resp.error.message);
    }

    pugi::xml_document doc; //The document we will get from the recieved URL
    pugi::xml_parse_result parseRes = doc.load_string(resp.text.c_str()); //Load the XML data from the recieved response
    if(!parseRes) //If the parsing failed...
    {
        throw std::runtime_error("Failed to parse XML recieved from " + url + "! Error: " + parseRes.description());
    }

    RssChannel rssCh;

    try
    {
        rssCh = RssChannel::fromXML(doc, url); //Construct an RSS channel from the XML document
        //Use wordy chrono library to get time in minutes since a known date that this feed was refreshed
        rssCh.lastChecked = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    catch(const std::exception& e) //Propogate any errors up
    {
        throw e;
    }
    

    logI("Loaded RSS feed from URL %s", url.c_str());
    return rssCh;
}

RssFeedManager::RssFeedManager(void)
{
    recordFile.open("subscribed.txt", std::ios::app | std::ios_base::out | std::ios_base::in); //Open the subscribed channels list in append, not truncate mode
}

RssFeedManager::~RssFeedManager()
{
    writeRecord(); //Write all records to the file
}

void RssFeedManager::addChannel(const std::string link)
{
    try
    {
        RssChannel ch = RssChannel::fromUrl(link); //Attempt to create an RSS channel from the XML document
        channels.push_back(ch);                     //Add the channel to our list if it was created 

    }
    catch(const std::exception& e) //Catch any errors thrown by the channel creation
    {
        throw std::runtime_error(std::string("Failed to add RSS channel to subscribed! Reason: ") + e.what());
    }
}

void RssFeedManager::removeChannel(const std::string title)
{
    //Remove an element from the list if the title matches the title given
    channels.erase(
    std::remove_if(channels.begin(), channels.end(), [&](const RssChannel& ch) -> bool 
    {
        if(ch.title.compare(title) == 0) return true;
        else                             return false;
    }), channels.end());
}

void RssFeedManager::loadChannelsFromRecord(void)
{
    recordFile.seekg(0, std::ios::end); //Seek the end of the file
    size_t size = recordFile.tellg();   //Get the size of the record file
    if(size < 10) return;               //If the file size is < 10 bytes, return early, there are no entries

    recordFile.seekg(0); //Return to line 0 of the file
    std::string line; //Read line of the record file

    while(!recordFile.eof()) //Get every channel in the file
    {
        char *badChar; //The character that a strin to size_t conversion failed on

        std::string title; //Title read from the record
        std::string url;   //URL of the RSS feed
        size_t ttl;        //Time to live of the RSS feed
        size_t lastUpdate; //The last updated time in minutes

        std::getline(recordFile, title);
        if(title.empty()) break; //Don't read final newline as another entry
        
        std::getline(recordFile, url);

        std::getline(recordFile, line); //Get ttl line in file
        ttl = std::strtoull(line.c_str(), &badChar, 10); //Convert the read string to a size_t
        std::getline(recordFile, line);
        lastUpdate = std::strtoull(line.c_str(), &badChar, 10); //Conver the last update minutes to a size_t

        size_t thisMinute = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now().time_since_epoch()).count(); //Get how many minutes have passed since epoch
        if( (thisMinute - lastUpdate) > ttl) //If we need to refresh the RSS feed, get it from the URL
        {
            try
            {
                channels.push_back(RssChannel::fromUrl(url)); //Attempt to construct an RSS channel from the URL
            }
            catch(const std::exception& e) //Catch any bad XML parsing errors
            {
                logE("Error loading channel from %s! Error: %s", url.c_str(), e.what());
                continue; //Continue to next channel in the record
            }
            logI("Downloaded RSS feed %s from %s", title.c_str(), url.c_str());
        }
        else //Use a cached RSS file if the ttl duration hasn't passed
        {
            logI("RSS feed \'%s\' TTL is %zu, it has been %zu minutes since the feed was last checked", title.c_str(), ttl, thisMinute - lastUpdate); //Log how long the cached feed has gone without an update
            std::string cachePath = "cached/"; //The cached XML file path
            cachePath.append(title);           //Append the file name  
            cachePath.append(".rss");          //Append the rss extension for clarity

            pugi::xml_document doc; //The xml document to load the cache from
            pugi::xml_parse_result res = doc.load_file(cachePath.c_str()); //Load the XML file from the cache
            if(!res) //If the XML parsing failed...
            {
                //Log the error 
                logW("Failed to parse cached XML file from %s; error %s, attempting to load from URL instead...", cachePath.c_str(), res.description());
                try //Try to download the RSS feed instead
                { 
                    channels.push_back(RssChannel::fromUrl(url)); //Attempt to load the channel from url
                }
                catch(const std::exception& e) //Catch any channel construction errors
                {
                    logE("Failed to load RSS channel from URL %s after failing to load cache file!", e.what());
                    continue; //Continue to next item in the record 
                }

                logI("Downloaded \'%s\' RSS feed from \'%s\' after failing to load cached XML", title.c_str(), url.c_str());
            }
            else
            {
                try
                {
                    channels.push_back(RssChannel::fromXML(doc, url)); //Make the rss channel from the XML document loaded
                    channels.back().lastChecked = lastUpdate; //Keep the old last checked timestamp after loading from XML

                    logI("RSS channel \'%s\' loaded from cached RSS file \'%s\'", title.c_str(), cachePath.c_str());
                }
                catch(const std::exception& e) //Catch any channel construction errors
                {
                    logW("Failed to parse cached XML file at \'%s\'; error: \'%s\', falling back to URL...", cachePath.c_str(), e.what());
                    try //Try to download the RSS feed instead
                    { 
                        channels.push_back(RssChannel::fromUrl(url)); //Attempt to load the channel from url
                    }
                    catch(const std::exception& e) //Catch any channel construction errors
                    {
                        logE("Failed to load RSS channel from URL %s after failing to load cache file!", e.what());
                        continue; //Continue to next item in the record 
                    }

                    logI("Downloaded \'%s\' RSS feed from %s after failing to load cached XML", title.c_str(), url.c_str());
                }


            }

        }
        
    }

    recordFile.close();
    recordFile.open("subscribed.txt", std::ios::app | std::ios_base::out | std::ios_base::in);
}

void RssFeedManager::writeRecord(void)
{
    recordFile.close();
    recordFile.open("subscribed.txt", std::ios::trunc | std::ios::out); //Reopen the record file in write mode

    for(auto& ch : channels) //For every channel, write it's last checked date
    {
        recordFile << ch.title << "\n" << ch.link << "\n" << ch.ttl << "\n" << ch.lastChecked << std::endl; //Record this channel in our list of subscribed
    }

    recordFile.close();
}

