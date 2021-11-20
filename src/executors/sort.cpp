#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    if((tokenizedQuery.size()!= 8 && tokenizedQuery.size() != 10) || tokenizedQuery[4] != "BY" || tokenizedQuery[6] != "IN"){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    if(tokenizedQuery.size() == 10){
        if(tokenizedQuery[8] == "BUFFER"){
            try{
                parsedQuery.sortBufferSize = stoi(tokenizedQuery[9]);
            }
            catch(...){
                cout << "SYNTAX ERROR: Buffer size should be a integer value." << endl;
            }
        }
    }
    else{
        parsedQuery.sortBufferSize = 10;
    }
    parsedQuery.queryType = SORT;
    parsedQuery.sortResultRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[3];
    parsedQuery.sortColumnName = tokenizedQuery[5];
    string sortingStrategy = tokenizedQuery[7];
    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnName, parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}

int sortColumnIndex;

bool compareRows(vector<int> &r1,vector<int> &r2){

    
    int val1 = r1[sortColumnIndex];
    int val2 = r2[sortColumnIndex];
    SortingStrategy strategy = parsedQuery.sortingStrategy;

    switch (strategy)
    {
        case ASC: return val1 < val2;
        case DESC: return val1 > val2;
    }
}

void blockify(vector<vector<int>> &rows,int partitionId, int rowLimit){
    int numberOfPages = ceil(rows.size()/(double)rowLimit);
    auto start = rows.begin();
    auto end = start + min(rowLimit,(int)(rows.end()-start));
    for(int i=0;i<numberOfPages;i++){
        vector<vector<int>> partition(start,end);
        bufferManager.writePage((string)"Partition"+to_string(partitionId),i,partition,partition.size());
        start = end;
        end =  start + min(rowLimit,(int)(rows.end()-start));
    }
}

void executeSORT(){
    logger.log("executeSORT");
    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);
    Table* resultantTable = new Table(parsedQuery.sortResultRelationName, table.columns);
    sortColumnIndex = table.getColumnIndex(parsedQuery.sortColumnName);
    Cursor cursor = table.getCursor();
    
    int counter = 0;
    int bs = parsedQuery.sortBufferSize;
    int sortingPhaseIterations = ceil(table.blockCount/(double)bs);
    int pageIndex = 0;
    while(counter < sortingPhaseIterations){
        int remainingPages = table.blockCount - pageIndex;
        int pagesToRead = min(remainingPages,bs);
        vector<vector<int>> rows;
        for(int pageCounter = 0;pageCounter < pagesToRead; pageCounter++){
            vector<vector<int>> temp;
            vector<int> row;
            for(int rowIndex=0;rowIndex<table.rowsPerBlockCount[pageIndex];rowIndex++){
                row = cursor.getNext();
                temp.push_back(row);
            }
            rows.insert(rows.end(),temp.begin(),temp.end());
            pageIndex += 1;
        }
        sort(rows.begin(),rows.end(),compareRows);
        blockify(rows,counter,table.maxRowsPerBlock);
        counter += 1;
        
    }

    return;
}

