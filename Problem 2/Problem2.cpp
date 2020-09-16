// Filipa Martins 2016267248
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std::chrono;
using namespace std;


void printPairs(vector <pair<double,double>>& v)
{
    for (size_t i = 0; i < v.size(); i++)
    {
        cout << std::fixed << std::setprecision(12);
        cout << v[i].first << " " << v[i].second << endl;
    }
}

void printMatrix(double ** P, int n, int k)
{
    cout << "MATRIX\n";
    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cout << std::fixed << std::setprecision(12);
            cout << P[i][j] << "  ";
        }
        cout << "\n";
    }
}

void readInput(vector<pair<double, double>>& pts, int& n, int& k)
{
    cin >> n >> k;
    for (int i = 0; i < n; i++)
    {   
        double x, y;
        cin >> x >> y;
        pts.push_back(make_pair(x, y));
    }
}

double area(pair<double, double>& p1)
{
    return  p1.first*p1.second; // x1*y1
}

double intersection(pair<double, double>& p1, pair<double, double>& p2)
{
    double x1 = p1.first, y2 = p2.second;
    return x1*y2; //intersectArea
}


double maximizeArea(double** P, vector<pair<double, double>>& pts, int n, int k){

	// P(i,j) is the subproblem of ﬁnding a subset of i(k) points from the set pj, ...., pn that 
    // contains pj and that maximizes the union of the area covered by those points.
    // P[i][j] stores the max area of subset of i(k) points from all points to j and necessarily contains j

    // if a subset of 1 item (k=1) and number of total points (j) is non-zero
    // we save the area of point j in P(i,j)
    for (int j = 0; j < n; j++)
		P[0][j] = area(pts[j]);
   
	// do for ith subset
	for (int i = 1; i < k; i++)
	{
		// consider all j from 1 to n
		for (int j = i; j < n; j++)
		{
            P[i][j] = 0;
            // find the max area subset that includes j point
            // consider previous subsets of k-1 points from set {j(k-1) ... j-1} (exclude point pj)
            for(int jj = i-1; jj < j; jj++){
                // max(actualMax, previousMax + areaActual - intersection(previous, actual))
                P[i][j] = max( P[i][j], P[i-1][jj] + P[0][j] - intersection(pts[jj], pts[j]) );
            }
		}
	}
    
	// return maximum value
    // | i = k-1 | k-1 <= j < n |
    double max_area = P[k-1][k-1];
    for(int j = k; j < n; j++)
        max_area = max( max_area, P[k-1][j] );

    return max_area;
}
   
int main()
{
    //auto start = high_resolution_clock::now();
    int n, k;
    double **P;
    vector<pair<double,double>> points;
   
    readInput(points, n, k);
    sort(points.begin(), points.end());
    
    P = new double*[k];
    for(int i = 0; i < k; i++)
        P[i] = new double[n];

    for (int i = 0; i < k; i++){
        for (int j = 0 ; j < n; j++){
            P[i][j] = 0;
        }
    }
		
    double max_area = maximizeArea(P, points, n, k);
    cout << std::fixed << std::setprecision(12) << max_area << "\n";              
       
    /*
    auto stop = high_resolution_clock::now();
    auto duration = (duration_cast<microseconds>(stop - start));
    std::cout << duration.count() << endl;*/
    return (0);
}