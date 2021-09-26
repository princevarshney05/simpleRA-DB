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
    isSparse();
    if (this->sparse)
    {
        //load operation for sparse matrix
        logger.log("Matrix::load");
        if (this->blockifySparse())
            return true;
        return false;
    }
    else
    {
        //load operation for non sparse matrix
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
}

void Matrix::isSparse()
{
    logger.log("Matrix:isSparse");

    //get row count
    fstream fin(this->sourceFileName, ios::in);
    string line, word;
    int total_rows = 0;
    int total_cols = 0;

    while (getline(fin, line))
    {
        total_rows++;
    }
    fin.close();

    //get column count
    fstream fin1(this->sourceFileName, ios::in);
    if (getline(fin1, line))
    {
        stringstream s(line);
        while (getline(s, word, ','))
        {
            total_cols++;
        }
    }
    fin1.close();

    //check sparse
    int non_zero_elements = 0;
    fstream fin2(this->sourceFileName, ios::in);
    while (getline(fin2, line))
    {
        stringstream s(line);
        while (getline(s, word, ','))
        {
            if ((stoi(word)) == 0)
            {
                non_zero_elements++;
            }
        }
    }
    fin2.close();

    //calculate sparse %
    double sparse_percentage = (double)non_zero_elements / (double)(total_cols * total_rows);
    if (sparse_percentage * 100 >= 60)
    {
        cout << "Sparse matrix : ";
        cout << "Sparce percentage : " << sparse_percentage * 100 << " %" << endl;
        this->total_non_zero_elements = non_zero_elements;
        this->rowCount = total_rows;
        this->columnCount = total_cols;
        this->sparse = true;
    }
    else
    {
        cout << "NOT Sparse matrix" << endl;
        this->sparse = false;
    }
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
    this->maxDimension = (uint)sqrt(this->maxElementsPerBlock);
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

    while (getline(fin, line))
    {
        stringstream s(line);
        columnPageIndex = 0;
        if ((this->rowCount % this->maxDimension == 0) and (this->rowCount != 0))
        {
            rowPageIndex += 1;
        }
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (submatrixCounter == this->maxDimension)
            {
                bufferManager.writeMatrixPage(this->matrixName, rowPageIndex, columnPageIndex, rowOfSubmatrix, submatrixCounter);
                this->columnsPerBlockCount[{rowPageIndex, columnPageIndex}] = submatrixCounter;
                this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}] += 1;
                columnPageIndex++;
                submatrixCounter = 0;
            }
            if (!getline(s, word, ','))
            {
                return false;
            }
            rowOfSubmatrix[submatrixCounter++] = stoi(word);
        }
        if (submatrixCounter)
        {
            bufferManager.writeMatrixPage(this->matrixName, rowPageIndex, columnPageIndex, rowOfSubmatrix, submatrixCounter);
            this->columnsPerBlockCount[{rowPageIndex, columnPageIndex}] = submatrixCounter;
            this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}] += 1;
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

bool Matrix::blockifySparse()
{
    logger.log("Matrix::blockifySparse");
    this->maxElementsPerBlock = (uint)((BLOCK_SIZE * 1000) / sizeof(int));
    this->maxRowsPerBlock = (uint)this->maxElementsPerBlock / 3; // 3 elements in each row: row_num, col_num, value
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    int rowPageIndex = 0;
    int columnPageIndex = 0; // remains zero for sparse ,since column size is fixed(3) for all pages
    int non_zero_elements = 0;
    vector<int> rowOfPage(3); // each row of Page has 3 columns
    int row_number = 0;       //this->rowcount is previously calculated for sparse matrix in isSparse(), so taking another variable
    while (getline(fin, line))
    {
        stringstream s(line);
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (non_zero_elements == this->maxRowsPerBlock)
            {
                rowPageIndex += 1;
                non_zero_elements = 0;
            }
            if (!getline(s, word, ','))
            {
                return false;
            }
            if (!stoi(word) == 0)
            {
                rowOfPage[0] = row_number;
                rowOfPage[1] = columnCounter;
                rowOfPage[2] = stoi(word);
                bufferManager.writeMatrixPage(this->matrixName, rowPageIndex, columnPageIndex, rowOfPage, 3);
                this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}] += 1;
                this->columnsPerBlockCount[{rowPageIndex, columnPageIndex}] = 3;
                non_zero_elements++;
            }
        }

        row_number++;
    }
    this->rowBlockCount = rowPageIndex;
    this->columnBlockCount = 1;

    return true;
}

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
        cursor->nextPage(cursor->rowPageIndex + 1, 0);
    }
}

