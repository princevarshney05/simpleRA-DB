// #include "cursor.h"

// [debug] : Indexing strategy not needed for matrix (?)
// enum IndexingStrategy
// {
//     BTREE,
//     HASH,
//     NOTHING
// };

//[debug] change below @brief for matrix
/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */

// [debug] : remove unwanted functions from below class
// [debug] : add additional functions required for matrix
class Matrix
{
    vector<unordered_set<int>> distinctValuesInColumns;

public:
    string sourceFileName = "";
    string matrixName = "";
    vector<string> columns;
    vector<uint> distinctValuesPerColumnCount;
    uint columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint rowBlockCount = 0;
    uint columnBlockCount = 0;
    uint maxRowsPerBlock = 0;
    uint maxElementsPerBlock=0;
    uint maxDimension=0;
    map<pair<uint,uint>,uint> rowsPerBlockCount;
    map<pair<uint,uint>,uint> columnsPerBlockCount;
    bool indexed = false;
    string indexedColumn = "";
    bool sparse = false; //true , if matrix is sparse(more than 60% elements are 0)

    bool countColumns(string firstLine);
    bool blockify();
    void updateStatistics(vector<int> row);
    Matrix();
    Matrix(string matrixName);
    Matrix(string matrixName, vector<string> columns);
    bool load();
    bool isColumn(string columnName);
    void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void transpose();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    void getNextPageExport1(Cursor *cursor,int pagePointer);
    void getNextPageExport2(Cursor *cursor,int pagePointer);
    Cursor getCursor();
    int getColumnIndex(string columnName);
    void unload();
    void isSparse(); //checks whether matrix is sparse

    /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
    // [debug] : change below for matrix printing/other op.

    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        int count = min((int)row.size(),20);
        for (int columnCounter = 0; columnCounter < count; columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }

    template <typename T>
    void writeRowExport(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ",";
            fout << row[columnCounter];
        }
        
    }

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Matrix::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};