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

struct my_comparator
{
    // queue elements are vectors so we need to compare those
    bool operator()(std::vector<int> const& r1, std::vector<int> const& r2) const
    {   
        int val1 = r1[sortColumnIndex];
        int val2 = r2[sortColumnIndex];
        SortingStrategy strategy = parsedQuery.sortingStrategy;

        switch (strategy)
        {
            case ASC: return val1 > val2;
            case DESC: return val1 < val2;
        }
        
    }
};

double logAnyBase(double x,double base){
    return log(x)/log(base);
}



void executeSORT(){
    logger.log("executeSORT");
    BLOCK_READ_ACCESS = 0;
    BLOCK_WRITE_ACCESS = 0;
    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);
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
        Table* partitionTable = new Table((string)"Partition"+to_string(counter), table.columns);
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

        for(auto &r:rows){
            partitionTable->writeRow(r);
        }

        if(partitionTable->blockify())
            tableCatalogue.insertTable(partitionTable);
        else{
            cout<<"Empty Table"<<endl;
            partitionTable->unload();
            delete partitionTable;
        }
        counter += 1;
        
    }


    int mergingPhaseIterations = ceil(logAnyBase((double)sortingPhaseIterations,(double)(bs-1)));
    counter = 0;
    int numberOfSortedFilesOld = sortingPhaseIterations;
    int numberOfSortedFilesNew;
    
    
    using pq = priority_queue<vector<int>,vector<vector<int>>,my_comparator>;
    while(counter < mergingPhaseIterations){
        numberOfSortedFilesNew = ceil(numberOfSortedFilesOld/(double)(bs-1));
        int partitionIndex = 0;
        int innerCounter = 0;
        while(innerCounter<numberOfSortedFilesNew){
            string tablename;
            if(counter == mergingPhaseIterations-1)
                tablename = parsedQuery.sortResultRelationName;
            else 
                tablename = (string)"Merge"+to_string(counter)+to_string(innerCounter);
            Table* partitionTable = new Table(tablename, table.columns);

            int remainingFiles = numberOfSortedFilesOld - partitionIndex;
            int filesToRead = min(remainingFiles,bs-1);

        
            vector<Table> oldTables(filesToRead);
            vector<Cursor> oldTablesCursor(filesToRead);
            vector<bool> completed(filesToRead);
            
            for(int i = 0;i<filesToRead;i++){
                if(counter == 0)
                    oldTables[i] = *tableCatalogue.getTable((string)"Partition"+to_string(partitionIndex));
                else 
                     oldTables[i] = *tableCatalogue.getTable((string)"Merge"+to_string(counter-1)+to_string(partitionIndex));
                oldTablesCursor[i] = oldTables[i].getCursor();
                completed[i] = false;
                partitionIndex++;
            }
            pq potentialRows;
            vector<int> row;
            while(count(completed.begin(),completed.end(),false) != 0){
                
                
                for(int i=0;i<filesToRead;i++){
                    if(!completed[i]){
                        row = oldTablesCursor[i].getNext();
                        if(row.empty()){
                            completed[i] = true;
                        }
                        else{
                            potentialRows.push(row);
                        }
                    }
                }
                if(!potentialRows.empty()){
                    partitionTable->writeRow(potentialRows.top());
                    potentialRows.pop();
                }
                
            }

            while(!potentialRows.empty()){
                partitionTable->writeRow(potentialRows.top());
                potentialRows.pop();
            }

            for(int i=0;i<filesToRead;i++){
                oldTables[i].unload();
            }


            if(partitionTable->blockify())
                tableCatalogue.insertTable(partitionTable);
            else{
                cout<<"Empty Table"<<endl;
                partitionTable->unload();
                delete partitionTable;
            }
            innerCounter++;
        }
        numberOfSortedFilesOld = numberOfSortedFilesNew;
        counter++;
    }
    cout<<"Blocks Read : "<<BLOCK_READ_ACCESS<<endl;
    cout<<"Blocks Written : "<<BLOCK_WRITE_ACCESS<<endl;
    cout<<"Total blocks Accessed : "<<BLOCK_READ_ACCESS+ BLOCK_WRITE_ACCESS<<endl;
    return;
}

