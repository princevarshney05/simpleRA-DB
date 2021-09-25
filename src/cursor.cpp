#include "global.h"

Cursor::Cursor(string Name, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(Name, pageIndex);
    this->pagePointer = 0;
    this->Name = Name;
    this->pageIndex = pageIndex;
    
}

Cursor::Cursor(string Name, int rowPageIndex, int columnPageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(Name, rowPageIndex,columnPageIndex);
    this->pagePointer = 0;
    this->Name = Name;
    this->rowPageIndex = rowPageIndex;
    this->columnPageIndex = columnPageIndex;
}





/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> Cursor::getNext()
{
    logger.log("Cursor::getNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if (result.empty())
    {
        matrixCatalogue.getMatrix(this->Name)->getNextPage(this);
        if (!this->pagePointer)
        {
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
vector<int> Cursor::getNextExport()
{
    logger.log("Cursor::getNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if (result.empty())
    {
        matrixCatalogue.getMatrix(this->Name)->getNextPageExport(this);
        if (!this->pagePointer)
        {
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
vector<int> Cursor::getNextTranspose()
{
    logger.log("Cursor::getNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    return result;
}

/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->Name, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

void Cursor::nextPage(int rowPageIndex,int columnPageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->Name, rowPageIndex,columnPageIndex);
    this->rowPageIndex = rowPageIndex;
    this->columnPageIndex = columnPageIndex;
    this->pagePointer = 0;
}