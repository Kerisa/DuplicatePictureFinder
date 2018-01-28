
//#include "svd.h"
//#include <Windows.h>
//#include "opencv2/opencv.hpp"
//
//
//static void PrintMatrix(CvMat *Matrix, int Rows, int Cols)
//{
//    for (int i = 0; i<Rows; i++)
//    {
//        for (int j = 0; j<Cols; j++)
//        {
//            printf("%.3f\t", cvGet2D(Matrix, i, j).val[0]);
//        }
//        printf("\n");
//    }
//}
//
void SVD(int *SA, int rows, int cols)
{
//    float *fA = new float[rows * cols];
//    for (int i = 0; i < rows*cols; ++i)
//        fA[i] = SA[i];
//    
//    // A = U*W*V
//    CvMat *A = cvCreateMat(rows, cols, CV_32FC1);
//    CvMat *W = cvCreateMat(rows, cols, CV_32FC1);
//    CvMat *U = cvCreateMat(rows, rows, CV_32FC1);
//    CvMat *V = cvCreateMat(cols, cols, CV_32FC1);
//    cvSetData(A, fA, A->step);
//    cvSVD(A, W, U, V);
//    
//    delete[] fA;
}
