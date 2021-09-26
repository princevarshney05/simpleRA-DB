# Dense Matrix
## Page layout to store matrix
A m x m matrix is split into multiple submatrices. The maximum dimension of each sub matrix is determined by maximum elements that can be stored in a single page of size ```BLOCK_SIZE```. If a page of given ```BLOCK_SIZE``` could store at most ``` e ``` elements, the maximum dimension of each submatrix would be ```sqrt(e)```. 

If we consider a square matrix of dimension 100 x 100 to be stored into pages with ```BLOCK_SIZE = 8kb```, the respective page design would look like : 

Considering each integer takes 4 bytes, the maximum element that can fit in a page is 8000bytes/4bytes = 2000. Taking square root of the value, we end up with 44, which essentially means we could store a square matrix of max dimension 44 x 44 in a single page. We split the rows and columns into chunks of 44 respectively to obtain a submatrix. In the case of 100 x 100,rows and columns gets divided into chunks of 44 , 44 and 12 respectively. The following table visualises this example :


| -                   |  0(44 columns)    |  1(44 columns)  |   2(44 columns) |   3(12 columns) | 
| -----------         | -----------       | -----------     | -----------     |-----------      |
| **0(44 rows)**      | Page_0_0 : 44 x 44 |Page_0_1 : 44 x 44|Page_0_2 : 44 x 44|Page_0_3 : 44 x 12|
| **1(44 rows)**      | Page_1_0 : 44 x 44 |Page_1_1 : 44 x 44|Page_1_2 : 44 x 44|Page_1_3 : 44 x 12|
| **2(44 rows)**      | Page_2_0 : 44 x 44 |Page_2_1 : 44 x 44|Page_2_2 : 44 x 44|Page_2_3 : 44 x 12|
| **3(12 rows)**      | Page_3_0 : 12 x 44 |Page_3_1 : 12 x 44|Page_3_2 : 12 x 44|Page_3_3 : 12 x 12|

As we can see, the entire matrix gets subdivided into 16 pages, named Page_i_j , each of dimension i x j.

## Transpose from Page
To transpose the matrix, we first transpose each of the submatrix indicated by Page_i_j.We then swap Page_i_j with Page_j_i for all pages where i>j.
In the above example, the following page swaps take place, after the submatrix in each of the pages are individually transposed:
* Page_1_0 <-> Page_0_1
* Page_2_0 <-> Page_0_2
* Page_2_1 <-> Page_1_2
* Page_3_0 <-> Page_0_3
* Page_3_1 <-> Page_1_3
* Page_3_2 <-> Page_2_3

We can conclude from the above operations that the entire matrix is swapped.

# Sparse Matrix
## Page layout to store matrix
For Sparse matrices with sparse paercentage >= 60%, we store each non zero value in a separate row in the following format :
<row_number, column_number, non zero value>

## Transpose from Page
In case of transpose, we interchange the row_number and column_number for each row in all the pages.

For eg :
for each row in page,
<row_number, column_number, non zero value> changes to <column_number, row_number, non zero value>