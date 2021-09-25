#include "global.h"

// [debug] :edit /add /remove functions as per need for matrix
/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief Construct a new Matrix:: Matrix object used when an assignment command
 * is encountered. To create the matrix object both the matrix name and the
 * columns the matrix holds should be specified.
 *
 * @param matrixName 
 * @param columns 
 */
Matrix::Matrix(string matrixName, vector<string> columns)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/temp/" + matrixName + ".csv";
    this->matrixName = matrixName;
    this->columns = columns;
    this->columnCount = columns.size();
    this->maxRowsPerBlock = (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
    // this->writeRow<string>(columns);
}
// [debug] : edit below
/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates matrix
 * statistics.
 *
 * @return true if the matrix has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (countColumns(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

bool Matrix::countColumns(string firstLine)
{
    logger.log("Matrix:countColumns");
    string word;
    stringstream s(firstLine);
    int count = 0;
    while (getline(s, word, ','))
    {
        count++;
    }
    // cout << "There are" << count << " columns in the matrix" << endl;
    this->columnCount = count;
    
    this->maxElementsPerBlock = (uint)((BLOCK_SIZE * 1000) / sizeof(int));
    this->maxDimension=(uint)sqrt(this->maxElementsPerBlock);
    this->maxRowsPerBlock = this->maxDimension;
    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */

bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<int> rowOfSubmatrix(this->maxDimension);
    int submatrixCounter = 0;
    int rowPageIndex = 0;
    int columnPageIndex = 0;
    
    while (getline(fin, line)){
        stringstream s(line);
        columnPageIndex = 0;
        if((this->rowCount % this->maxDimension == 0) and (this->rowCount != 0)){
            rowPageIndex += 1;
        }
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++){
            if(submatrixCounter == this->maxDimension){
                bufferManager.writeMatrixPage(this->matrixName, rowPageIndex,columnPageIndex,rowOfSubmatrix, submatrixCounter);
                this->columnsPerBlockCount[{rowPageIndex,columnPageIndex}] = submatrixCounter;
                this->rowsPerBlockCount[{rowPageIndex,columnPageIndex}] += 1;
                columnPageIndex++;
                submatrixCounter = 0;
            }
            if (!getline(s, word, ','))
            {
                return false;
            }
            rowOfSubmatrix[submatrixCounter++] = stoi(word);
        }
        if(submatrixCounter){
            bufferManager.writeMatrixPage(this->matrixName, rowPageIndex,columnPageIndex,rowOfSubmatrix, submatrixCounter);
            this->columnsPerBlockCount[{rowPageIndex,columnPageIndex}] = submatrixCounter;
            this->rowsPerBlockCount[{rowPageIndex,columnPageIndex}] += 1;
            columnPageIndex++;
            submatrixCounter = 0;
            
        }
        this->rowCount++;
        
    }
    this->blockCount = this->rowsPerBlockCount.size();
    this->rowBlockCount = ceil(this->rowCount / (double)this->maxDimension);
    this->columnBlockCount = rowBlockCount;
    if (this->rowCount == 0)
        return false;
    
    return true;
    

}

// bool Matrix::blockify()
// {
    
//     logger.log("Matrix::blockify");
//     ifstream fin(this->sourceFileName, ios::in);
//     string line, word;
//     vector<int> elementsInBlock(this->maxElementsPerBlock, 0);
//     int pageCounter = 0;
//     int elementsInBlockCounter = 0;

//     while (getline(fin, line)){
//         stringstream s(line);
//         for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++){
//             if(elementsInBlockCounter == this->maxElementsPerBlock){
//                 bufferManager.writeMatrixPage(this->matrixName, this->blockCount, elementsInBlock, pageCounter,elementsInBlockCounter);
//                 this->blockCount++;
//                 this->rowsPerBlockCount.emplace_back(pageCounter);
//                 pageCounter = 0;
//                 elementsInBlockCounter = 0;
//             }
//             if (!getline(s, word, ','))
//             {
                
//                 return false;
//             }
//             elementsInBlock[elementsInBlockCounter++] = stoi(word);

//         }
//         pageCounter++;
//         this->rowCount++;
//     }
//     if(pageCounter != 0){
//         bufferManager.writeMatrixPage(this->matrixName, this->blockCount, elementsInBlock, pageCounter,elementsInBlockCounter);
//         this->blockCount++;
//         this->rowsPerBlockCount.emplace_back(pageCounter);
//         pageCounter = 0;
//         elementsInBlockCounter = 0;
//     }
//     if (this->rowCount == 0)
//         return false;
    
//     return true;


// }
// bool Matrix::blockify()
// {
//     logger.log("Matrix::blockify");
//     ifstream fin(this->sourceFileName, ios::in);
//     string line, word;
//     vector<int> row(this->columnCount, 0);
//     vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
//     int pageCounter = 0;
//     unordered_set<int> dummy;
//     dummy.clear();
//     this->distinctValuesInColumns.assign(this->columnCount, dummy);
//     this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
//     // getline(fin, line);
//     while (getline(fin, line))
//     {
//         stringstream s(line);
//         for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
//         {
//             if (!getline(s, word, ','))
//             {
//                 // cout << word << " -- false hogya matrix" << endl;
//                 return false;
//             }
//             // cout << word << endl;
//             row[columnCounter] = stoi(word);
//             rowsInPage[pageCounter][columnCounter] = row[columnCounter];
//         }
//         pageCounter++;
//         this->updateStatistics(row);
//         if (pageCounter == this->maxRowsPerBlock)
//         {
//             bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
//             this->blockCount++;
//             this->rowsPerBlockCount.emplace_back(pageCounter);
//             pageCounter = 0;
//         }
//     }
//     cout << this->maxRowsPerBlock << endl;
//     if (pageCounter)
//     {
//         bufferManager.writePage(this->matrixName, this->blockCount, rowsInPage, pageCounter);
//         this->blockCount++;
//         this->rowsPerBlockCount.emplace_back(pageCounter);
//         pageCounter = 0;
//     }

//     if (this->rowCount == 0)
//         return false;
//     this->distinctValuesInColumns.clear();
//     return true;
// }

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row 
 */
void Matrix::updateStatistics(vector<int> row)
{
    this->rowCount++;
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (!this->distinctValuesInColumns[columnCounter].count(row[columnCounter]))
        {
            this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
            this->distinctValuesPerColumnCount[columnCounter]++;
        }
    }
}