void Matrix::getNextPageExport1(Cursor *cursor, int pagePointer)
{
    logger.log("Matrix::getNext");

    if (cursor->columnPageIndex < this->columnBlockCount - 1)
    {
        cursor->nextPage(cursor->rowPageIndex, cursor->columnPageIndex + 1, pagePointer);
    }
    else if (pagePointer < this->maxDimension - 1)
    {
        cursor->nextPage(cursor->rowPageIndex, 0, pagePointer + 1);
    }
    else if (cursor->rowPageIndex < this->rowBlockCount - 1)
    {
        cursor->nextPage(cursor->rowPageIndex + 1, 0, 0);
    }
}
void Matrix::getNextPageExport2(Cursor *cursor, int pagePointer)
{
    logger.log("Matrix::getNext");

    if (cursor->rowPageIndex < this->rowBlockCount - 1)
    {
        cursor->nextPage(cursor->rowPageIndex + 1, 0, 0);
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

    Cursor cursor(this->matrixName, 0, 0);
    vector<int> row;
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int colCounter = 0; colCounter < this->columnBlockCount; colCounter++)
        {
            row = cursor.getNextExport(rowCounter % this->maxDimension);
            this->writeRowExport(row, fout);
            if (colCounter != this->columnBlockCount - 1)
                fout << ",";
        }
        fout << endl;
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
 * @brief Function that returns the index of columnm indicated by columnName
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

void Matrix::transpose()
{
    if (this->sparse)
    {
        logger.log("Matrix:sparseTranspose");
        int rowPageIndex, columnPageIndex = 0;
        for (rowPageIndex = 0; rowPageIndex <= this->rowBlockCount; rowPageIndex++)
        {
            Cursor cursor(this->matrixName, rowPageIndex, columnPageIndex);
            int i, j;
            vector<int> result;
            vector<vector<int>> page_rows(this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}], vector<int>(3));

            for (i = 0; i < this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}]; i++)
            {
                result = cursor.getNextTranspose();
                j = 0;
                for (auto &r : result)
                {
                    page_rows[i][j++] = r;
                }
            }
            int temp;
            for (int i = 0; i < page_rows.size(); i++)
            {
                temp = page_rows[i][0];
                page_rows[i][0] = page_rows[i][1];
                page_rows[i][1] = temp;
            }

            bufferManager.writeMatrixPage(this->matrixName, rowPageIndex, columnPageIndex, page_rows);
            bufferManager.emptyPages();
        }
        cout << "SPARSE MATRIX TRANSPOSED" << endl;
    }
    else
    {
        logger.log("Matrix:transpose");
        // cout << "Not Sparse" << endl;
        int rowPageIndex;
        int columnPageIndex;
        for (rowPageIndex = 0; rowPageIndex < this->rowBlockCount; rowPageIndex++)
        {
            for (columnPageIndex = 0; columnPageIndex <= rowPageIndex; columnPageIndex++)
            {
                Cursor cursor1(this->matrixName, rowPageIndex, columnPageIndex);
                Cursor cursor2(this->matrixName, columnPageIndex, rowPageIndex);

                int p1_col = this->columnsPerBlockCount[{rowPageIndex, columnPageIndex}];
                int p1_row = this->rowsPerBlockCount[{rowPageIndex, columnPageIndex}];
                int p2_col = this->columnsPerBlockCount[{columnPageIndex, rowPageIndex}];
                int p2_row = this->rowsPerBlockCount[{columnPageIndex, rowPageIndex}];
                vector<vector<int>> p1_t(p1_col, vector<int>(p1_row));
                vector<vector<int>> p2_t(p2_col, vector<int>(p2_row));

                vector<int> result;
                int j;
                for (int i = 0; i < p1_row; i++)
                {
                    j = 0;
                    result = cursor1.getNextTranspose();
                    for (auto &r : result)
                    {
                        p1_t[j++][i] = r;
                    }
                }
                for (int i = 0; i < p2_row; i++)
                {
                    j = 0;
                    result = cursor2.getNextTranspose();
                    for (auto &r : result)
                    {
                        p2_t[j++][i] = r;
                    }
                }
                bufferManager.writeMatrixPage(this->matrixName, rowPageIndex, columnPageIndex, p2_t);
                bufferManager.writeMatrixPage(this->matrixName, columnPageIndex, rowPageIndex, p1_t);
                bufferManager.emptyPages();
            }
        }
        cout << "MATRIX TRANSPOSED" << endl;
    }
}
