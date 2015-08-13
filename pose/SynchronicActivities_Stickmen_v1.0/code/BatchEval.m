function [detRate PCP PCPdetail evalidx scores] = BatchEval(gtToBBfunc, eval_function,all_eval_stickmen, all_gt_stickmen, pcp_matching_threshold)
% user interface to evaluate stickmen sets for more that one image
% (multi-frame evaluation)
%
% Input:
% eval_function - is e function handler for an evaluation function on single image (i.e. EvalStickmen)
% all_eval_stickmen and all_gt_stickman - (contains stickmen to be evaluated for multiple frame 
%                   and the corresponding ground truth annotations respectively.
%                   They are struct arrays (one entry per image) with the following fields:
%                   .stickmen - struct array containing all stickmen for this frame, with fields:
%                       .coor - stickman end-points coordinates coor(:,nparts) = [x1 y1 x2 y2]'
%                       .det - is the detection window associated with the stickman in [minx miny maxx maxy]
% 
%                       if .det field doesn't exist, then a detection window will be derived automatically from head and torso sticks
%                   any number of fields with arbitrary names containing strings or digits uniquly determining the sample
%                   e.g.: .episode - corresponding episode number, .frame - corresponding image number
%                         or .filename - corresponding image name
% pcp_matching_threshold (optional) - defines the PCP sticks matching threshold (default 0.5) -> for definition look into README.txt 
%
% elements in all_eval_stickmen are evaluated against corresponding elements in all_gt_stickman
%
% Output:
% detRate - is the detection rate of the system (see README.txt)
% PCP - Percentage of Correctly estimated body Parts, evaluated only in correct detections (see README.txt)
% PCPdetailed - per body part PCP
% 
% see also EvalStickmen.m 
%
% Eichner/2009

if nargin < 5
  pcp_matching_threshold  = 0.5;
end

[all_gt_stickmen,all_eval_stickmen] = match_structs(all_gt_stickmen,all_eval_stickmen,'stickmen');

total = length(all_eval_stickmen);
scores = cell(total,1);
scoredetail = cell(total,1);
annotparts = cell(total,1);
evalidx = cell(total,1);

% evaluate each frame
for i=1:total
  [evalidx{i} scores{i} annotparts{i} scoredetail{i}] = eval_function(gtToBBfunc, all_eval_stickmen(i).stickmen, all_gt_stickmen(i).stickmen, pcp_matching_threshold);
end

matched = [evalidx{:}] > 0;
scoresall = [scores{:}];
annotparts = [annotparts{:}];
% compute detRate and PCP
detRate =  sum(matched)/numel(annotparts);
PCP = sum(scoresall(matched))/sum(annotparts(matched));
PCPdetail = [];
% if consistent number of sticks has been annotated over the whole dataset
% compute per body part PCP
if all(annotparts(1) == annotparts(:)) 
  scoredetail = [scoredetail{:}];
  scoredetail = [scoredetail{:}];
  PCPdetail = sum(scoredetail(:,matched),2)/sum(matched);
end



end

