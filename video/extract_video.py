import cv2
import argparse

cap = cv2.VideoCapture('test-0712-004.avi')
count = 0
wr_count = 391;
while cap.isOpened():

    ret,frame = cap.read()
    frame = cv2.resize(frame, (0,0), fx=0.5, fy=0.5) 
    cv2.imshow('window-name',frame)
    if count > 100 :
    	if (count % 20) == 0:
    		cv2.imwrite("im0%3d.jpg" % wr_count, frame)
		wr_count = wr_count + 1
    count = count + 1
    if cv2.waitKey(10) & 0xFF == ord('q'):
        break


cap.release()
cap.destroyAllWindows()
