import os
import copy
import time
from bs4 import BeautifulSoup
import requests

from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By

CHANGELOG_DIR = "./changelog/"

def get_pr_title( soup ):
    pr_title = ""
    bdi_tag = soup.find_all( "bdi" )

    for bdi in bdi_tag:
        class_name = bdi.get( "class" )

        if not class_name == None and \
           not len( class_name ) == 0 and \
           class_name[0] == "js-issue-title":
            pr_title = bdi.text

    return pr_title

def check_test_file( file_name ):
    split_name = file_name.split( "/" )

    if split_name[0] == "tests":
        return True

    return False

def get_difference( soup ):
    result = {}
    div_tag = soup.find_all( "div" )

    for div in div_tag:
        class_name = div.get( "class" )

        if not class_name == None and \
           not len( class_name ) == 0 and \
           class_name[0] == "js-diff-progressive-container":
            cde_tag = div.find_all( "copilot-diff-entry" )

            for cde in cde_tag:
                diff_file_name = cde.get( "data-file-path" )
                if check_test_file( diff_file_name ):
                    continue

                diff_text = "x"
                tr_tag = cde.find_all( "tr" )

                for tr in tr_tag:
                    class_name = tr.get( "class" )

                    if not class_name == None and \
                       not len( class_name ) == 0 and \
                       class_name[0] == "show-top-border":
                        
                        code_line = tr.find( "span" ).get("data-code-marker")

                        if code_line == "+" or code_line == "-":
                            diff_text += tr.text.replace( "\n", "" )
                            #print( "" )
                        
                result[diff_file_name] = copy.deepcopy( diff_text )
                
    return result

def get_explanation( soup ):
    explanation_text = ""
    div_tag = soup.find_all( "div" )

    for div in div_tag:
        class_name = div.get( "class" )

        if not class_name == None and \
           not len( class_name ) == 0 and \
           class_name[0] == "edit-comment-hide":
            explanation_text = div.text.replace( "\n", "" )
            break

    return explanation_text

def get_reocrds_data( url ):
    result = {}
    r = requests.get( url )
    soup = BeautifulSoup( r.content, "html.parser" )

    span_tag = soup.find_all( "span" )

    for span in span_tag:
        class_name = span.get( "class" )

        if not class_name == None and \
           len( class_name ) == 2 and \
           class_name[0] == "sig-name":
            result[span.text.replace( "\n", "" )] = []

    return result

def main():
    wf = open( "RecorsSearchResult.txt", "w" )

    records9_data_dict = get_reocrds_data( "https://docs.trafficserver.apache.org/en/9.2.x/admin-guide/files/records.config.en.html" )
    records10_data_dict = get_reocrds_data( "https://docs.trafficserver.apache.org/en/10.0.x/admin-guide/files/records.yaml.en.html" )

    wf.write( "9->10 delete\n" )
    for record in records9_data_dict.keys():
        if not record in records10_data_dict:
            wf.write( record + "\n" )

    wf.write( "\n\n" )
    wf.write( "9->10 add\n" )
    add_record_list = []
    
    for record in records10_data_dict.keys():
        if not record in records9_data_dict:
            add_record_list.append( record )
            wf.write( record + "\n" )
            

    pr_num_list = []
    file_names = os.listdir( CHANGELOG_DIR )

    for file_name in file_names:
        f = open( CHANGELOG_DIR + file_name )
        line_data = f.readlines()

        for i in range( 1, len( line_data ) ):
            clear_data = line_data[i].rstrip( "\n" )
            split_data = clear_data.split( " " )
            pr_number = split_data[2].replace( "#", "" )

            if not pr_number in pr_num_list:
                pr_num_list.append( pr_number )

        f.close()

    count = len( pr_num_list )
    driver = webdriver.Chrome()
    
    for pr_num in pr_num_list:
        print( count )
        count -= 1
        test_str = "proxy.config.exec_thread.limit"
        url = "https://github.com/apache/trafficserver/pull/{}".format( pr_num )
        driver.get( url )
        time.sleep( 2 )
        html = driver.page_source.encode('utf-8')
        soup = BeautifulSoup( html, "html.parser" )
        explanation_text = get_explanation( soup )

        diff_url = url + "/files"
        driver.get( diff_url )
        time.sleep( 2 )
        html = driver.page_source.encode('utf-8')
        soup = BeautifulSoup( html, "html.parser" )
        #get_pr_title( soup )
        diff_data = get_difference( soup )

        for file_name in diff_data.keys():
            for record in records9_data_dict.keys():
                if ( record in diff_data[file_name] or record in explanation_text ) and \
                   not url in records9_data_dict[record]:
                    records9_data_dict[record].append( url )

            for record in add_record_list:
                if record in diff_data[file_name] or record in explanation_text and \
                   not url in records10_data_dict[record]:
                    records10_data_dict[record].append( url )

    wf.write( "\n\n" )
    wf.write( "PR ATS9 record Check\n" )

    for record in records9_data_dict.keys():
        if not len( records9_data_dict[record] ) == 0:
            wf.write( record + "\n" )

            for url in records9_data_dict[record]:
                wf.write( url + "\n" )

            wf.write( "\n" )

    wf.write( "\n" )
    wf.write( "PR ATS10 record Check\n" )
            
    for record in records10_data_dict.keys():
        if not len( records10_data_dict[record] ) == 0:
            wf.write( record + "\n" )

            for url in records10_data_dict[record]:
                wf.write( url + "\n" )

            wf.write( "\n" )

    wf.close()
            
if __name__ == "__main__":
    main()
