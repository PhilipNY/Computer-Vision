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
     
    Mat source = images[0].lightsource;
    Mat source_t = images[0].lightsource.t();
    //cout << source << source_t;
    Mat pseudo = (source_t * source).inv(DECOMP_SVD) * source_t;//(U^TU)U^T 3*1

    for(int i = 0; i < images[0].img.rows; i++) {
        for(int j = 0; j < images[0].img.cols; j++) {
            Mat b = pseudo * images[0].img.at<uchar>(i, j); 
            cout << b;
            cout<< b.at<float>(0, 0) << " " << b.at<float>(1, 0) << " "<< b.at<float>(2, 0) << norm(b, NORM_L2);
            if(norm(b, NORM_L2) != 0) {
                for(int k = 0; k < 3; k++) {
                    //images[0].normal.at<Vec3f>(Point(i, j)) = Vec3f(0, 0, 0);
                    images[0].normal.at<Vec3f>(i, j).val[k] = b.at<float>(k, 0) / norm(b, NORM_L2);
                }
            }
            else {
                images[0].normal.at<Vec3f>(i, j) = Vec3f(0, 0, 0); 
            }
            cout << images[0].normal.at<Vec3f>(i, j) << endl;
            //cout << norm(b, NORM_L2) << " " << images[0].normal << endl;
            //cout << int(images[0].img.at<uchar>(i, j));
            //Scalar intensity = img[0].at<uchar>(i, j);
            //cout << intensity << " ";  
        }
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