/**
 * @brief Function prints the first few rows of the matrix. If the matrix contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    uint count = min((long long)PRINT_COUNT, this->rowCount);

    //print headings(not required for matrix)
    // this->writeRow(this->columns, cout);

    Cursor cursor(this->matrixName, 0, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < count; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRow(row, cout);
    }
    printRowCount(this->rowCount);
}

/**
 * @brief This function returns one row of the matrix using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor 
 * @return vector<int> 
 */
void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if (cursor->rowPageIndex < this->rowBlockCount - 1)
    {
        cursor->nextPage(cursor->rowPageIndex + 1,0);
    }
}

void Matrix::getNextPageExport(Cursor *cursor)
{
    logger.log("Matrix::getNext");

    if(cursor->columnPageIndex < this->columnBlockCount -1){
        cursor->nextPage(cursor->rowPageIndex,cursor->columnPageIndex+1);
    }
    else if (cursor->rowPageIndex < this->rowBlockCount - 1)
    {
        cursor->nextPage(cursor->rowPageIndex + 1,0);
    }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent()
{
    logger.log("Matrix::makePermanent");
    if (!this->isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::out);

    //print headings
    this->writeRow(this->columns, fout);

    Cursor cursor(this->matrixName, 0, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        row = cursor.getNext();
        this->writeRowExport(row, fout);
    }
    fout.close();
}

/**
 * @brief Function to check if matrix is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent()
{
    logger.log("Matrix::isPermanent");
    if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
        return true;
    return false;
}

/**
 * @brief The unload function removes the matrix from the database by deleting
 * all temporary files created as part of this matrix
 *
 */
void Matrix::unload()
{
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
    if (!isPermanent())
        bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this matrix
 * 
 * @return Cursor 
 */
Cursor Matrix::getCursor()
{
    logger.log("Matrix::getCursor");
    Cursor cursor(this->matrixName, 0, 1);
    return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 * 
 * @param columnName 
 * @return int 
 */
int Matrix::getColumnIndex(string columnName)
{
    logger.log("Matrix::getColumnIndex");
    for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    {
        if (this->columns[columnCounter] == columnName)
            return columnCounter;
    }
}

void Matrix::transpose(){
    int rowPageIndex ;
    int columnPageIndex ;
    for(rowPageIndex =0;rowPageIndex<this->rowBlockCount;rowPageIndex++){
        for(columnPageIndex = 0;columnPageIndex<=rowPageIndex;columnPageIndex++){
            Cursor cursor1(this->matrixName,rowPageIndex,columnPageIndex);
            Cursor cursor2(this->matrixName, columnPageIndex,rowPageIndex);
            
            int p1_col = this->columnsPerBlockCount[{rowPageIndex,columnPageIndex}];
            int p1_row = this->rowsPerBlockCount[{rowPageIndex,columnPageIndex}];
            int p2_col = this->columnsPerBlockCount[{columnPageIndex,rowPageIndex}];
            int p2_row = this->rowsPerBlockCount[{columnPageIndex,rowPageIndex}];
            vector<vector<int>> p1_t (p1_col,vector<int>(p1_row));
            vector<vector<int>> p2_t (p2_col,vector<int>(p2_row));

            vector<int> result;
            int j;
            for(int i=0;i<p1_row;i++){
                j = 0;
                result = cursor1.getNextTranspose();
                for(auto &r:result){
                    p1_t[j++][i] = r;
                }
            }
            for(int i=0;i<p2_row;i++){
                j = 0;
                result = cursor2.getNextTranspose();
                for(auto &r:result){
                    p2_t[j++][i] = r;
                }
            }
            bufferManager.writeMatrixPage(this->matrixName,rowPageIndex,columnPageIndex,p2_t);
            bufferManager.writeMatrixPage(this->matrixName,columnPageIndex,rowPageIndex,p1_t);
            bufferManager.emptyPages();

        }
    }
}

