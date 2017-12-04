// カメラキャプチャ→顔検出→天使の輪っか（カラフル）
#include <cv.h>
#include <ctype.h>
#include <highgui.h>
#include <GL/glut.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 顔検出の変数
int x, y;
double w = 1600, h = 1200;
CvCapture *capture = 0;
IplImage *img = 0, *img_gray = 0;
int i;
const char *cascade_name = 
  "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml";
const char *cascade_name_local = 
  "/usr/share/OpenCV-2.3.1/haarcascades/haarcascade_frontalface_default.xml";
CvHaarClassifierCascade *cascade = 0;
CvMemStorage *storage = 0;
CvSeq *faces;
CvPoint center[10] = {};
int radius[10] = {};

// ライトの情報
static const GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
static const GLfloat light_ambient[] = {0.4, 0.4, 0.4, 1.0};
static const GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};

// 全色共通
static const GLfloat diffuse[] = {0.0, 0.0, 0.0, 1.0};
static const GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
static const GLfloat shininess[] = {100.0};

// 着色の変数
#define COLOR 6
// 金
static const GLfloat gold_ambient[] = {1.0, 1.0, 0, 1.0};
// 水色
static const GLfloat skyblue_ambient[] = {0.0, 1.0, 1.0, 1.0};
// 白
static const GLfloat white_ambient[] = {1.0, 1.0, 1.0, 1.0};
// 黒
static const GLfloat black_ambient[] = {0.0, 0.0, 0.0, 1.0};
// ピンク
static const GLfloat pink_ambient[] = {1.0, 0.7, 0.9, 1.0};
// 紫
static const GLfloat purple_ambient[] = {1.0, 0.3, 1.0, 1,0};

// OpenCVを用いたカメラキャプチャ関数
int initcv(int argc, char **argv)
{
  // コマンド引数によって指定された番号のカメラに対するキャプチャ構造体を作成する
  if (argc == 1 || (argc == 2 && strlen (argv[1]) == 1 && isdigit (argv[1][0])))
    capture = cvCreateCameraCapture (argc == 2 ? argv[1][0] - '0' : 0);
  
  // キャプチャサイズを設定する．
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, w);
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, h);
  
  img = cvQueryFrame (capture);
  img_gray = cvCreateImage (cvGetSize (img), IPL_DEPTH_8U, 1);

  // ブーストされた分類器のカスケードを読み込む
  cascade = (CvHaarClassifierCascade *) cvLoad (cascade_name, 0, 0, 0);
  if ( cascade == 0 ) {
    cascade = (CvHaarClassifierCascade *) cvLoad (cascade_name_local, 0, 0, 0);
  }
  
  // メモリを確保し，読み込んだ画像のグレースケール化，ヒストグラムの均一化を行う
  storage = cvCreateMemStorage (0);
  cvClearMemStorage (storage);
  cvCvtColor (img, img_gray, CV_BGR2GRAY);
  cvEqualizeHist (img_gray, img_gray);
  
  // 顔検出
#if CV_MAJOR_VERSION <= 2 && CV_MINOR_VERSION <= 1 
  faces = cvHaarDetectObjects (img_gray, cascade, storage, 1.11, 4, 0, cvSize (40, 40));
#else
  faces = cvHaarDetectObjects (img_gray, cascade, storage, 1.11, 4, 0, cvSize (40, 40), cvSize (0, 0));
#endif
  
  // 検出された全ての顔位置をcenter配列に格納
  for (i = 0; i < (faces ? faces->total : 0); i++) {
    CvRect *r = (CvRect *) cvGetSeqElem (faces, i);
    center[i].x = cvRound (r->x + r->width * 0.5);
    center[i].y = cvRound (r->y + r->height * 0.5);
    radius[i] = cvRound((r->width + r->height) * 0.25);
  }

  // 上下左右反転（OpenCV→OpenGL）
  cvFlip(img, NULL, -1);
}

// OpenGL空間の初期化
void initgl(void)
{
  // クリアの値の設定
  glClearColor (0.2, 0.2, 0.2, 0.0);
  glClearDepth( 1.0 );
  
  // デプステストを行う
  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  
  glShadeModel (GL_SMOOTH);
  
  // デフォルトライト
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_ambient);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

// 色を付ける関数
void color(void)
{
  int COLOR_NUMBER = rand() % COLOR;
  
  switch(COLOR_NUMBER)
    {
    case 0:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, gold_ambient);
      break;
    case 1:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, skyblue_ambient);
      break;
    case 2:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white_ambient);
      break;
    case 3:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black_ambient);
      break;
    case 4:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pink_ambient);
      break;
    case 5:
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, purple_ambient);
      break;
    }
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

// 天使の輪っかの表示
void display(void)
{
  // Windowのクリア
  glClear(GL_COLOR_BUFFER_BIT);

  //画像をOpenGL空間に表示
  glDrawPixels(img->width, img->height, GL_BGR_EXT, GL_UNSIGNED_BYTE, img->imageData);

  glMatrixMode(GL_MODELVIEW);

  // 天使の輪っかの作成
  for(i = 0; i < (faces ? faces->total : 0); i++) {
    glLoadIdentity();
    glTranslatef(7.0-14.0*((double)center[i].x/img->width), 7.0+30.0*((double)radius[i]/img->height)-14.0*((double)center[i].y/img->height), -10.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    color();
    glPushMatrix();
    glutSolidTorus(1.8*((double)radius[i]/img->width), 18.0*((double)radius[i]/img->width), 100, 100);
    glPopMatrix();
  }

  // display関数が呼ばれた回数をcount
  glutSwapBuffers();
}

void reshape (int w, int h)
{
  glViewport (0, 0, (GLsizei) w, (GLsizei) h); 

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 20.0);
}

// 動画にする関数
void camera(void)
{
  img = cvQueryFrame (capture);
  img_gray = cvCreateImage (cvGetSize (img), IPL_DEPTH_8U, 1);

  cascade = (CvHaarClassifierCascade *) cvLoad (cascade_name, 0, 0, 0);
  if ( cascade == 0 ) {
    cascade = (CvHaarClassifierCascade *) cvLoad (cascade_name_local, 0, 0, 0);
  }
  
  storage = cvCreateMemStorage (0);
  cvClearMemStorage (storage);
  cvCvtColor (img, img_gray, CV_BGR2GRAY);
  cvEqualizeHist (img_gray, img_gray);
  
#if CV_MAJOR_VERSION <= 2 && CV_MINOR_VERSION <= 1 
  faces = cvHaarDetectObjects (img_gray, cascade, storage, 1.11, 4, 0, cvSize (40, 40));
#else
  faces = cvHaarDetectObjects (img_gray, cascade, storage, 1.11, 4, 0, cvSize (40, 40), cvSize (0, 0));
#endif
  
  for (i = 0; i < (faces ? faces->total : 0); i++) {
    CvRect *r = (CvRect *) cvGetSeqElem (faces, i);
    center[i].x = cvRound (r->x + r->width * 0.5);
    center[i].y = cvRound (r->y + r->height * 0.5);
    radius[i] = cvRound((r->width + r->height) * 0.25);
  }

  cvFlip(img, NULL, -1);

  glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
  switch (key) {
  case 27: // ESCで終了
    exit(0);
    break;
  }
}

int main(int argc, char **argv)
{
  // 色ランダム
  srand(time(NULL));
  
  glutInit(&argc, argv);
  initcv(argc, argv);
  
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(img->width, img->height);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("Movie");

  initgl();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(camera);
  glutKeyboardFunc(keyboard);
  glutMainLoop();
  
  cvReleaseImage(&img);

  return 0;
}
