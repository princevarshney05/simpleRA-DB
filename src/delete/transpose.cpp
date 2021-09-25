#include <bits/stdc++.h>
using namespace std;

vector<vector<int>> transpose(string pageName)
{
    ifstream pg1(pageName);
    string line, word;
    int row_count = 0;
    int column_count = 0;

    int flag = 1;
    while (getline(pg1, line))
    {
        stringstream s(line);
        row_count++;
        if (flag == 1)
        {
            while (getline(s, word, ' '))
            {
                flag = 0;
                column_count++;
            }
        }
    }
    pg1.close();

    // cout << column_count << " ";
    ifstream pg1_new(pageName);
    vector<int> row(column_count, 0);
    vector<vector<int>> matrix(row_count, row);

    int r_count = 0, col_count = 0;
    while (getline(pg1_new, line))
    {
        stringstream s(line);
        col_count = 0;
        while (getline(s, word, ' '))
        {
            // cout << stoi(word) << endl;
            matrix[r_count][col_count] = stoi(word);
            col_count++;
        }
        r_count++;
    }

    for (int i = 0; i < row_count; i++)
    {
        for (int j = 0; j < column_count; j++)
        {
            if (i > j)
            {
                int temp = matrix[i][j];
                matrix[i][j] = matrix[j][i];
                matrix[j][i] = temp;
            }
            else
                break;
        }
    }

    pg1_new.close();

    // cout << matrix.size() << endl;
    // cout << matrix[0].size() << endl;
    // cout << column_count << endl;
    // cout << row_count << endl;

    return matrix;
}

void swap_pages(string pageName1, vector<vector<int>> p1, string pageName2, vector<vector<int>> p2)
{
    ofstream fout1(pageName1, ios::trunc);
    ofstream fout2(pageName2, ios::trunc);

    for (int i = 0; i < p2.size(); i++)
    {
        for (int j = 0; j < p2[0].size(); j++)
        {
            fout1 << p2[i][j] << " ";
        }
        fout1 << endl;
    }
    for (int i = 0; i < p1.size(); i++)
    {
        for (int j = 0; j < p1[0].size(); j++)
        {
            fout2 << p1[i][j] << " ";
        }
        fout2 << endl;
    }
}

int main()
{
    string pg1 = "l1";
    string pg2 = "l2";
    vector<vector<int>> page1_transpose = transpose(pg1);
    vector<vector<int>> page2_transpose = transpose(pg2);
    swap_pages(pg1, page1_transpose, pg2, page2_transpose);
}