#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp> // imread
#include <opencv2/highgui.hpp> // imshow, waitKey
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

struct single_img {
    Mat lightsource;
    Mat img;
    Mat normal;
}; 

int main() {
    single_img images[6];
    for(int i = 1; i <= 6; i++) {
        //int to string
        stringstream ss;
        ss << i;
        string filename = "pic/bunny/pic" + ss.str() + ".bmp";
        // Load image as a single channel grayscale Mat.
        images[i - 1].img = imread(filename, IMREAD_GRAYSCALE);
        //init normal matrix  
        Mat A(Mat::zeros(images[i - 1].img.rows, images[i - 1].img.cols, CV_32FC(3))); 
        images[i - 1].normal = A.clone();
    }
    
    //read light source file
    fstream file;
    file.open("pic/bunny/LightSource.txt", ios::in);
    if(file) {
        string tmp, value;
        int j = 0;
        while(file >> tmp >> value) {
            int vertex_tmp[3];
            //parse lightsource.txt
            value = value.substr(1, value.length() - 2);
            size_t found = value.find(",");
            for(int i = 0; i < 3; found = value.find(","), i++) {
                string value_tmp;
                if(found != string::npos) {
                    value_tmp = value.substr(0, found);
                    value = value.substr(found + 1);
                }
                else {
                    value_tmp = value;
                }
                //string to int
                istringstream ss(value_tmp);
                ss >> vertex_tmp[i];
            }
            //store to iamges[j].lightsource
            images[j].lightsource = (Mat_<float>(1, 3) << vertex_tmp[0], vertex_tmp[1], vertex_tmp[2]);
            j++;
        }
    }
    else {
        cerr << "Can't open light source file." << endl;
        exit(1);
    }
    
    // comine the lightsource matrix to one
    Mat source(Mat::zeros(6 ,3, CV_32FC(1)));
    for(int i =0; i < 6; i++) {
        images[i].lightsource.row(0).copyTo(source.row(i));
    }
    //pseudo invert
    Mat source_t = source.t();
    Mat pseudo = (source_t * source).inv(DECOMP_SVD) * source_t;//(U^TU)U^T 3*6
    //cout<<pseudo;

    //construct intensity matrix
    int size = images[0].img.cols * images[0].img.rows;
    Mat intensity(Mat::zeros(6, size, CV_32FC(1)));//6*size
    for(int k = 0; k < 6; k++) {
        for(int i = 0; i < images[k].img.rows; i++) {
            for(int j = 0; j < images[k].img.cols; j++) {
                intensity.at<float>(k, i * images[k].img.cols + j) = images[0].img.at<uchar>(i, j);
                //Mat b = pseudo * images[0].img.at<uchar>(i, j); 
                //cout << b;
                //cout<< b.at<float>(0, 0) << " " << b.at<float>(1, 0) << " "<< b.at<float>(2, 0) << norm(b, NORM_L2);
                /*
                if(norm(b, NORM_L2) != 0) {
                    for(int k = 0; k < 3; k++) {
                        //images[0].normal.at<Vec3f>(Point(i, j)) = Vec3f(0, 0, 0);
                        images[0].normal.at<Vec3f>(i, j).val[k] = b.at<float>(k, 0) / norm(b, NORM_L2);
                    }
                }
                else {
                    images[0].normal.at<Vec3f>(i, j) = Vec3f(0, 0, 0); 
                }*/
                //cout << images[0].normal.at<Vec3f>(i, j) << endl;
            }
        }
    }
    
    //normal = b/|b|
    Mat b = pseudo * intensity;//3*size
    Mat normal(Mat::zeros(3, size, CV_32FC(1)));
    for(int i = 0; i < size; i++) {
        //cout << b.col(i);
        //cout << norm(b.col(i), NORM_L2);
        //normal.at<Vec3f>(i, j).val[k] = b.at<float>(k, 0) / norm(b, NORM_L2)
        if(norm(b.col(i), NORM_L2) != 0) {
            Mat tmp = b.col(i) / norm(b.col(i), NORM_L2);
            tmp.copyTo(normal.col(i));
        }
        else {
            Mat tmp(Mat::zeros(3, 1, CV_32FC(1)));
            tmp.copyTo(normal.col(i));
        }
        //tmp.copyTo(normal.col(i));
    }
    
    // Mat is a thin template wrapper on top of the Mat class.
    // Mat_::operator()(y, x) does the same thing as Mat::at(y, x).
    /*
    Mat_<uchar> imgWrp(img);
    Mat_<uchar> smallImgWrp(img.size() / 2);
    for (int rowIndex = 0; rowIndex != smallImgWrp.rows; ++rowIndex)
        for (int colIndex = 0; colIndex != smallImgWrp.cols; ++colIndex)
            smallImgWrp(rowIndex, colIndex) =
                imgWrp(rowIndex * 2, colIndex * 2);
    Mat result(img.rows + smallImgWrp.rows, img.cols, CV_8U, Scalar(0));
    imgWrp.copyTo(result(Rect(0, 0, imgWrp.cols, imgWrp.rows)));
    smallImgWrp.copyTo(
        result(Rect(0, imgWrp.rows, smallImgWrp.cols, smallImgWrp.rows)));
    cv::imshow("Hi", result);
    cv::waitKey(); // Wait for the user to press a key.
    */
    return 0;
}
