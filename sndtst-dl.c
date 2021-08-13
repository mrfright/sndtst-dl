/*   sndtst-dl    */
/* download all of the audio files from sndtst.com */
/* and save in an organized directory structure    */

#include <stdio.h>
#include <curl/curl.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char *SNDTST_URL = "https://sndtst.com";

uint write_maincb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}

uint write_systemcb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}

void parseSystemNode(TidyDoc doc, TidyNode tnod, char* sndtstdir);
    
void parseIndexNode(TidyDoc doc, TidyNode tnod)
{
    struct stat st = {0};

    char *sndtstdir = "sndtst";
    if (stat(sndtstdir, &st) == -1) {
        mkdir(sndtstdir, 0700);
    }
    
    TidyNode child;
    for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            /* walk the attribute list looking for the systems*/
            for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                if(tidyAttrValue(attr)) {
                    if(strcmp("col-6 col-md-3 text-center Platform", tidyAttrValue(attr))==0) {
                        parseSystemNode(doc, child, sndtstdir); 
                    }
                }
            }
        } //only care about named nodes, not content here because looking for the systems
        
        parseIndexNode(doc, child);
    }
}


/* at this point, should be node with each system info */
void parseSystemNode(TidyDoc doc, TidyNode tnod, char* sndtstdir)
{
    TidyNode child;
    child = tidyGetChild(tnod);
    char system_url[100] = {0};
    strcpy(system_url, SNDTST_URL);

    int valid_url = 0;    

    //this one should  be the link
    if(child) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            TidyAttr attr;

            for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                if(tidyAttrValue(attr)) {
                    strcat(system_url, tidyAttrValue(attr));
                    valid_url = 1;
                }
            }
        }
        else {
            printf("expected system link didn't have node name\n");
        }
    }
    else {
        printf("no child for link for system\n");
    }

    child = tidyGetChild(child);
    unsigned int numgames = 0;
    //this one should be the number of games on that system
    if(child) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            //get child of this h1, that's the number text
            
            TidyNode numchild = tidyGetChild(child);

            if(numchild) {
                TidyBuffer buf;
                tidyBufInit(&buf);
                tidyNodeGetText(doc, numchild, &buf);
                //printf("%s\n", buf.bp?(char *)buf.bp:"");
                numgames = strtoul(buf.bp, 0L, 10);
                tidyBufFree(&buf);    
            }
            else {
                printf("system num no child\n");
            }
        }
        else {
            printf("expected system number didn't have node name\n");
        }
    }
    else {
        printf("no number for system\n");
    }


    child = tidyGetNext(child);
    char systemdir[1000] = {0};
    int valid_systemdir = 0;
    //this one should be the system name, make a directory of it
    if(child) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            //get child of this h1, that's the number text
            
            TidyNode namechild = tidyGetChild(child);

            if(namechild) {
                TidyBuffer buf;
                tidyBufInit(&buf);
                tidyNodeGetText(doc, namechild, &buf);

                struct stat st = {0};

                if(buf.bp){
                    strcpy(systemdir, sndtstdir);
                    strcat(systemdir, "/");
                    strcat(systemdir, (char *)buf.bp);
                    systemdir[strcspn(systemdir, "\n")] = 0;                    

                    if (stat(systemdir, &st) == -1) {
                        mkdir(systemdir, 0700);
                    }
                    valid_systemdir = 1;
                }
                else {
                    printf("system name no buf.bp\n");
                }

                tidyBufFree(&buf);
            }
            else {
                printf("system namechild not there\n");
            }
        }
        else {
            printf("expected system name didn't have node name\n");
        }
    }
    else {
        printf("no child for system name\n");
    }

    //if valid name, number(?), url, then process the system
    if(valid_url && valid_systemdir && numgames) {
        getSystemPage(system_url, numgames, systemdir);
    }
}

 
int main(int argc, char **argv)
{

    CURL *curl;
    char curl_errbuf[CURL_ERROR_SIZE];
    TidyDoc tdoc;
    TidyBuffer docbuf = {0};
    TidyBuffer tidy_errbuf = {0};
    int err;
 
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, SNDTST_URL);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_maincb);
 
    tdoc = tidyCreate();
    tidyOptSetBool(tdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(tdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(tdoc, &tidy_errbuf);
    tidyBufInit(&docbuf);
 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &docbuf);
    err = curl_easy_perform(curl);
    if(!err) {
        err = tidyParseBuffer(tdoc, &docbuf); /* parse the input */
        if(err >= 0) {
            err = tidyCleanAndRepair(tdoc); /* fix any problems */
            if(err >= 0) {
                err = tidyRunDiagnostics(tdoc); /* load tidy error buffer */
                if(err >= 0) {
                    parseIndexNode(tdoc, tidyGetRoot(tdoc)); /* walk the tree */
                    fprintf(stderr, "%s\n", tidy_errbuf.bp); /* show errors */
                }
            }
        }
    }
    else
        fprintf(stderr, "%s\n", curl_errbuf);
 
    /* clean-up */
    curl_easy_cleanup(curl);
    tidyBufFree(&docbuf);
    tidyBufFree(&tidy_errbuf);
    tidyRelease(tdoc);
    return err;
 

 

}






 
int getSystemPage(char *systemurl, int numgames, char* systemdir)
{

    CURL *systemcurl;
    char systemcurl_errbuf[CURL_ERROR_SIZE];
    TidyDoc systemtdoc;
    TidyBuffer systemdocbuf = {0};
    TidyBuffer systemtidy_errbuf = {0};
    int systemerr;
 
    systemcurl = curl_easy_init();
    curl_easy_setopt(systemcurl, CURLOPT_URL, systemurl);
    curl_easy_setopt(systemcurl, CURLOPT_ERRORBUFFER, systemcurl_errbuf);
    curl_easy_setopt(systemcurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(systemcurl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(systemcurl, CURLOPT_WRITEFUNCTION, write_systemcb);
 
    systemtdoc = tidyCreate();
    tidyOptSetBool(systemtdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(systemtdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(systemtdoc, &systemtidy_errbuf);
    tidyBufInit(&systemdocbuf);
 
    curl_easy_setopt(systemcurl, CURLOPT_WRITEDATA, &systemdocbuf);
    systemerr = curl_easy_perform(systemcurl);
    if(!systemerr) {
        systemerr = tidyParseBuffer(systemtdoc, &systemdocbuf); /* parse the input */
        if(systemerr >= 0) {
            systemerr = tidyCleanAndRepair(systemtdoc); /* fix any problems */
            if(systemerr >= 0) {
                systemerr = tidyRunDiagnostics(systemtdoc); /* load tidy error buffer */
                if(systemerr >= 0) {
                    printf("processed %s\n", systemurl);
                    //parseIndexNode(systemtdoc, tidyGetRoot(systemtdoc)); /* walk the tree */
                    //fprintf(stderr, "%s\n", systemtidy_errbuf.bp); /* show errors */
                    
                }
            }
        }
    }
    else {
        //fprintf(stderr, "%s\n", systemcurl_errbuf);
    }
    
    /* clean-up */
    curl_easy_cleanup(systemcurl);
    tidyBufFree(&systemdocbuf);
    tidyBufFree(&systemtidy_errbuf);
    tidyRelease(systemtdoc);
    return systemerr;
 

 

}
