function err = calc_error(obj_filename,  gt_filename)

img = imread(obj_filename);
gt = double(imread(gt_filename));

img = rgb2gray(img);

diff = abs(double(img ~= 0) - gt./255);
err = sum(sum(diff))/prod(size(gt));
