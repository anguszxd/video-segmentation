function [evalNo, Score, Nannotparts, ScoreDetailed] = EvalStickmenMulti(gtToBBfunc, stickmen_set, gt_stickmen, pcp_matching_threshold)
% user interface to evaluate set of stickmen together with corresponding detection bounding boxes against a set of ground-truth stickmen
% (supports multiple annotations per file)
% (single-image evaluation)
%
% Input:
% gtToBBfunc - function that generates ground-truth detections from gt_stickman annotations e.g. detBBFromStickmanPascal
% stickmen_set - (contains set of stickmen to be evaluated with the ground-truth stickman) is an struct array with fields: 
%       .coor - stickman end-points coordinates (:,nparts) = [x1 y1 x2 y2]'
%       .det - the detection bounding box associated with the stickman [minx miny maxx maxy]
%
%       if .det field doesn't exist, then for association purpose a detection bounding boxes will be generated for each element in stickmen_set 
%       in the same way as for groundtruth_stickman

% gt_stickmen - (contains ground-truth stickman coordiantes) is a struct array with field:
%        .coor - containing ground-truth stickman end-points coordinates (:,nparts) = [x1 y1 x2 y2]' 
%
% pcp_matching_threshold (optional) - defines the PCP sticks matching threshold (default 0.5) -> for definition look into README.txt 
%
% Output:
% evalNo - index of the evaluated stickman that has been matched with the ground-truth stickman
% Score -  indicates the number of correctly estimated body parts in stickmen_set(evalNo)
%
% in case when none of the stickmen_set can be associated with the gt_stickmen or stickmen_set is empty then evalNo = 0, Score = nan;
%
% the evaluation procedure for each annotation in gt_stickmen stops as soon one of the stickmen_set is associated to the ground-truth annotation 
% (if two det windows from stickmen_set overlap by more than PASCAL criterion then evaluation stops reporting an error)
%
% Eichner/2009

if nargin < 4
  pcp_matching_threshold  = 0.5;
end


pars.iou_thresh = 0.5;
classname = 'ubf';
pars.det_hwratio(2) = 0.9;
pars.face_regparams = [1.3,0.1,3.6,0.9]; % regparams for Roger's face detector -> ccf

Neval = length(stickmen_set);
Ngt = length(gt_stickmen);

ScoreDetailed = cell(1,Ngt);
Score = zeros(1,Ngt);
evalNo = zeros(1,Ngt);
Nannotparts = zeros(1,Ngt);

% set Nparts and initialize detailed scores with zeros - in case of early exit due to empty stickmen_set
for gtix = 1:length(gt_stickmen) % hallucinate detection window from the gt stickman
  Nannotparts(gtix) = size(gt_stickmen(gtix).coor,2); %count both occluded and not occluded parts
  ScoreDetailed{gtix} = zeros(Nannotparts(gtix),1);
end

if isempty(stickmen_set) || isempty(stickmen_set(1).coor)
  return;
end


% create the .det field if does not exist
if ~isfield(stickmen_set(1),'det')
 stickmen_set(1).det = []; % setting for one elements adds this field for whole array
end
for i=1:Neval
  if isempty(stickmen_set(i).det)
    % if detection was not provided then derive from stickman
    stickmen_set(i).det = gtToBBfunc(stickmen_set(i).coor,classname,pars);
  end
end

% no two of the provided detections may be overlaping by more than the PASCAL criterion
det_overlap = all_pairs_bb_iou(cat(1,stickmen_set(:).det)', cat(1,stickmen_set(:).det)');
det_overlap = ~eye(size(det_overlap)).*det_overlap; % remove elements from the diagonal
if any(det_overlap(:) > pars.iou_thresh)
  error('no two detections may overlap with each other by more than the PASCAL criterion !!!!');
end

for gtix = 1:length(gt_stickmen) % hallucinate detection window from the gt stickman
  gt_bb = gtToBBfunc(gt_stickmen(gtix).coor,classname,pars);
  for i=1:Neval
    if all_pairs_bb_iou(gt_bb',  stickmen_set(i).det') > pars.iou_thresh 
      % evaluate only when eval_bb can be associated with gt_bb iou > 0.5
      % stop evaluation after first bb match
      evalNo(gtix) = i; % so evalNo ~= 0 and stickman is taken into account when calculating PCP
      Score(gtix) = 0;
      if isempty(stickmen_set(i).coor)
        disp('WARNING: the evaluated stickmen coor field is empty -> score 0');
      else
        [Score(gtix) ScoreDetailed{gtix}] = DirectEvalStickman(stickmen_set(i).coor, gt_stickmen(gtix).coor,pcp_matching_threshold);
      end
      break;
    end
  end
end


end
