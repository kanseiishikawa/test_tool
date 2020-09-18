#include <iostream>
#include <string>
#include <sys/time.h>
#include <curl/curl.h>

#include "file_send.h"

int main()
{
    file_send fs;
    fs.init();
    fs.file_set( "sendfile", "main.cpp" );
    fs.form_set( "test", "test" );
    fs.send_data( "http://localhost:8080" );
}
