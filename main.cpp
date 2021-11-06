#include <iostream>
#include <memory>
#include <QApplication>
#include <QLabel>

using Mat = std::vector<std::vector<double>>;

auto imageToGrayScale(const QImage &inputImage) -> QImage {
    auto outImage = inputImage.copy().convertToFormat(QImage::Format_Grayscale8);
    return outImage;
}

auto calcDerivatives(const QImage &inputImage) -> std::tuple<Mat, Mat, Mat> {
    int divX[3][3] = {{-1, 0, 1},
                      {-2, 0, 2},
                      {-1, 0, 1}};
    int divY[3][3] = {{-1, -2, -1},
                      {0,  0,  0,},
                      {1,  2,  1}};
    Mat dx_mat(inputImage.width());
    Mat dy_mat(inputImage.width());
    Mat dx_dy_mat(inputImage.width());
    for (auto i = 0; i < inputImage.width(); ++i) {
        dx_mat[i].resize(inputImage.height(), 0.0);
        dy_mat[i].resize(inputImage.height(), 0.0);
        dx_dy_mat[i].resize(inputImage.height(), 0.0);
    }
    for (auto i = 0; i < inputImage.width(); ++i) {
        for (auto j = 0; j < inputImage.height(); ++j) {
            dx_mat[i][j] = inputImage.pixelColor(i, j).value();
            dy_mat[i][j] = inputImage.pixelColor(i, j).value();
            dx_dy_mat[i][j] = dx_mat[i][j] * dy_mat[i][j];
        }
    }
    int xLen = inputImage.width() - 2;
    int yLen = inputImage.height() - 2;
    int xOut = 1;
    for (int i = 0; i < xLen; ++i) {
        int yOut = 1;
        for (int j = 0; j < yLen; ++j) {
            double sumX = 0;
            double sumY = 0;
            for (int kernelI = 0; kernelI < 3; ++kernelI) {
                for (int kernelJ = 0; kernelJ < 3; ++kernelJ) {
                    auto value = inputImage.pixelColor(i + kernelI, j + kernelJ).value();
                    sumX += (value * divX[kernelI][kernelJ]);
                    sumY += (value * divY[kernelI][kernelJ]);
                }
            }
            double dx = sumX;
            double dy = sumY;
            double dxdy = sumX * sumY;
            dx_mat[xOut][yOut] = dx;
            dy_mat[xOut][yOut] = dy;
            dx_dy_mat[xOut][yOut] = dxdy;
            yOut++;
        }
        xOut++;
    }
    return {dx_mat, dy_mat, dx_dy_mat};
}

auto
calc_harris_response_matrix(const Mat &dx_mat, const Mat &dy_mat, const Mat &dx_dy_mat, int w_size, int h_size,
                            int window_size, double r_value) -> Mat {
    Mat responseMatrix(w_size, std::vector<double>(h_size, 0));
    int offset = window_size / 2;
    for (auto x = offset; x < w_size - offset; ++x) {
        for (auto y = offset; y < h_size - offset; ++y) {
            double Sxx = 0.0;
            double Syy = 0.0;
            double Sxy = 0.0;
            for (auto window_x = x - offset; window_x < x + offset + 1; ++window_x) {
                for (auto window_y = y - offset; window_y < y + offset + 1; ++window_y) {
                    Sxx += std::pow(dx_mat[window_x][window_y], 2);
                    Syy += std::pow(dy_mat[window_x][window_y], 2);
                    Sxy += dx_dy_mat[window_x][window_y];
                }
            }
            double det = Sxx * Syy - Sxy * Sxy;
            double trace = Sxx + Syy;
            responseMatrix[x][y] = det - r_value * std::pow(trace, 2);
        }
    }
    return responseMatrix;

}

//auto nonMaxSuppression(const QImage &inputImage,Mat &responseMatrix, int w_size, int h_size, int window_size, int threshold) -> void {
//    auto outImage = inputImage.copy();
//    for (auto i = 0; i < outImage.width(); ++i) {
//        for (auto j = 0; j < outImage.height(); ++j) {
//            if (responseMatrix[i][j] > threshold) {
//                auto  neighbor_pairs = std::vector<
//                outImage.setPixel(i, j, 0);
//            }
//        }
//    }
//
//
//
//}


auto markPoints(const QImage &inputImage, const Mat &responseMatrix, double threshold) -> QImage {
    auto outImage = inputImage.copy().convertToFormat(QImage::Format_RGB32);
    for (auto i = 0; i < outImage.width(); ++i) {
        for (auto j = 0; j < outImage.height(); ++j) {
            ///std::cout << responseMatrix[i][j] << std::endl;
            if (responseMatrix[i][j] > threshold) {
                outImage.setPixel(i, j, QColor(255, 0, 0).rgb());
            }
        }
    }
    return outImage;

}


auto main(int argc, char *argv[]) -> int {
    QApplication a(argc, argv);
    auto label = std::make_unique<QLabel>();
    auto img = QImage("../pictures/robopenguin.bmp");
    img = img.convertToFormat(QImage::Format_RGB32);
    img = imageToGrayScale(img);
    auto[dx_mat, dy_mat, dx_dy_mat] = calcDerivatives(img);
    auto r_matrix = calc_harris_response_matrix(dx_mat, dy_mat, dx_dy_mat, img.width(), img.height(), 5, 0.04);
    img = markPoints(img, r_matrix, 28000);
    label->setPixmap(QPixmap::fromImage(img));
    label->show();
    return QApplication::exec();
}
