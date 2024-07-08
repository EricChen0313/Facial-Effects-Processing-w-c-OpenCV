#include "opencv2/opencv.hpp" // �ޤJ OpenCV �禡�w
#include <iostream> // �ޤJ C++ ��J��X�y�禡�w

using namespace cv; // �ϥ� OpenCV �R�W�Ŷ�
using namespace std; // �ϥμзǩR�W�Ŷ�

/* �ۭq�禡 */
void detectAndDisplay(void); // �ŧi�����H�y���禡

/* �����ܼ� */
String face_cascade_name = "data/haarcascade_frontalface_alt.xml"; // �����H�y�������h�����������ɮצW��
String eyes_cascade_name = "data/haarcascade_eye_tree_eyeglasses.xml"; // �H���������h�����������ɮצW��

CascadeClassifier face_cascade; // �إߥ����H�y�������h������������
CascadeClassifier eyes_cascade; // �إߤH���������h������������

Mat im; // ��J�v��
int option = 5; // �S�Ŀﶵ
VideoCapture cap1("data/sleepy.mp4"); // �إߤH�y���T����
VideoCapture cap2("data/explosion.mp4"); // �إ߯S�ļv������
Point eye_centers[2]; // �������ߪ�(x,y)��m
Point click_position; // �ƹ��I����m
bool play_explosion = false; // ����explosion�лx
int explosion_frame_count = 0; // explosion�v�����V�p�ƾ�
bool face_detected = false; // �H�y�����лx

// �w�q�ƹ������禡
static void mouse_callback(int event, int x, int y, int, void*) {
    // ��ƹ����U����A�ھ��I���m�A�o��ﶵ (option) �ƭ� 
    if (event == EVENT_LBUTTONDOWN) {
        if (y > im.rows - 50) { // �Y�I����m�b�v���U�誺�ﶵ�ϰ�
            if (x < 100) option = 1; // ���
            else if (x < 200) option = 2; // �G�y 
            else if (x < 300) option = 3; // ���ɧJ
            else option = 4; // ��L�ﶵ
        }
        else if (face_detected) { // �u��������H�y�ɤ~�༽��explosion
            // �I����L�ϰ���ܲ����B�����ü���explosion�v��
            option = 4;
            click_position = Point(x, y);
            play_explosion = true;
            explosion_frame_count = 0;
            cap2.set(CAP_PROP_POS_FRAMES, 0); // �q�Y�}�l����explosion.mp4
        }
    }
}

/** �D�{�� */
int main(void) {
    if (!cap1.isOpened() || !cap2.isOpened()) { // �Y�L�k�}�ҤH�y���T��explosion�v���A��ܿ��~�T��
        printf("--(!)Error loading video/camera\n");
        waitKey(0);
        return -1;
    }

    // ���J�H�y�P�H�����������Ѽ�
    if (!face_cascade.load(face_cascade_name)) {
        printf("--(!)Error loading face cascade\n"); // �Y�L�k���J�����H�y�������h���������A��ܿ��~�T��
        waitKey(0);
        return -1;
    }
    if (!eyes_cascade.load(eyes_cascade_name)) {
        printf("--(!)Error loading eyes cascade\n"); // �Y�L�k���J�H���������h���������A��ܿ��~�T��
        waitKey(0);
        return -1;
    }

    namedWindow("window"); // �إ���ܼv��������
    setMouseCallback("window", mouse_callback); // �]�w�ƹ��^�ը禡

    while (char(waitKey(1)) != 27 && cap1.isOpened()) { // ����䤣�O Esc �B�H�y���T���󤴦b�}�ҮɡA�������j��
        cap1 >> im; // ������T���e��
        if (im.empty()) { // �p�G�S�����e��
            printf(" --(!) No captured im -- Break!"); // ��ܿ��~�T��
            break;
        }
        /* �����H�y�A����ܯS�ļv�� */
        detectAndDisplay();

        // �p�G�ݭn����explosion�v��
        if (play_explosion) {
            Mat explosion_frame;
            cap2 >> explosion_frame;
            if (!explosion_frame.empty()) {
                // �Y�pexplosion�v��
                resize(explosion_frame, explosion_frame, Size(), 0.5, 0.5);

                // �h���¦�I��
                Mat mask;
                inRange(explosion_frame, Scalar(0, 0, 0), Scalar(30, 30, 30), mask);
                bitwise_not(mask, mask);

                // �T�O�I����m���explosion�v�����W�X�d��
                int x = min(max(0, click_position.x - explosion_frame.cols / 2), im.cols - explosion_frame.cols);
                int y = min(max(0, click_position.y - explosion_frame.rows / 2), im.rows - explosion_frame.rows);
                Rect roi(x, y, explosion_frame.cols, explosion_frame.rows);

                Mat im_roi = im(roi);
                explosion_frame.copyTo(im_roi, mask);
            }
            explosion_frame_count++;
            if (explosion_frame_count >= cap2.get(CAP_PROP_FRAME_COUNT)) {
                play_explosion = false;
            }
        }

        /* ��ܼv�� */
        imshow("window", im);
    }
    return 0;
}

