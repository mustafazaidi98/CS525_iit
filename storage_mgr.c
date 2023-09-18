#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"


// manipulating page files
void initStorageManager(void) {
    printf("Storage Manager Initialized\n");
}
FILE *file;
// Create a page file -> responsible for read and write

RC createPageFile(char *fileName) { 

    // Initializes file pointer within the function scope 
    file = fopen(fileName , "w+"); 

    // Check if the file was successfully created 
    if (file == NULL) { 
        return RC_FILE_NOT_FOUND; 
    }

    // Allocate the buffer for a single page 
    char *pageBuffer = (char*) malloc(PAGE_SIZE);

    // Check if memory allocation was successful 
    if(pageBuffer == NULL){
        fclose(file); 
        return RC_WRITE_FAILED; 
    }

    // Initialize the page buffer with \0 
    memset(pageBuffer, '\0' , PAGE_SIZE); 

    // Write the buffer to file 
    size_t bytesWritten = fwrite(pageBuffer, sizeof(char) , PAGE_SIZE, file); 

    // Check if all the writes are successful 
    if (bytesWritten != PAGE_SIZE){ 
        free(pageBuffer); 
        fclose(file); 
        return RC_WRITE_FAILED; 
    }

    // Free the allocated memory and close the file
    free(pageBuffer); 
    fclose(file); 

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

// Writes a block of data to specific page in an open page file 

RC writeBlock(int pageNum , SM_FileHandle *fileHandle, SM_PageHandle memPage){

// Check if the file handle or memory page is null 
if(fileHandle == NULL || memPage == NULL){
    return RC_FILE_HANDLE_NOT_INIT; 
}

// Validate the page number is in the correct range
if (pageNum < 0 || pageNum >= fileHandle->totalNumPages){
    return RC_READ_NON_EXISTING_PAGE;
}

// Calculate the offset to know where to start writing 
long offset = pageNum * PAGE_SIZE;

// Set the file pointer to correct position
if (fseek(fileHandle->mgmtInfo, offset, SEEK_SET) != 0){
    return RC_WRITE_FAILED;
}

// Write the page to the file
size_t writeResult = fwrite(memPage, sizeof(char), PAGE_SIZE, fileHandle->mgmtInfo);

// Check if the writing was successful 
if(writeResult != PAGE_SIZE){
    return RC_WRITE_FAILED;
}

// Update the current page position 
fileHandle->curPagePos = pageNum;

return RC_OK; 


}

// Writes a block of data to te current page position in a open page file 

RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage){

    // Check if the file handle is null or if the current page position is invalid
    if(fileHandle == NULL){
        return RC_FILE_HANDLE_NOT_INIT; 
    }

    if (fileHandle->curPagePos < 0 ){
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Check if the memPage is null 
    if(memPage == NULL){
        return RC_WRITE_FAILED; 
    }

    // Call the writeBlock function to perform the actual write operation 
    RC writeStatus = writeBlock(fileHandle->curPagePos, fileHandle, memPage);

    // Check the return status of the writeBlock function   
    if(writeStatus != RC_OK){
        return writeStatus; // return the error code
    
    }

    return RC_OK;


}

// Write a function to append an empty block to the end of the page file

RC appendEmptyBlock(SM_FileHandle *fileHandle){

    // Validate if the file handle and its fileName field are initialized
    if(fileHandle == NULL || fileHandle->fileName == NULL){
        return RC_FILE_HANDLE_NOT_INIT; 
    }

    // Open the file in append mode
    FILE *appendFile = fopen(fileHandle->fileName, "a+");

    // Check if the file was opened successfully
    if(appendFile == NULL){
        return RC_FILE_NOT_FOUND; 
    }

    // Allocate an empty page buffer filled with zeros 
    char (*emptyPageBuffer) = (char*) calloc(PAGE_SIZE, sizeof(char));
    if(emptyPageBuffer == NULL){
        fclose(appendFile); 
        // Indicate that the memory allocation failed
        return RC_WRITE_FAILED; 
    }

    // Write the empty page to the end of the file 
    size_t writtenSize = fwrite(emptyPageBuffer, sizeof(char), PAGE_SIZE, appendFile);

    // Validate the number of bytes written
    if(writtenSize != PAGE_SIZE){
        fclose(appendFile); 
        free(emptyPageBuffer); 
        return RC_WRITE_FAILED; 
    }

    // Update the file handle's metadata
    fileHandle->totalNumPages + 1;
    fileHandle->curPagePos = fileHandle->totalNumPages; 
    
    // Close the file and release memory
    fclose(appendFile);
    free(emptyPageBuffer);

    return RC_OK;
}

// Write a function to ensure that the capacity of the page file is at least numberOfPages, appending empty blocks as needed
RC ensureCapacity(int numberOfPages, SM_FileHandle *fileHandle){

    // Check if the file handle is initialized 
    if(fileHandle == NULL || fileHandle->fileName == NULL){
        return RC_FILE_HANDLE_NOT_INIT; 
    }

    // Check if the current total number of pages is already sufficient
    if (fileHandle -> totalNumPages >= numberOfPages){
        return RC_OK; 
    }

    // Open the file in append mode 
    FILE *openFile = fopen(fileHandle->fileName, "a+");
    if(openFile == NULL){
        return RC_FILE_NOT_FOUND; 
    }

    // Loop to append enough empty blocks to the file 
    while(fileHandle->totalNumPages < numberOfPages){
        RC status = appendEmptyBlock(fileHandle);
        if(status != RC_OK){
            fclose(openFile); 
            return status; 
        }
    }

    // Close the file
    fclose(openFile);

    return RC_OK;

}

