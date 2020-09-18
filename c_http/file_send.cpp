#include<iostream>
#include<string>
#include<vector>
#include<cmath>

#include <sys/time.h>
#include <curl/curl.h>

#include "file_send.h"

void
file_send::init() {
    curl = curl_easy_init();
    multi_handle = curl_multi_init();
    still_running = 0;
 
    form = NULL;
    field = NULL;
    headerlist = NULL;
}

void
file_send::file_set( std::string form_name, std::string file_name ) {
    std::pair< std::string, std::string > instance;
    instance.first = form_name;
    instance.second = file_name;
    file_name_list.push_back( instance );
}

void
file_send::form_set( std::string form_name, std::string form_data ) {
    std::pair< std::string, std::string > instance;
    instance.first = form_name;
    instance.second = form_data;
    form_name_list.push_back( instance );
}

void
file_send::send_data( std::string url ) {
    const char buf[] = "Expect:";
    std::string file_name = "";
    
    form = curl_mime_init(curl);

    for( int i = 0; i < file_name_list.size(); i++ ) {
        /* ファイルアップロードフィールドに入力します */ 
        field = curl_mime_addpart( form );
        curl_mime_name( field, file_name_list[i].first.c_str() );//フォームの名前を入力
        curl_mime_filedata(field, file_name_list[i].second.c_str() );

        if( file_name.size() == 0 )
        {
            file_name = file_name_list[i].second;
        } else {
            file_name += ":" + file_name_list[i].second;
        }
    }
    
    for( int i = 0; i < form_name_list.size(); i++ )
    {
        field = curl_mime_addpart( form );
        curl_mime_name( field, form_name_list[i].first.c_str() );
        curl_mime_data( field, form_name_list[i].second.c_str(), CURL_ZERO_TERMINATED );        
    }

    if( file_name.size() != 0 ) {
        field = curl_mime_addpart( form );
        curl_mime_name( field, "file_name" );
        curl_mime_data( field, file_name.c_str(), CURL_ZERO_TERMINATED );
    }

    field = curl_mime_addpart( form );
    curl_mime_name( field, "submit" );
    curl_mime_data( field, "send", CURL_ZERO_TERMINATED );

    /* カスタムヘッダーリストを初期化します（期待：100-継続は不要であることを示します） */ 
    headerlist = curl_slist_append(headerlist, buf );

    /* このPOSTを受信するURL */ 
    //curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080" );
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    
    curl_multi_add_handle(multi_handle, curl);
    curl_multi_perform(multi_handle, &still_running);
    
    while(still_running) {
        struct timeval timeout;
        int rc; /* select() return code */ 
        CURLMcode mc; /* curl_multi_fdset() return code */ 
        
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;
        
        long curl_timeo = -1;
        
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        
        /* 適切なタイムアウトを設定して遊んでください */ 
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        curl_multi_timeout(multi_handle, &curl_timeo);
        if(curl_timeo >= 0) {
            timeout.tv_sec = curl_timeo / 1000;
            if(timeout.tv_sec > 1)
                timeout.tv_sec = 1;
            else
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
        }
        
        /* 転送からファイル記述子を取得する */ 
        mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
        
        if(mc != CURLM_OK) {
            fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
            break;
        }
        
        /* On success the value of maxfd is guaranteed to be >= -1. We call
           select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
           no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
           to sleep 100ms, which is the minimum suggested value in the
           curl_multi_fdset() doc. */ 
        if(maxfd == -1) {
#ifdef _WIN32
            Sleep(100);
            rc = 0;
#else
            /* Portable sleep for platforms other than Windows. */ 
            struct timeval wait = { 0, 100 * 1000 }; /* 100ms */ 
            rc = select(0, NULL, NULL, NULL, &wait);
#endif
        }
        else {
            /* Note that on some platforms 'timeout' may be modified by select().
               If you need access to the original value save a copy beforehand. */ 
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }
            
        switch(rc) {
        case -1:
            break;
        case 0:
        default:
            curl_multi_perform(multi_handle, &still_running);
            break;
        }
    }
    
    curl_multi_cleanup(multi_handle);
    
    /* always cleanup */ 
    curl_easy_cleanup(curl);
    
    /* then cleanup the form */ 
    curl_mime_free(form);
    
    /* free slist */ 
    curl_slist_free_all(headerlist);
    
}
