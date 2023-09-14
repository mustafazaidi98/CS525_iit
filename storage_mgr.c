#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"


// manipulating page files
void initStorageManager(void) {
    printf("Storage Manager Initialized\n");
}

// Create a page file -> responsible for read and write

RC createPageFile(char *fileName) { 

    // Initializes file pointer within the function scope 
    FILE *newFilePtr = fopen(fileName , "w+"); 

    // Check if the file was successfully created 
    if (newFilePtr == NULL) { 
        return RC_FILE_NOT_FOUND; 
    }

    // Allocate the buffer for a single page 
    char *pageBuffer = (char*) malloc(PAGE_SIZE);

    // Check if memory allocation was successful 
    if(pageBuffer == NULL){
        fclose(newFilePtr); 
        return RC_WRITE_FAILED; 
    }

    // Initialize the page buffer with \0 
    memset(pageBuffer, '\0' , PAGE_SIZE); 

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
    // Updae the file handle unformation
    int totalPageCount = (fileSize + 1) / PAGE_SIZE ;     
    fileHandle->mgmtInfo = localFile;
    fileHandle->totalNumPages = totalPageCount; 
    fileHandle->fileName = fileName; 
    fileHandle->curPagePos = 0; 

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
    if(fclose((FILE *) fileHandle->mgmtInfo) != 0) {
        return RC_WRITE_FAILED; 
    }

    // Update the file handle to an invalid syntax 
    fileHandle->mgmtInfo = NULL; 
    fileHandle->totalNumPages = 0; 
    fileHandle->curPagePos = -1; // set to an invalid position
    fileHandle->fileName = NULL; // set to Null 

    return RC_OK;

}



RC destroyPageFile(char *fileToDestroy){ 
    FILE *localFilePtr = fopen(fileToDestroy, "r");

    // Check if the file exists by trying to open it
    if(localFilePtr ==NULL){ 
        return RC_FILE_NOT_FOUND; 
    }

    // Close the file if it was successfully opened 
    fclose(localFilePtr); 

    // Proceed to remove the file 
    if (remove(fileToDestroy) != 0){
        return RC_WRITE_FAILED;
    }

    return RC_OK; 
}


//The method reads the block at position pageNum from a file and stores its content in the memory pointed
//to by the memPage page handle.
//If the file has less than pageNum pages, the method should return RC READ NON EXISTING PAGE.
RC readBlock(int pageNum,SM_FileHandle *fHandle,SM_PageHandle memPage){
//if the given page number exceeds the total pages, or is less than zero                 
	if(pageNum<0||(*fHandle).totalNumPages<pageNum){
		return RC_READ_NON_EXISTING_PAGE;
	}
    	FILE *file;
		float memPageSize;
		//moving file start pointer to required page
		fseek(file,pageNum*PAGE_SIZE, SEEK_SET);
		//copying page to memory
		memPageSize = fread(memPage,sizeof(char),PAGE_SIZE,file);
		//checking if any anomaly
		if(memPageSize!=PAGE_SIZE){
			return RC_READ_NON_EXISTING_PAGE;
		}
		//updating current page pointer
		(*fHandle).curPagePos = pageNum;
		return RC_OK;
}

//Return the current page position in a file
int getBlockPos(SM_FileHandle *fHandle)
{
	if (fHandle == NULL)
       return RC_FILE_NOT_FOUND;
	return (*fHandle).curPagePos;
}

//Read the first page in a file
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL)
	   return RC_FILE_NOT_FOUND;
	return readBlock(0, fHandle, memPage);
}
//Read the last page in a file
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL)
	   return RC_FILE_NOT_FOUND;
	return readBlock((*fHandle).totalNumPages-1, fHandle, memPage);
}
//Read the previous page relative to the curPagePos
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL)
		return RC_FILE_NOT_FOUND;
	return readBlock((*fHandle).curPagePos - 1, fHandle, memPage);
}
//Read the page at curPagePos
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL)
		return RC_FILE_NOT_FOUND;
	return readBlock((*fHandle).curPagePos, fHandle, memPage);
}
//Read the next page relative to the curPagePos
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
	if (fHandle == NULL)
		return RC_FILE_NOT_FOUND;
	return readBlock((*fHandle).curPagePos + 1, fHandle, memPage);
}