/** detectAndDisplay �禡���e */
void detectAndDisplay(void) {
    /** �H�y�����e�B�z */
    vector<Rect> faces; // �x�s�H�y ROI �ϰ쪺�V�q
    Mat im_gray; // �Ƕ��v������

    // �m��v����Ƕ�
    cvtColor(im, im_gray, COLOR_BGR2GRAY);
    // ��Ƕ��Ϲ��i�檽��ϧ��ŤơA�W�[����
    equalizeHist(im_gray, im_gray);

    // �˴������H�y
    face_cascade.detectMultiScale(im_gray, faces, 1.1, 4, 0, Size(80, 80));

    /** �p�G��������H�y�A����H�U�ԭz */
    face_detected = !faces.empty();
    if (face_detected) {
        // ��X�̤j���H�y ROI �ϰ�
        Rect faceROI = faces[0];
        for (size_t i = 1; i < faces.size(); ++i) {
            if (faces[i].area() > faceROI.area()) {
                faceROI = faces[i];
            }
        }

        // �y�L�X�j�H�y ROI�A�H�T�O�л\��ӤH�y
        faceROI.x = max(faceROI.x - 10, 0);
        faceROI.y = max(faceROI.y - 10, 0);
        faceROI.width = min(faceROI.width + 20, im.cols - faceROI.x);
        faceROI.height = min(faceROI.height + 20, im.rows - faceROI.y);

        // �����H����m
        Mat faceROI_gray = im_gray(faceROI);
        vector<Rect> eyes;
        eyes_cascade.detectMultiScale(faceROI_gray, eyes, 1.1, 2, 0, Size(30, 30));
        if (eyes.size() == 2) {
            eye_centers[0] = Point(faceROI.x + eyes[0].x + eyes[0].width / 2, faceROI.y + eyes[0].y + eyes[0].height / 2);
            eye_centers[1] = Point(faceROI.x + eyes[1].x + eyes[1].width / 2, faceROI.y + eyes[1].y + eyes[1].height / 2);
        }
        else {
            eye_centers[0] = Point(faceROI.x + faceROI.width / 3, faceROI.y + faceROI.height / 3);
            eye_centers[1] = Point(faceROI.x + 2 * faceROI.width / 3, faceROI.y + faceROI.height / 3);
        }

        // ��ܯS�Ŀﶵ
        putText(im, "Green", Point(10, im.rows - 20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 2);
        putText(im, "Green", Point(10, im.rows - 22), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
        putText(im, "Slim", Point(110, im.rows - 20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 2);
        putText(im, "Slim", Point(110, im.rows - 22), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
        putText(im, "Mosaic", Point(210, im.rows - 20), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 2);
        putText(im, "Mosaic", Point(210, im.rows - 22), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);

        // �ھڿﶵ�B�z ROI �v��
        // �ﶵ1: ���
        if (option == 1) {
            Mat im_face = im(faceROI);
            Mat im_hsv, mask;

            // �ഫ��HSV��m�Ŷ�
            cvtColor(im_face, im_hsv, COLOR_BGR2HSV);

            // �w�q�ֽ��⪺�d��
            Scalar lower_skin_color = Scalar(0, 0, 0);
            Scalar upper_skin_color = Scalar(40, 255, 255);

            // ���ֽ��⪺�d��A�óЫر��X
            inRange(im_hsv, lower_skin_color, upper_skin_color, mask);

            // �u�w��HSV��m�Ŷ����Ĥ@�ӳq�D�i��վ�
            for (int i = 0; i < im_hsv.rows; i++) {
                for (int j = 0; j < im_hsv.cols; j++) {
                    if (mask.at<uchar>(i, j) > 0) {
                        im_hsv.at<Vec3b>(i, j)[0] = 60; // �N�ֽ���אּ���
                    }
                }
            }

            // �̲���^��RGB��m�Ŷ�
            cvtColor(im_hsv, im_face, COLOR_HSV2BGR);

            // �N�B�z�᪺�y���ϰ�v���ƻs�^��v��
            im_face.copyTo(im(faceROI), mask);
        }

        // �ﶵ2: �G�y
        else if (option == 2) {
            Mat im_face = im(faceROI);
            Mat map_x, map_y;
            map_x.create(im_face.size(), CV_32FC1);
            map_y.create(im_face.size(), CV_32FC1);

            float strength = 50.0; // �����q���v�A�i�H�ھڻݭn�վ�
            float center_x = faceROI.x + faceROI.width / 2.0;
            float center_y = faceROI.y + faceROI.height / 2.0;

            for (int i = 0; i < im_face.rows; ++i) {
                for (int j = 0; j < im_face.cols; ++j) {
                    float offset_x = (center_x - abs(j - center_x)) * strength / center_x;
                    map_x.at<float>(i, j) = j + offset_x;
                    map_y.at<float>(i, j) = i;
                }
            }

            remap(im_face, im_face, map_x, map_y, INTER_LINEAR, BORDER_REPLICATE);
        }

        // �ﶵ3: ���ɧJ
        else if (option == 3) {
            Mat im_face = im(faceROI);
            Mat small_face;
            resize(im_face, small_face, Size(), 0.1, 0.1, INTER_NEAREST);
            resize(small_face, im_face, im_face.size(), 0, 0, INTER_NEAREST);
        }

        // �ﶵ4: �e������ƹ��I����m�����u
        else if (option == 4) {
            line(im, eye_centers[0], click_position, Scalar(0, 0, 255), 2); // �������I����m
            line(im, eye_centers[1], click_position, Scalar(0, 0, 255), 2); // �k�����I����m
        }

        // ø�s�H�y�ϰ�x�ήءA�H�ΤW�誺�Ǹ�
        rectangle(im, faceROI, Scalar(0, 255, 0), 2);
        putText(im, "M11202125", Point(faceROI.x, faceROI.y - 10), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
    }

    /* ��ܼv�� */
    imshow("window", im);
}
