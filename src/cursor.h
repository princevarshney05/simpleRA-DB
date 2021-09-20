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
    int TorM; // 0 for table , 1 for matrix

public:
    Cursor(string Name, int pageIndex, int TorM);
    vector<int> getNext();
    void nextPage(int pageIndex);
};