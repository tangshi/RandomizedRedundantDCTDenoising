//////////////////////////////////////////////////////////////////////////////////////////////
//Author Norishige Fukushima
//LICENSE: 3-clause BSD license
//For demonstration code for S. Fujita, N. Fukushima, M. Kimura, and Y. Ishibashi, 
//"Randomized Redundant DCT: Efficient Denoising by Using Random Subsampling of DCT Patches," 
//in Proc. ACM SIGGRAPH Asia Technical Briefs, Nov. 2015.
//http://fukushima.web.nitech.ac.jp/research/rrdct/
//////////////////////////////////////////////////////////////////////////////////////////////
#include "RedundantDXTDenoise.h"

using namespace std;
using namespace cv;

#define OPENCV_DCTDENOISE_AND_NLM

#ifdef OPENCV_DCTDENOISE_AND_NLM
#include <opencv2/xphoto.hpp>
#include <opencv2/photo.hpp>
/* required libs:
    opencv_photo
    opencv_xphoto 
*/
#endif

void guiDenoise(Mat& src, Mat& dest, string wname = "denoise")
{
	namedWindow(wname);

	int sw = 2; createTrackbar("sw", wname, &sw, 5);
	int blksize = 3; createTrackbar("bsize2^n", wname, &blksize, 6);
	int snoise = 20; createTrackbar("noise", wname, &snoise, 100);
	int thresh = 200; createTrackbar("thresh", wname, &thresh, 2000);
	int r = 2; createTrackbar("r", wname, &r, 20);

	int radius = 2; createTrackbar("rad:NLM", wname, &radius, 20);
	int h = 250; createTrackbar("h:NLM", wname, &h, 2500);
	int h_c = 180; createTrackbar("hc:NLM", wname, &h_c, 2500);

	Mat noise;
	addNoise(src, noise, snoise);
	int key = 0;
	RedundantDXTDenoise dctDenoise;
	RRDXTDenoise rrdct;
	rrdct.generateSamplingMaps(src.size(), Size(16, 16), 1, r, RRDXTDenoise::SAMPLING::LATTICE);

	bool isNoiseUpdate = false;
	while (key != 'q')
	{
		int bsize = (int)pow(2.0, blksize);
		Size block = Size(bsize, bsize);
		if (isNoiseUpdate) addNoise(src, noise, snoise);
		{
			CalcTime t;
			
			if (sw == 0)
			{
				noise.copyTo(dest);
				
			}
			else if (sw == 1)
			{
				dctDenoise(noise, dest, thresh / 10.f, block);
			}
			else if (sw == 2)
			{
				rrdct.generateSamplingMaps(src.size(), block, 1, r, RRDXTDenoise::SAMPLING::LATTICE);
				rrdct(noise, dest, thresh / 10.f, block);
			}
			else if (sw == 3)
			{
				rrdct.generateSamplingMaps(src.size(), block, 1, r, RRDXTDenoise::SAMPLING::LATTICE);
				rrdct(noise, dest, thresh / 10.f, block, RRDXTDenoise::BASIS::DHT);
			}
			else if (sw == 4)
			{

#ifdef OPENCV_DCTDENOISE_AND_NLM
				fastNlMeansDenoisingColored(noise, dest, h / 10.f, h_c / 10.f, 3, 2 * radius + 1);
#else
				cout << "cv::fastNlMeansDenoisingColored is not compiled. Please define OPENCV_DCTDENOISE_AND_NLM " << endl;
#endif
			}
			else if (sw == 5)
			{
#ifdef OPENCV_DCTDENOISE_AND_NLM
				cv::xphoto::dctDenoising(noise, dest, thresh, bsize);
#else
				cout << "cv::xphoto::dctDenoising is not compiled. Please define OPENCV_DCTDENOISE_AND_NLM " << endl;
#endif
			}

			if (key == 'n') isNoiseUpdate = (isNoiseUpdate) ? false: true;

			if (key == 'h' || key == '?')
			{
				cout << " 'n' swichs flag for updating noise image or not " << endl;
				cout << "sw==0: noisy image" << endl;
				cout << "sw==1: parallel dct denoising " << endl;
				cout << "sw==2: rr-dct denoising " << endl;
				cout << "sw==3: rr-dht denoising " << endl;
				cout << "sw==4: non-local means denoising " << endl;
				cout << "sw==5: OpenCV implementation of dct denosing " << endl;
				key = waitKey(1000);
			}
		}
		cout << YPSNR(src, dest) << " dB" << endl;
		imshow(wname, dest);
		key = waitKey(1);
	}
}

int main(int argc, const char *argv[])
{
	{
		//for debug
		//Mat src = imread("img/kodim13.png"); Mat dest; guiDenoise(src, dest); return 0;
	}

	const string keys =
	{
		"{help h|   | print this message}"
		"{@src(image)  |   |src image}"
		"{@dest(image) |   |dest image}"
		"{@noise(float)|   |noise stddev}"
		"{b     |DCT|basis DCT or DHT}"
		"{bs    |8  |block width      }"
		"{g gui |   |call interactive denoising}"
		"{d     |0  |minimum d for sampling. if(d==0) auto}"
		"{s     |lattice  |sampling type (lattice, poisson, full) or (l, p, f)}"
	};
	cv::CommandLineParser parser(argc, argv, keys);
	if (argc == 1)
	{
		parser.printMessage();
		return 0;
	}

	Mat src = imread(parser.get<string>(0));
	Mat dest;
	if (src.empty())
	{
		cout << "file path:" << parser.get<string>(0) << "is not correct." << endl;
		return 0;
	}

	if (parser.has("gui"))
	{
		//call gui demo
		guiDenoise(src, dest);
		imwrite(parser.get<string>(1), dest);
		return 0;
	}
	else
	{
		string basis = parser.get<string>("b");
		string sampling = parser.get<string>("s");
		int bs = parser.get<int>("bs");
		Size psize = Size(bs, bs);
		int d = parser.get<int>("d");
		if (d == 0) d = bs / 3;
		float sigma = parser.get<float>(2);

		RedundantDXTDenoise::BASIS dxtbasis;
		RRDXTDenoise::SAMPLING samplingType;
		if (basis == "DCT") dxtbasis = RedundantDXTDenoise::BASIS::DCT;
		else if (basis == "DHT") dxtbasis = RedundantDXTDenoise::BASIS::DHT;

		if (sampling == "lattice" || sampling == "l") samplingType = RRDXTDenoise::SAMPLING::LATTICE;
		else if (sampling == "poisson" || sampling == "p") samplingType = RRDXTDenoise::SAMPLING::POISSONDISK;
		else samplingType = RRDXTDenoise::SAMPLING::FULL;

		RRDXTDenoise rrdxt;
		rrdxt.generateSamplingMaps(src.size(), psize, 1, d, samplingType);
		rrdxt(src, dest, sigma, psize, dxtbasis);
	}

	imwrite(parser.get<string>(1), dest);
	return 0;
}