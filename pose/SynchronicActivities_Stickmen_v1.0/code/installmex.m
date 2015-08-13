% script compiling platform-specyfic mex files
% run this before executing any evaluation routines
utilsdir = './utils';
eval(['mex -outdir ' utilsdir ' ' fullfile(utilsdir,'vgg_nearest_neighbour_dist.cxx')]);
eval(['mex -outdir ' utilsdir ' ' fullfile(utilsdir,'all_pairs_bb_iou.cxx')]);