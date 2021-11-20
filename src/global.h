#include "executor.h"

extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern uint bucket_rows_count;
extern uint bucket_columns_count;
extern uint bucket_maxRowsPerBlock_count;
extern uint BLOCK_ACCESS_READ;
extern uint BLOCK_ACCESS_WRITE;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern BufferManager bufferManager;