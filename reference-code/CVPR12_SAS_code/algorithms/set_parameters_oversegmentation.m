function [para_ms, para_gbis] = set_parameters_oversegmentation(img_loc)

% Set parameters for over-segmentation
% 1. Two complementary segmentation methods, Mean Shift and FH, are used
% 2. For each method, different parameters are used
% 3. Use image complexity to guide the selection of FH parameters

%%% Parameters for Mean Shift
para_ms.hs{1} = 7; para_ms.hr{1} = 7;  para_ms.M{1} = 100;
para_ms.hs{2} = 7; para_ms.hr{2} = 9;  para_ms.M{2} = 100;
para_ms.hs{3} = 7; para_ms.hr{3} = 11; para_ms.M{3} = 100;

para_ms.K = length(para_ms.hs);

%%% Parameters for FH: adaptive superpixels
% 1. Idea: include large superpixels for complex images to suppress
% strong edges within objects
% 2. Use LAB color variances to measure image complexity

img = colorspace('Lab<-', imread(img_loc));
[X,Y,Z] = size(img);
cvar = var( reshape(img,[X*Y,Z]) ); clear img;

if sum(cvar) < 500 || sum(cvar(2:end)) < 100
    % gbis para
    para_gbis.sigma{1} = 0.5; para_gbis.k{1} = 100; para_gbis.minsize{1} = 50;
    para_gbis.sigma{2} = 0.8; para_gbis.k{2} = 200; para_gbis.minsize{2} = 100;
else
    % gbis para
    para_gbis.sigma{1} = 0.8; para_gbis.k{1} = 150; para_gbis.minsize{1} = 50;
    para_gbis.sigma{2} = 0.8; para_gbis.k{2} = 200; para_gbis.minsize{2} = 100;
    para_gbis.sigma{3} = 0.8; para_gbis.k{3} = 300; para_gbis.minsize{3} = 100;
end

para_gbis.K = length(para_gbis.sigma);