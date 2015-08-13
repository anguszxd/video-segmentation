function [y x detrate] = calcPCPcurve(gtToBBfunc, eval_function, all_eval_stickmen,all_gt_stickmen,pcp_thresholds,show,curvecolor,linespec)
% plotting PCPcurve, 
% input: 
% gtToBBfunc - function that generates ground-truth detections from gt_stickman annotations e.g. detBBFromStickmanPascal
% eval_function is a function handler to the single image evaluation routine (e.g. EvalStickmen)
% all_eval_stickmen and all_gt_stickmen as defined in BatchEval
% if show is set to true then plotting the curve
% output: [x y] point coordinates on the PCP curve
if nargin < 6 
  show = false;
end

if nargin < 5 || isempty(pcp_thresholds)
  MIN = 0.1;
  STEP = 0.01;
  MAX = 0.5;
  x = MIN:STEP:MAX;
else
  x = pcp_thresholds;
end

y = zeros(1,length(x));

idx = 1;
for i=x
  [detrate y(idx)] = BatchEval(gtToBBfunc,eval_function,all_eval_stickmen,all_gt_stickmen,i);
  idx = idx + 1;
end

if show
  if nargin < 7
    curvecolor = [0 0 1];
  end
  if nargin < 8
    linespec = '-';
  end
  plot(x,y,linespec,'Color',curvecolor);
  xlabel('PCP_threshold','Interpreter','None');
  ylabel('PCP');
end




