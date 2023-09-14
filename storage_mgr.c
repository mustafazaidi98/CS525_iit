#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include storage_mgr.h

// manipulating page files
void initStorageManager(void) {
    printf("Storage Manager Initialized\n");
}


// Create a page file -> responsible for read and write

RC createPageFile(char *fileName) { 

    // Initializes file pointer within the function scope 
    FILE *newFilePtr = fopen(filename , "w+"); 

    // Check if the file was successfully created 
    if (newFilePtr == NULL) { 
        return RC_FILE_NOT_FOUND; 
    }

    // Allocate the buffer for a single page 
    char *pageBuffer = (char*) malloc(PAGE_SIZE)

    // Check if memory allocation was successful 
    if(pageBuffer == NULL){
        fclose(newFilePtr); 
        return RC_MEMORY_ALLOCATION_ERROR; 
    }

    // Initialize the page buffer with zeros 
    memset(pageBuffer, 0 , PAGE_SIZE); 

    // Write the buffer to file 
    size_t bytesWritten = fwrite(pageBuffer, sizeof(char) , PAGE_SIZE, newFilePtr); 

    // Check if all the writes are successful 
    if (bytesWritten != PAGE_SIZE){ 
        free(pageBuffer); 
        fclose(newFilePtr); 
        return RC_WRITE_FAILED; 
    }

    // Free the allocated memory and close the file
    free(pageBuffer); 
    fclose(newFilePtr); 

    return RC_OK;  

}



// Opens an existing page file and initializes the file handle 
RC openPageFile(char *fileName , SM_FileHandle *fileHandle){

    // Initialize local file pointer (function scope)
    FILE *localFile = fopen(fileName, "r+"); 

    // Check if the file was successfully opened 
    if (localFile == NULL){
        return RC_FILE_NOT_FOUND;
    }

    // Move to the end of the file 
    if(fseek(localFile, 0 , SEEK_END) != 0){
        fclose(localFile); 
        return RC_READ_NON_EXISTING_PAGE; 
    }

    // Get the file size 
    // using long to handle large files 
    long fileSize = ftell(localFile); 
    if (fileSize == -1){ 
        fclose(localFile); 
        return RC_READ_NON_EXISTING_PAGE; 
    }

    // Calculate the total number of pages here 
    int totalPageCount = (fileSize + 1) / PAGE_SIZE ;     // Updae the file handle unformation
    fileHandle-> mgmtInfo = localFile;
    fileHandle-> totalNumPages = totalPageCount; 
    fileHandle -> fileName = fileName; 
    fileHandle-> curPagePos = 0; 

    // Rewind the file pointer to the beginning 
    rewind(localFile); 

    return RC_OK; 
}


// Closes the page file and updates the file handle to an invalid state 
RC closePageFile(SM_FileHandle *fileHandle){

    if(fileHandle == NULL || fileHandle->mgmtInfo == NULL ) {
        return RC_FILE_HANDLE_NOT_INIT; 
    }

    // Close the file 
    if(fclose((FILE *) fileHandle-> mgmtInfo) != 0) {
        return RC_FILE_NOT_CLOSED; 
    }

    // Update the file handle to an invalid syntax 
    fileHandle-> mgmtInfo = NULL; 
    fileHandle-> totalNumPages = 0; 
    fileHandle-> curPagePos = -1; // set to an invalid position
    fileHandle-> fileName = NULL; // set to Null 

    return RC_OK;
     
}