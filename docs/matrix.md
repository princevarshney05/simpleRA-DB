# Dense Matrix

## Page layout

A n x n matrix is split into multiple submatrices. The maximum dimension of each sub matrix is determined by maximum elements that can be stored in a single page of size `BLOCK_SIZE`. If a page of given `BLOCK_SIZE` could store at most `e` elements given by `(BLOCK_SIZE*1000)/sizeof(int)`, the maximum dimension of each submatrix would be `sqrt(e)`.

If we consider a square matrix of dimension 100 x 100 to be stored into pages with `BLOCK_SIZE = 8kb`, the respective page design would look like :

Considering each integer takes 4 bytes, the maximum number of elements that can fit in a page is 8000bytes/4bytes = 2000. Taking square root of the value, we end up with 44, which essentially means we could store a square matrix of max dimension 44 x 44 in a single page. We split the rows and columns into chunks of 44 respectively to obtain a submatrix. In the case of 100 x 100,rows and columns gets divided into chunks of 44 , 44 and 12 respectively. The following table visualises this example :

| -              | 0(44 columns)      | 1(44 columns)      | 3(12 columns)      |
| -------------- | ------------------ | ------------------ | ------------------ |
| **0(44 rows)** | Page_0_0 : 44 x 44 | Page_0_1 : 44 x 44 | Page_0_2 : 44 x 12 |
| **1(44 rows)** | Page_1_0 : 44 x 44 | Page_1_1 : 44 x 44 | Page_1_2 : 44 x 12 |
| **3(12 rows)** | Page_2_0 : 12 x 44 | Page_2_1 : 12 x 44 | Page_2_2 : 12 x 12 |

As we can see, the entire matrix gets subdivided into 9 pages, named Page_i_j. Number of rows and columns in each page is bounded by 44.

## Inplace Tranpose

To transpose the matrix, we first transpose each of the submatrix indicated by Page_i_j.We then swap Page_i_j with Page_j_i for all pages where i>j.
In the above example, the following page swaps take place, after the submatrix in each of the pages are individually transposed:

- Page_1_0 <-> Page_0_1
- Page_2_0 <-> Page_0_2
- Page_2_1 <-> Page_1_2

We ensure that exactly 2 MM blocks (Pages) are accessed at any given time. One can conclude from the above operations that the entire matrix is swapped.

# Sparse Matrix

## Page layout

For Sparse matrices with sparse paercentage >= 60%, we store each non zero value in a separate row in the following format :
<row_number, column_number, non zero value>

## Inplace Transpose

In case of transpose, we interchange the row_number and column_number for each row in all the pages.

For eg :
for each row in page,
<row_number, column_number, non zero value> changes to <column_number, row_number, non zero value>

The transpose of sparse matrix is in-place accesses at 1 MM block(Page) at any given time.

# Compression ratio

- Considering 8kb Page and 4 bytes for storing int.
- No. of Matrix elements stored in 1 Page of our dense matrix design `NO_DENSE` is given by 8000/4=2000(In reality, we are only storing 1936).
- No. of Matrix elements stored in 1 Page of our sparse matrix design `NO_SPARSE` is given by 8000/(4\*3) ~= 666(for every element we are storing three integers).
- No. of Pages to store a nxn dense matrix is given by `ceil((n*n)/NO_DENSE)`.
- No. of Pages to store a nxn sparse matrix is given by `ceil((n*n*(1-SPARSE_FACTOR))/NO_SPARSE)` where SPARSE_FACTOR is the percentage of 0 elements in the matrix.
- In the following table, we compare No. of Pages required to store dense matrix and sparse matrix for various sizes of matrices.
  |Matrix|Dense|Sparse 60%|Sparse 75%|Sparse 90%|Sparse 95%|Sparse 99%|
  | ---- | --- | -------- | -------- | -------- | -------- | -------- |
  |10x10|1|1|1|1|1|1|
  |100x100|5|7|4|2|1|1|
  |1000x10000|500|601|376|151|76|16|
  |100000x100000|50000|60061|37538|15016|7508|1502|
- For storing a element in sparse matrix we are storing 3 numbers in page, On the other hand, to store a element in dense matrix we just store a single number in page. Therefore, when sparse percentage is low i.e more number of elements in the matrix we end up taking more pages in sparse matrix than dense matrix(compare Sparse 60% with dense). From sparse 75%, we can see the benefits of storing sparse matrixes differently from dense matrix.
