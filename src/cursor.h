#include "bufferManager.h"
/**
 * @brief The cursor is an important component of the system. To read from a
 * table, you need to initialize a cursor. The cursor reads rows from a page one
 * at a time.
 *
 */
class Cursor
{
public:
    Page page;
    int pageIndex;
    string Name;
    int pagePointer;
    int rowPageIndex;
    int columnPageIndex;
    

public:
    Cursor(string Name, int pageIndex);
    Cursor(string Name, int rowPageIndex, int columnPageIndex);
    vector<int> getNext();
    vector<int> getNextExport();
    vector<int> getNextTranspose();
    void nextPage(int pageIndex);
    void nextPage(int rowPageIndex,int columnPageIndex);
    
};