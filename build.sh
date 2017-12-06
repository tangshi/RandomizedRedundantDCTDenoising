# g++ -msse4 \# sse4 support 
# -L/usr/lib/x86_64-linux-gnu/\
# -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \# required opencv libs
# -lopencv_photo -lopencv_xphoto \# required if OPENCV_DCTDENOISE_AND_NLM defined
# *.cpp -o rrdct.exe 

g++ -msse4 -L/usr/lib/x86_64-linux-gnu/ -lopencv_photo -lopencv_xphoto -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc *.cpp -o rrdct.exe
