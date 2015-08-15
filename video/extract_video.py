import cv2
import argparse

cap = cv2.VideoCapture('test-0712-004.avi')
count = 0
wr_count = 0;
while cap.isOpened():
    ret,frame = cap.read()
    cv2.imshow('window-name',frame)
    if (count % 8) == 0:
    	cv2.imwrite("frame%d.jpg" % wr_count, frame)
	wr_count = wr_count + 1
    count = count + 1
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break


cap.release()
cap.destroyAllWindows()
