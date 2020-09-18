#include<iostream>
#include<string>
#include<vector>

#include <sys/time.h>
#include <curl/curl.h>

#ifndef FILE_SEND_H
#define FILE_SEND_H

class file_send
{
private:
    CURL *curl;
 
    CURLM *multi_handle;
    int still_running;
 
    curl_mime *form;
    curl_mimepart *field;
    struct curl_slist *headerlist;

    std::vector< std::pair< std::string, std::string > > file_name_list;
    std::vector< std::pair< std::string, std::string > > form_name_list;
public:
    void init();
    void file_set( std::string form_name, std::string file_name );
    void form_set( std::string form_name, std::string form_data );
    void send_data( std::string url );
};
#endif
