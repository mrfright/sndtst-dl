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


uint write_gamecb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}


uint write_titlegamecb(char *in, uint size, uint nmemb, TidyBuffer *out)
{
    uint r;
    r = size * nmemb;
    tidyBufAppend(out, in, r);
    return r;
}


void parseSystemNode(TidyDoc doc, TidyNode tnod, char* sndtstdir);
int getSystemPage(char *systemurl, int numgames, char* systemdir);
void parseSystemLink(TidyDoc doc, TidyNode tnod, char* systemdir);
int getGamePage(char *gameurl, char *gamename, char* gamedir);
int getSong(char *songurl, char* gamedir, char *songname);
    
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
















    
void parseSystemPage(TidyDoc doc, TidyNode tnod, char* systemdir)
{    
    TidyNode child;
    for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            /* walk the attribute list looking for the systems*/
            for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                if(tidyAttrValue(attr)) {
                    if(strcmp("col-md-9", tidyAttrValue(attr))==0) {
                        parseSystemLink(doc, child, systemdir);
                    }
                }
            }
        } //only care about named nodes, not content here because looking for the systems
        
        parseSystemPage(doc, child, systemdir);
    }
}


    
void parseSystemLink(TidyDoc doc, TidyNode tnod, char *systemdir)
{    
    TidyNode child;
    for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            //each link in the col-md-9 div is a link to a game on this system
            //get the link and the name
            //for each make a dir
            //then go to that link and parse all those games
            if(strcmp("a", name) == 0) {
                TidyAttr attr;

                /* walk the attribute list looking for the systems*/
                for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                    if(tidyAttrValue(attr)) {
                        //att text should be link
                        

                        //combine this tidyAttrVavlue(attr) with base url for full
                        //game link

                        //get the child for this node, it's text
                        //that's the game name
                        char gameurl[100];
                        strcpy(gameurl, SNDTST_URL);
                        strcat(gameurl, tidyAttrValue(attr));

                        TidyNode namechild = tidyGetChild(child);

                        if(namechild) {
                            TidyBuffer namebuf;
                            tidyBufInit(&namebuf);
                            tidyNodeGetText(doc, namechild, &namebuf);

                            //combine systemdir with game name
                            char gamedir[1000];
                            
                            strcpy(gamedir, systemdir);
                            strcat(gamedir, "/");
                            strcat(gamedir, namebuf.bp);

                            gamedir[strcspn(gamedir, "\n")] = 0;


                            //create dir
                            struct stat st = {0};


                            if (stat(gamedir, &st) == -1) {
                                mkdir(gamedir, 0700);
                            }
    
                            
                            getGamePage(gameurl, namebuf.bp, gamedir);
                            
                            tidyBufFree(&namebuf);    
                        }
                        else {
                            printf("no game name child for %s\n", gameurl);
                        }
                    }
                    else {
                        printf("no game url\n");
                    }
                }
            }
        } //only care about named nodes, not content here because looking for the systems
        
        parseSystemLink(doc, child, systemdir);
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















    
void parseGamePage(TidyDoc doc, TidyNode tnod, char* gamedir, int *gamenum)
{    
    TidyNode child;


    for(child = tidyGetChild(tnod); child; child = tidyGetNext(child) ) {
        ctmbstr name = tidyNodeGetName(child);
        if(name) {
            /* if it has a name, then it's an HTML tag ... */
            TidyAttr attr;

            /* walk the attribute list looking for the systems*/

            for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
                if(tidyAttrValue(attr)) {

                    if(strcmp("og:image", tidyAttrValue(attr))==0) {
                        //if got the og:image, the next attr should be the link
                        attr = tidyAttrNext(attr);
                        if(tidyAttrValue(attr)) {
                            printf("game image url: %s\n", tidyAttrValue(attr));
                            //get game number from image?
//https://s3-us-west-2.amazonaws.com/media.sndtst.com/game/269/title
                            //if the start equals the start of that,
                            //and the end equals the end of that
                            //extract number
                            //then if

                            char *basegameurl = "https://s3-us-west-2.amazonaws.com/media.sndtst.com/game/";
                            char *basetitleend = "/title";
                            int basetitlelen = strlen(basetitleend);
                            int gametitlestrlen = strlen(tidyAttrValue(attr));
                            if(strlen(basegameurl) < gametitlestrlen) {
                                //then big enough to have a number
                                

                                if(gametitlestrlen > basetitlelen) {
                                    //then big enough to have the title
                                    char *gametitle = tidyAttrValue(attr);
                                    char *titlestr = gametitle + (gametitlestrlen - basetitlelen);

                                    if(strcmp(basetitleend, titlestr) == 0) {
                                        
                                        // numstr = gametitle+strlen(basegameurl)
                                        //num numlen = gametitle - 
                                        //          (strlen(basegameurl) +
                                        //          basetitlelen)
                                        char *numstrptr = gametitle + strlen(basegameurl);
                                        int numlen = gametitlestrlen -
                                            (strlen(basegameurl) + basetitlelen);

                                        char numstr[100];
                                        strcpy(numstr, numstrptr);
                                        numstr[numlen] = '\0';
                                        printf("game num: '%s'\n", numstr);
                                        printf(numstr);
                                        *gamenum = strtol(numstr, NULL, 10);
                                        printf("gamenum: %i\n", *gamenum);

                                        //convert str num to actual int

                                        //save title as cover in the dir
                                        getTitlePic(gametitle, gamedir);

                                        //get each game url, name

                                        //copy getTitlePic, change for the audio file
                                    }
                                    else {
                                        printf("basetitleend '%s' not match titlestr '%s'\n",
                                               basetitleend,
                                               titlestr);
                                    }
                                }
                                else {
                                    printf("gametitlestrlen '%s':%d not greater than basetitlelen '%s':%d\n",
                                           tidyAttrValue(attr),
                                           gametitlestrlen,
                                           basetitleend,
                                           basetitlelen);
                                }
                            }
                            else {
                                printf("basegame url '%s':%d not smaller than title '%s':%d",
                                       basegameurl,
                                       strlen(basegameurl),
                                       tidyAttrValue(attr),
                                       gametitlestrlen);
                            }
                        }
                        //parseSystemLink(doc, child, systemdir);
                    }
                    else if(strcmp("col-md-9", tidyAttrValue(attr))==0) {
                        printf("found game audio%s: %d\n", *gamenum>0?": game num found":": game num not found", *gamenum);

                                      
                        //have the div, get all nex
                        ctmbstr gamename = tidyNodeGetName(child);
                        printf("should be game div: %s\n", gamename);
                        TidyNode gamechild = tidyGetChild(child);//button
                        gamename = tidyNodeGetName(gamechild);
                        printf("should be game button: %s\n", gamename);

                        gamechild = tidyGetNext(gamechild);//h2
                        gamechild = tidyGetNext(gamechild);//ol
                        
                        gamename = tidyNodeGetName(gamechild);
                        printf("should be game ol: %s\n", gamename);
                        
                        gamechild = tidyGetChild(gamechild);//now an li with song data

                        for(; gamechild; gamechild = tidyGetNext(gamechild)){
                            gamename = tidyNodeGetName(gamechild);

                            TidyAttr songattr = tidyAttrFirst(gamechild);
                            char songfile[200];
                            strcpy(songfile, tidyAttrValue(songattr));
                            char songname[200];
                            songattr = tidyAttrNext(songattr);
                            strcpy(songname, tidyAttrValue(songattr));
                            printf("\t\t song file:%s, song name:%s\n", songfile, songname);
//https://s3-us-west-2.amazonaws.com/media.sndtst.com/game/249/song/249-27ee5769a4533a2fOg4ef2f680Og14952a807c4OgLQ7bfb.ogg

                            char songurl[300];
                            char gamenumstr[100];
                            sprintf(gamenumstr,"%d",*gamenum);
                            strcpy(songurl,
                                   "https://s3-us-west-2.amazonaws.com/media.sndtst.com/game/");
                            strcat(songurl, gamenumstr);
                            strcat(songurl, "/song/");
                            strcat(songurl, songfile);
                            strcat(songurl, ".ogg");

                            printf("song url:\n%s\n", songurl);

                            printf("game dir: %s\n", gamedir);

                            getSong(songurl, gamedir, songname);
                            
#if 0
TidyAttr attr;



  for(attr = tidyAttrFirst(child); attr; attr = tidyAttrNext(attr) ) {
  if(tidyAttrValue(attr)) {

  if(strcmp("og:image", tidyAttrValue(attr))==0) {
  //if got the og:image, the next attr should be the link
  attr = tidyAttrNext(attr);
  if(tidyAttrValue(attr)) {
  printf("game image url: %s\n", tidyAttrValue(attr));

#endif
                            




                            
                        }
  
                    }
                }

                //url looks like this
                //https://s3-us-west-2.amazonaws.com/media.sndtst.com/game/269/song/269-506aa9346fb986dcOgLQ33e32277Og14a3c1c1266OgLQ7e5f.ogg
            }
        } //only care about named nodes, not content here because looking for the systems
        
        parseGamePage(doc, child, gamedir, gamenum);
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
                    parseSystemPage(systemtdoc, tidyGetRoot(systemtdoc), systemdir); /* walk the tree */
                    //fprintf(stderr, "%s\n", systemtidy_errbuf.bp); /* show errors */
                    
                }
            }
        }
    }
    else {
        fprintf(stderr, "%s\n", systemcurl_errbuf);
    }
    
    /* clean-up */
    curl_easy_cleanup(systemcurl);
    tidyBufFree(&systemdocbuf);
    tidyBufFree(&systemtidy_errbuf);
    tidyRelease(systemtdoc);
    return systemerr;
 

 

}








 
int getGamePage(char *gameurl, char *gamename, char* gamedir)
{

    CURL *gamecurl;
    char gamecurl_errbuf[CURL_ERROR_SIZE];
    TidyDoc gametdoc;
    TidyBuffer gamedocbuf = {0};
    TidyBuffer gametidy_errbuf = {0};
    int gameerr;
 
    gamecurl = curl_easy_init();
    curl_easy_setopt(gamecurl, CURLOPT_URL, gameurl);
    curl_easy_setopt(gamecurl, CURLOPT_ERRORBUFFER, gamecurl_errbuf);
    curl_easy_setopt(gamecurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(gamecurl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(gamecurl, CURLOPT_WRITEFUNCTION, write_gamecb);
 
    gametdoc = tidyCreate();
    tidyOptSetBool(gametdoc, TidyForceOutput, yes); /* try harder */
    tidyOptSetInt(gametdoc, TidyWrapLen, 4096);
    tidySetErrorBuffer(gametdoc, &gametidy_errbuf);
    tidyBufInit(&gamedocbuf);
 
    curl_easy_setopt(gamecurl, CURLOPT_WRITEDATA, &gamedocbuf);
    gameerr = curl_easy_perform(gamecurl);
    if(!gameerr) {
        gameerr = tidyParseBuffer(gametdoc, &gamedocbuf); /* parse the input */
        if(gameerr >= 0) {
            gameerr = tidyCleanAndRepair(gametdoc); /* fix any problems */
            if(gameerr >= 0) {
                gameerr = tidyRunDiagnostics(gametdoc); /* load tidy error buffer */
                if(gameerr >= 0) {
                    //download the album cover
                    //download all the mp3s
                    //parseSystemPage(gametdoc, tidyGetRoot(gametdoc)); /* walk the tree */
                    //fprintf(stderr, "%s\n", systemtidy_errbuf.bp); /* show errors */
                    int thisgamenum = 0;
                    parseGamePage(gametdoc, tidyGetRoot(gametdoc), gamedir, &thisgamenum); /* walk the tree */
                }
            }
        }
    }
    else {
        fprintf(stderr, "%s\n", gamecurl_errbuf);
    }
    
    /* clean-up */
    curl_easy_cleanup(gamecurl);
    tidyBufFree(&gamedocbuf);
    tidyBufFree(&gametidy_errbuf);
    tidyRelease(gametdoc);
    return gameerr;
 

 

}















 
int getTitlePic(char *gametitlepicurl, char* gamedir)
{
    CURL *gametitlecurl;
    char gametitlecurl_errbuf[CURL_ERROR_SIZE];

    int gametitleerr;
 
    gametitlecurl = curl_easy_init();
    curl_easy_setopt(gametitlecurl, CURLOPT_URL, gametitlepicurl);
    curl_easy_setopt(gametitlecurl, CURLOPT_ERRORBUFFER, gametitlecurl_errbuf);
    curl_easy_setopt(gametitlecurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(gametitlecurl, CURLOPT_VERBOSE, 1L);
//    curl_easy_setopt(gametitlecurl, CURLOPT_WRITEFUNCTION, write_titlegamecb);

    char coverpath[100];
    strcpy(coverpath, gamedir);
    strcat(coverpath, "/cover");
    printf("coverpath: '%s'\n", coverpath);
    FILE* coverpicfile = fopen( coverpath, "w");
 
    curl_easy_setopt(gametitlecurl, CURLOPT_WRITEDATA, coverpicfile);
    
    gametitleerr = curl_easy_perform(gametitlecurl);
    if(!gametitleerr) {
        //should have pic written by writetitlegamecb
    }
    else {
        fprintf(stderr, "%s\n", gametitlecurl_errbuf);
    }

    /* clean-up */
    curl_easy_cleanup(gametitlecurl);
    fclose(coverpicfile);

    return gametitleerr;
}


























 
int getSong(char *songurl, char* gamedir, char *songname)
{
    CURL *songcurl;
    char songcurl_errbuf[CURL_ERROR_SIZE];

    int songerr;
 
    songcurl = curl_easy_init();
    curl_easy_setopt(songcurl, CURLOPT_URL, songurl);
    curl_easy_setopt(songcurl, CURLOPT_ERRORBUFFER, songcurl_errbuf);
    curl_easy_setopt(songcurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(songcurl, CURLOPT_VERBOSE, 1L);

    char songpath[300];
    strcpy(songpath, gamedir);
    strcat(songpath, "/");
    strcat(songpath, songname);
    strcat(songpath, ".ogg");
    printf("songpath: '%s'\n", songpath);
    FILE* songfile = fopen( songpath, "w");
 
    curl_easy_setopt(songcurl, CURLOPT_WRITEDATA, songfile);
    
    songerr = curl_easy_perform(songcurl);
    if(!songerr) {
        //should have song written 
    }
    else {
        fprintf(stderr, "%s\n", songcurl_errbuf);
    }

    /* clean-up */
    curl_easy_cleanup(songcurl);
    fclose(songfile);

    return songerr;
}
