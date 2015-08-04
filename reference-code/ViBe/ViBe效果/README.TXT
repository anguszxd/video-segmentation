Click on vibe (or vibe.bat) to launch the program!

Then select 
   File ->  Open video file for COLOR processing -> and choose sequence (or sequence.avi)

Please note that the program intentionally stops after about 5 minutes and 10.000 processed frames. 

If your want to save your results or modify the parameters, it is possible to run the program with the appropriated arguments. 

------------------------- vibe-gray.exe ---------------------------------------------------------------
Usage:
 C:\mypath\vibe-gray.exe --input [-i] <input-file.avi> [--output [-o] <output-file.avi>] [options] 

Options:
  --display-result-only [-ro] 		: display (and save) vibe segmentation exclusively 
  --nb-samples [-n] <value> 		: number of samples per pixel model (default: 20) 
  --matching-threshold [-t] <value> 	: threshold used to compare pixel samples to current pixel values (default: 20) 
  --matching-number [-s] <value> 	: number of close samples needed to classify a current pixel value as background (default: 2) 
  --subsampling-factor [-f] <value> 	: amount of random temporal subsampling (default: 16) 

Examples:
  If you wish to read from myvideo.avi, specify some parameters, display both images, and ouput a sequence of jpeg files:
    $> C:\mypath\vibe-gray.exe -i myvideo.avi -n 10 -s 3 --subsampling-factor 32 -o result-%010d.jpg

  If you wish to read from an axis netcam, just display (and save) the segmented image, and output as an .avi file:
    $> C:\mypath\vibe-gray.exe -i http://192.168.1.12/mjpg/video.mjpg --display-result-only -o result.avi

  If you wish to read from a sequence of jpeg images, output to an mpeg file, and display both images:
    $> C:\mypath\vibe-gray.exe -i image-%10d.jpg -o result.mpg


------------------------- vibe-rgb.exe ---------------------------------------------------------------
Usage:
 C:\mypath\vibe-rgb.exe --input [-i] <input-file.avi> [--output [-o] <output-file.avi>] [options] 

Options:
  --display-result-only [-ro] 		: display (and save) vibe segmentation exclusively 
  --nb-samples [-n] <value> 		: number of samples per pixel model (default: 20) 
  --matching-threshold [-t] <value> 	: threshold used to compare pixel samples to current pixel values (default: 20) 
  --matching-number [-s] <value> 	: number of close samples needed to classify a current pixel value as background (default: 2) 
  --subsampling-factor [-f] <value> 	: amount of random temporal subsampling (default: 16) 

Examples:
  If you wish to read from myvideo.avi, specify some parameters, display both images, and ouput a sequence of jpeg files:
    $> C:\mypath\vibe-rgb.exe -i myvideo.avi -n 10 -s 3 --subsampling-factor 32 -o result-%010d.jpg

  If you wish to read from an axis netcam, just display (and save) the segmented image, and output as an .avi file:
    $> C:\mypath\vibe-rgb.exe -i http://192.168.1.12/mjpg/video.mjpg --display-result-only -o result.avi

  If you wish to read from a sequence of jpeg images, output to an mpeg file, and display both images:
  $> C:\mypath\vibe-rgb.exe -i image-%10d.jpg -o result.mpg

------------------------------------------------------------------------------------------------------

