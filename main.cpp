#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp> // imread
#include <opencv2/highgui.hpp> // imshow, waitKey
#include <opencv2/opencv.hpp>
#include <math.h>
#include <stdlib.h> //atoi
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace cv;
using namespace std;

struct single_img {
    Mat lightsource;
    Mat img;
}; 

int main(int argc, char* argv[]) {
    single_img images[6];
    string pic_name = "bunny";
    for(int i = 1; i <= 6; i++) {
        if(argc > 1) {
            if(atoi(argv[1]) == 2) {
                pic_name = "venus";
            } 
        }   
        //int to string
        stringstream ss;
        ss << i;
        string filename = "pic/" + pic_name + "/pic" + ss.str() + ".bmp";
        // Load image as a single channel grayscale Mat.
        images[i - 1].img = imread(filename, IMREAD_GRAYSCALE);
    }
    
    //read light source file
    fstream file;
    string lightsource_position = "pic/" + pic_name + "/LightSource.txt";
    file.open(lightsource_position.c_str(), ios::in);
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
            images[j].lightsource = (Mat_<double>(1, 3) << vertex_tmp[0], vertex_tmp[1], vertex_tmp[2]);
            j++;
        }
    }
    else {
        cerr << "Can't open light source file." << endl;
        exit(1);
    }
    
    // combine the lightsource matrix to one
    Mat source(Mat::zeros(6 ,3, CV_64FC(1)));
    for(int i =0; i < 6; i++) {
        images[i].lightsource.row(0).copyTo(source.row(i));
    }
    //pseudo invert
    Mat source_t = source.t();
    Mat pseudo = ((source_t * source).inv(CV_SVD)) * source_t;//(U^TU)U^T 3*6

    //retrieve intensity
    int size = images[0].img.cols * images[0].img.rows;
    Mat intensity(Mat::zeros(6, size, CV_64FC(1)));//6*size
    for(int k = 0; k < 6; k++) {
        for(int i = 0; i < images[0].img.rows; i++) {
            for(int j = 0; j < images[0].img.cols; j++) {
                intensity.at<double>(k, i * images[0].img.cols + j) = images[k].img.at<uchar>(i, j);
            }
        }
    }
 
    //normal = b/|b|
    Mat b = pseudo * intensity;//3*size
    /*check b 
    for(int i = 0; i < images[0].img.rows; i++) {
        for(int j = 0; j < images[0].img.cols; j++) {
            cout << b.col(i * images[0].img.cols + j);
        }
    }*/
    Mat normal(Mat::zeros(3, size, CV_64FC(1)));
    for(int i = 0; i < size; i++) {
        double norm_tmp = sqrt(pow(b.col(i).at<double>(0, 0), 2) + pow(b.col(i).at<double>(1, 0), 2) + pow(b.col(i).at<double>(2, 0), 2));
        if(norm_tmp != 0) {
            Mat tmp = b.col(i) / norm_tmp;
            tmp.copyTo(normal.col(i));
        }
        else {
            Mat tmp(Mat::zeros(3, 1, CV_64FC(1)));
            tmp.copyTo(normal.col(i));
        }
    }
  

    //depth 
    Mat depth(Mat::zeros(images[0].img.rows, images[0].img.cols, CV_64FC(1)));
    depth.at<double>(0, 0) = 0;
    for(int i = 1; i < images[0].img.rows; i++) {
        Mat current = normal.col(i * images[0].img.cols);
        if(current.at<double>(2, 0) != 0) {
            depth.at<double>(i, 0) = (-1) * current.at<double>(1, 0) / current.at<double>(2, 0) + depth.at<double>(i - 1, 0); 
        }
        else {
            depth.at<double>(i, 0) = 0;
        }
    }

    
    for(int i = 0; i < images[0].img.rows; i++) {
        for(int j = 1; j < images[0].img.cols; j++) {
            Mat current = normal.col(i * images[0].img.cols + j);
            if(current.at<double>(2, 0) != 0) {
                depth.at<double>(i, j) = (-1) * current.at<double>(0, 0) / current.at<double>(2, 0) + depth.at<double>(i, j - 1);
            }
            else {
                depth.at<double>(i, j) = 0;
            }
        }
    }


    fstream ply;
    ply.open("hw.ply", ios::out);
    ply << "ply\nformat ascii 1.0\ncomment alpha=1.0\nelement vertex " << images[0].img.rows * images[0].img.cols << "\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue z\nend_header\n";
    for(int i = 0; i < images[0].img.rows; i++) {
        for(int j = 0; j < images[0].img.cols; j++) {
            ply << i << " " << j << " " << depth.at<double>(i,j) << " 255 255 255" << endl;
        }
    }

    return 0;
}
